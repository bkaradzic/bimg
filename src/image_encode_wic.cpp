/*
 * Copyright 2011-2026 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bimg/blob/master/LICENSE
 */

#include "bimg_p.h"

#if BIMG_CONFIG_ENCODE_WIC

#include "image_wic.h"

namespace bimg
{
	static const GUID kGUID_ContainerFormatBmp      = { 0x0af1d87e, 0xfcfe, 0x4188, { 0xbd, 0xeb, 0xa7, 0x90, 0x64, 0x71, 0xcb, 0xe3 } };
	static const GUID kGUID_ContainerFormatPng      = { 0x1b7cfaf4, 0x713f, 0x473c, { 0xbb, 0xcd, 0x61, 0x37, 0x42, 0x5f, 0xae, 0xaf } };
	static const GUID kGUID_WICPixelFormat8bppGray  = { 0x6fddc324, 0x4e03, 0x4bfe, { 0xb1, 0x85, 0x3d, 0x77, 0x76, 0x8d, 0xc9, 0x08 } };
	static const GUID kGUID_WICPixelFormat24bppBGR  = { 0x6fddc324, 0x4e03, 0x4bfe, { 0xb1, 0x85, 0x3d, 0x77, 0x76, 0x8d, 0xc9, 0x0c } };
	static const GUID kGUID_WICPixelFormat32bppBGRA = { 0x6fddc324, 0x4e03, 0x4bfe, { 0xb1, 0x85, 0x3d, 0x77, 0x76, 0x8d, 0xc9, 0x0f } };

	struct WicEncoder
	{
		const GUID* m_container;
		const GUID* m_color;
		const GUID* m_gray;
	};

	static const WicEncoder s_wicEncoder[] =
	{
		{ &kGUID_ContainerFormatBmp, &kGUID_WICPixelFormat24bppBGR,  &kGUID_WICPixelFormat24bppBGR },
		{ &kGUID_ContainerFormatPng, &kGUID_WICPixelFormat32bppBGRA, &kGUID_WICPixelFormat8bppGray },
	};
	static_assert(WicContainer::Count == BX_COUNTOF(s_wicEncoder) );

	struct WicEncode : public WicContext
	{
		WicEncode()
			: m_stream(NULL)
			, m_encoder(NULL)
			, m_frame(NULL)
			, m_bitmap(NULL)
			, m_flip(NULL)
			, m_converter(NULL)
		{
		}

		~WicEncode()
		{
			WIC_RELEASE(m_converter);
			WIC_RELEASE(m_flip);
			WIC_RELEASE(m_bitmap);
			WIC_RELEASE(m_frame);
			WIC_RELEASE(m_encoder);
			WIC_RELEASE(m_stream);
		}

		bool encode(WicContainer::Enum _container, uint32_t _width, uint32_t _height, uint32_t _srcPitch, const void* _src, TextureFormat::Enum _format, bool _yflip)
		{
			typedef HRESULT (WINAPI* PFN_CreateStreamOnHGlobal)(HGLOBAL, BOOL, LPSTREAM*);
			PFN_CreateStreamOnHGlobal createStreamOnHGlobal = bx::dlsym<PFN_CreateStreamOnHGlobal>(m_ole32Dll, "CreateStreamOnHGlobal");

			if (NULL == createStreamOnHGlobal)
			{
				return false;
			}

			const bool gray = TextureFormat::R8 == _format;

			const GUID& srcFormat = gray
				? kGUID_WICPixelFormat8bppGray
				: TextureFormat::RGBA8 == _format ? kGUID_WICPixelFormat32bppRGBA : kGUID_WICPixelFormat32bppBGRA
				;

			const WicEncoder& encoder = s_wicEncoder[_container];
			WICPixelFormatGUID dstFormat = gray ? *encoder.m_gray : *encoder.m_color;

			if (FAILED(createStreamOnHGlobal(NULL, TRUE, &m_stream) )
			||  FAILED(m_factory->CreateEncoder(*encoder.m_container, NULL, &m_encoder) )
			||  FAILED(m_encoder->Initialize(m_stream, WICBitmapEncoderNoCache) )
			||  FAILED(m_encoder->CreateNewFrame(&m_frame, NULL) )
			||  FAILED(m_frame->Initialize(NULL) )
			||  FAILED(m_frame->SetSize(_width, _height) )
			||  FAILED(m_frame->SetPixelFormat(&dstFormat) )
			||  FAILED(m_factory->CreateBitmapFromMemory(_width, _height, srcFormat, _srcPitch, _srcPitch*_height, (BYTE*)const_cast<void*>(_src), &m_bitmap) ) )
			{
				return false;
			}

			IWICBitmapSource* source = m_bitmap;

			if (_yflip)
			{
				if (FAILED(m_factory->CreateBitmapFlipRotator(&m_flip) )
				||  FAILED(m_flip->Initialize(source, WICBitmapTransformFlipVertical) ) )
				{
					return false;
				}

				source = m_flip;
			}

			if (!IsEqualGUID(dstFormat, srcFormat) )
			{
				if (FAILED(m_factory->CreateFormatConverter(&m_converter) )
				||  FAILED(m_converter->Initialize(source, dstFormat, WICBitmapDitherTypeNone, NULL, 0.0, WICBitmapPaletteTypeCustom) ) )
				{
					return false;
				}

				source = m_converter;
			}

			return SUCCEEDED(m_frame->WriteSource(source, NULL) )
				&& SUCCEEDED(m_frame->Commit() )
				&& SUCCEEDED(m_encoder->Commit() )
				;
		}

		int32_t write(bx::WriterI* _writer, bx::Error* _err)
		{
			typedef HRESULT (WINAPI* PFN_GetHGlobalFromStream)(LPSTREAM, HGLOBAL*);
			PFN_GetHGlobalFromStream getHGlobalFromStream = bx::dlsym<PFN_GetHGlobalFromStream>(m_ole32Dll, "GetHGlobalFromStream");

			LARGE_INTEGER zero = {};
			ULARGE_INTEGER size = {};
			HGLOBAL memory = NULL;

			if (NULL == getHGlobalFromStream
			||  FAILED(m_stream->Seek(zero, STREAM_SEEK_END, &size) )
			||  FAILED(getHGlobalFromStream(m_stream, &memory) ) )
			{
				return 0;
			}

			const void* data = GlobalLock(memory);

			if (NULL == data)
			{
				return 0;
			}

			const int32_t total = bx::write(_writer, data, int32_t(size.QuadPart), _err);
			GlobalUnlock(memory);

			return total;
		}

		IStream*               m_stream;
		IWICBitmapEncoder*     m_encoder;
		IWICBitmapFrameEncode* m_frame;
		IWICBitmap*            m_bitmap;
		IWICBitmapFlipRotator* m_flip;
		IWICFormatConverter*   m_converter;
	};

	int32_t imageEncodeWic(bx::WriterI* _writer, WicContainer::Enum _container, uint32_t _width, uint32_t _height, uint32_t _srcPitch, const void* _src, TextureFormat::Enum _format, bool _yflip, bx::Error* _err)
	{
		bx::Error unavailable;

		WicEncode wic;

		if (!wic.init(&unavailable)
		||  !wic.encode(_container, _width, _height, _srcPitch, _src, _format, _yflip) )
		{
			return 0;
		}

		return wic.write(_writer, _err);
	}

} // namespace bimg

#else

namespace bimg
{
	int32_t imageEncodeWic(bx::WriterI* /*_writer*/, WicContainer::Enum /*_container*/, uint32_t /*_width*/, uint32_t /*_height*/, uint32_t /*_srcPitch*/, const void* /*_src*/, TextureFormat::Enum /*_format*/, bool /*_yflip*/, bx::Error* /*_err*/)
	{
		return 0;
	}

} // namespace bimg

#endif // BIMG_CONFIG_ENCODE_WIC
