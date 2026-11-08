/*
 * Copyright 2011-2026 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bimg/blob/master/LICENSE
 */

#include "bimg_p.h"

#if BIMG_CONFIG_USE_WIC && (0            \
	|| BIMG_CONFIG_PARSE_PNG  \
	|| BIMG_CONFIG_PARSE_JPEG \
	|| BIMG_CONFIG_PARSE_BMP  \
	|| BIMG_CONFIG_PARSE_GIF  \
	)

#include "image_wic.h"

namespace bimg
{
	static ImageParser::Enum wicDetectFormat(const uint8_t* _data, uint32_t _size)
	{
		if (_size >= 8
		&&  0x89 == _data[0] && 'P' == _data[1] && 'N' == _data[2] && 'G' == _data[3])
		{
			return ImageParser::Png;
		}

		if (_size >= 3
		&&  0xff == _data[0] && 0xd8 == _data[1] && 0xff == _data[2])
		{
			return ImageParser::Jpeg;
		}

		if (_size >= 2
		&&  'B' == _data[0] && 'M' == _data[1])
		{
			return ImageParser::Bmp;
		}

		if (_size >= 6
		&&  'G' == _data[0] && 'I' == _data[1] && 'F' == _data[2])
		{
			return ImageParser::Gif;
		}

		return ImageParser::Count;
	}

	static bool wicIsEnabled(ImageParser::Enum _format)
	{
		switch (_format)
		{
		case ImageParser::Png:  return 0 != BIMG_CONFIG_PARSE_PNG;
		case ImageParser::Jpeg: return 0 != BIMG_CONFIG_PARSE_JPEG;
		case ImageParser::Bmp:  return 0 != BIMG_CONFIG_PARSE_BMP;
		case ImageParser::Gif:  return 0 != BIMG_CONFIG_PARSE_GIF;
		default:                return false;
		}
	}

	struct WicFrame : public WicContext
	{
		WicFrame()
			: m_stream(NULL)
			, m_decoder(NULL)
			, m_frame(NULL)
			, m_width(0)
			, m_height(0)
		{
		}

		~WicFrame()
		{
			WIC_RELEASE(m_frame);
			WIC_RELEASE(m_decoder);
			WIC_RELEASE(m_stream);
		}

		bool open(const void* _data, uint32_t _size, bx::Error* _err)
		{
			if (!init(_err) )
			{
				return false;
			}

			if (FAILED(m_factory->CreateStream(&m_stream) )
			||  FAILED(m_stream->InitializeFromMemory( (BYTE*)const_cast<void*>(_data), _size) )
			||  FAILED(m_factory->CreateDecoderFromStream(m_stream, NULL, WICDecodeMetadataCacheOnDemand, &m_decoder) )
			||  FAILED(m_decoder->GetFrame(0, &m_frame) ) )
			{
				BX_ERROR_SET(_err, BIMG_ERROR, "WIC: Failed to decode image.");
				return false;
			}

			UINT width  = 0;
			UINT height = 0;
			if (FAILED(m_frame->GetSize(&width, &height) )
			||  0 == width
			||  0 == height)
			{
				BX_ERROR_SET(_err, BIMG_ERROR, "WIC: Invalid image size.");
				return false;
			}

			m_width  = width;
			m_height = height;

			return true;
		}

		IWICStream*            m_stream;
		IWICBitmapDecoder*     m_decoder;
		IWICBitmapFrameDecode* m_frame;
		uint32_t               m_width;
		uint32_t               m_height;
	};

	static bool wicRestorePngGrayKey(WicFrame& _wic, const void* _data, uint32_t _size, ImageContainer& _image, bx::Error* _err)
	{
		static const uint8_t pngHeader[] =
		{
			0x89, 'P', 'N', 'G', 0x0d, 0x0a, 0x1a, 0x0a,
			0, 0, 0, 13, 'I', 'H', 'D', 'R',
		};
		const uint8_t* data = (const uint8_t*)_data;
		if (_size < 33
		||  0 != bx::memCmp(data, pngHeader, sizeof(pngHeader) )
		||  0 != data[25])
		{
			return true;
		}

		const uint32_t bitDepth = data[24];
		uint32_t formatIndex;
		switch (bitDepth)
		{
		case 1: formatIndex = 0; break;
		case 2: formatIndex = 1; break;
		case 4: formatIndex = 2; break;
		case 8: formatIndex = 3; break;
		default: return true;
		}

		static const GUID indexedFormats[] =
		{
			{ 0x6fddc324, 0x4e03, 0x4bfe, { 0xb1, 0x85, 0x3d, 0x77, 0x76, 0x8d, 0xc9, 0x01 } },
			{ 0x6fddc324, 0x4e03, 0x4bfe, { 0xb1, 0x85, 0x3d, 0x77, 0x76, 0x8d, 0xc9, 0x02 } },
			{ 0x6fddc324, 0x4e03, 0x4bfe, { 0xb1, 0x85, 0x3d, 0x77, 0x76, 0x8d, 0xc9, 0x03 } },
			{ 0x6fddc324, 0x4e03, 0x4bfe, { 0xb1, 0x85, 0x3d, 0x77, 0x76, 0x8d, 0xc9, 0x04 } },
		};
		WICPixelFormatGUID sourceFormat;
		if (FAILED(_wic.m_frame->GetPixelFormat(&sourceFormat) ) )
		{
			BX_ERROR_SET(_err, BIMG_ERROR, "WIC: Failed to query PNG pixel format.");
			return false;
		}

		if (!IsEqualGUID(sourceFormat, indexedFormats[formatIndex]) )
		{
			return true;
		}

		IWICPalette* palette = NULL;
		if (FAILED(_wic.m_factory->CreatePalette(&palette) )
		||  FAILED(_wic.m_frame->CopyPalette(palette) ) )
		{
			WIC_RELEASE(palette);
			BX_ERROR_SET(_err, BIMG_ERROR, "WIC: Failed to read PNG grayscale palette.");
			return false;
		}

		WICColor colors[256];
		UINT count = 0;
		const HRESULT hr = palette->GetColors(BX_COUNTOF(colors), colors, &count);
		WIC_RELEASE(palette);
		if (FAILED(hr)
		||  count != (1u<<bitDepth) )
		{
			BX_ERROR_SET(_err, BIMG_ERROR, "WIC: Invalid PNG grayscale palette.");
			return false;
		}

		uint32_t key = UINT32_MAX;
		for (uint32_t ii = 0; ii < count; ++ii)
		{
			const uint32_t alpha = colors[ii]>>24;
			if (0xff != alpha)
			{
				if (0 != alpha
				||  UINT32_MAX != key)
				{
					BX_ERROR_SET(_err, BIMG_ERROR, "WIC: Invalid PNG grayscale transparency.");
					return false;
				}

				key = ii;
			}
		}

		if (UINT32_MAX != key)
		{
			// WIC retains grayscale indices but zeros the transparent palette entry's RGB.
			const uint8_t gray = uint8_t(key*255 / (count-1) );
			uint8_t* rgba = (uint8_t*)_image.m_data;
			for (uint32_t ii = 0; ii < _image.m_size; ii += 4)
			{
				if (0 == rgba[ii+3])
				{
					rgba[ii+0] = gray;
					rgba[ii+1] = gray;
					rgba[ii+2] = gray;
				}
			}
		}

		return true;
	}

	ImageContainer* imageParseWic(bx::AllocatorI* _allocator, const void* _data, uint32_t _size, bx::Error* _err)
	{
		BX_ERROR_SCOPE(_err);

		const ImageParser::Enum format = wicDetectFormat( (const uint8_t*)_data, _size);

		if (!wicIsEnabled(format) )
		{
			return NULL;
		}

		WicFrame wic;
		if (!wic.open(_data, _size, _err) )
		{
			return NULL;
		}

		IWICFormatConverter* converter = NULL;
		if (FAILED(wic.m_factory->CreateFormatConverter(&converter) )
		||  FAILED(converter->Initialize(wic.m_frame, kGUID_WICPixelFormat32bppRGBA, WICBitmapDitherTypeNone, NULL, 0.0, WICBitmapPaletteTypeCustom) ) )
		{
			WIC_RELEASE(converter);
			BX_ERROR_SET(_err, BIMG_ERROR, "WIC: Failed to convert image to RGBA8.");
			return NULL;
		}

		ImageContainer* image = imageAlloc(_allocator, TextureFormat::RGBA8, wic.m_width, wic.m_height, 0, 1, false, false);
		if (NULL == image)
		{
			WIC_RELEASE(converter);
			BX_ERROR_SET(_err, BIMG_ERROR, "WIC: Unsupported dimensions.");
			return NULL;
		}

		const HRESULT hr = converter->CopyPixels(NULL, wic.m_width*4, image->m_size, (BYTE*)image->m_data);
		WIC_RELEASE(converter);

		if (FAILED(hr) )
		{
			imageFree(image);
			BX_ERROR_SET(_err, BIMG_ERROR, "WIC: Failed to copy pixels.");
			return NULL;
		}

		if (ImageParser::Png == format
		&&  !wicRestorePngGrayKey(wic, _data, _size, *image, _err) )
		{
			imageFree(image);
			return NULL;
		}

		image->m_parser = format;

		bool hasAlpha = false;
		const uint8_t* rgba = (const uint8_t*)image->m_data;
		for (uint32_t ii = 3; ii < image->m_size; ii += 4)
		{
			if (0xff != rgba[ii])
			{
				hasAlpha = true;
				break;
			}
		}

		image->m_hasAlpha = hasAlpha;

		return image;
	}

	bool imageParseInfoWic(bx::AllocatorI* _allocator, ImageContainer& _imageContainer, const void* _data, uint32_t _size, bx::Error* _err)
	{
		BX_UNUSED(_allocator);
		BX_ERROR_SCOPE(_err);

		const ImageParser::Enum format = wicDetectFormat( (const uint8_t*)_data, _size);

		if (!wicIsEnabled(format) )
		{
			return false;
		}

		WicFrame wic;
		if (!wic.open(_data, _size, _err) )
		{
			return false;
		}

		return imageInfoFinalize(_imageContainer, format, TextureFormat::RGBA8, wic.m_width, wic.m_height, _err);
	}

} // namespace bimg

#else

namespace bimg
{
	ImageContainer* imageParseWic(bx::AllocatorI* /*_allocator*/, const void* /*_data*/, uint32_t /*_size*/, bx::Error* /*_err*/)
	{
		return NULL;
	}

	bool imageParseInfoWic(bx::AllocatorI* /*_allocator*/, ImageContainer& /*_imageContainer*/, const void* /*_data*/, uint32_t /*_size*/, bx::Error* /*_err*/)
	{
		return false;
	}

} // namespace bimg

#endif // BIMG_CONFIG_USE_WIC && (BIMG_CONFIG_PARSE_PNG || BIMG_CONFIG_PARSE_JPEG || BIMG_CONFIG_PARSE_BMP || BIMG_CONFIG_PARSE_GIF)
