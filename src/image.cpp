/*
 * Copyright 2011-2017 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause
 */

#include "bimg_p.h"
#include <bx/hash.h>

namespace bimg
{
	static const ImageBlockInfo s_imageBlockInfo[] =
	{
		//  +-------------------------------------------- bits per pixel
		//  |  +----------------------------------------- block width
		//  |  |  +-------------------------------------- block height
		//  |  |  |   +---------------------------------- block size
		//  |  |  |   |  +------------------------------- min blocks x
		//  |  |  |   |  |  +---------------------------- min blocks y
		//  |  |  |   |  |  |   +------------------------ depth bits
		//  |  |  |   |  |  |   |  +--------------------- stencil bits
		//  |  |  |   |  |  |   |  |   +---+---+---+----- r, g, b, a bits
		//  |  |  |   |  |  |   |  |   r   g   b   a  +-- encoding type
		//  |  |  |   |  |  |   |  |   |   |   |   |  |
		{   4, 4, 4,  8, 1, 1,  0, 0,  0,  0,  0,  0, uint8_t(bx::EncodingType::Unorm) }, // BC1
		{   8, 4, 4, 16, 1, 1,  0, 0,  0,  0,  0,  0, uint8_t(bx::EncodingType::Unorm) }, // BC2
		{   8, 4, 4, 16, 1, 1,  0, 0,  0,  0,  0,  0, uint8_t(bx::EncodingType::Unorm) }, // BC3
		{   4, 4, 4,  8, 1, 1,  0, 0,  0,  0,  0,  0, uint8_t(bx::EncodingType::Unorm) }, // BC4
		{   8, 4, 4, 16, 1, 1,  0, 0,  0,  0,  0,  0, uint8_t(bx::EncodingType::Unorm) }, // BC5
		{   8, 4, 4, 16, 1, 1,  0, 0,  0,  0,  0,  0, uint8_t(bx::EncodingType::Unorm) }, // BC6H
		{   8, 4, 4, 16, 1, 1,  0, 0,  0,  0,  0,  0, uint8_t(bx::EncodingType::Unorm) }, // BC7
		{   4, 4, 4,  8, 1, 1,  0, 0,  0,  0,  0,  0, uint8_t(bx::EncodingType::Unorm) }, // ETC1
		{   4, 4, 4,  8, 1, 1,  0, 0,  0,  0,  0,  0, uint8_t(bx::EncodingType::Unorm) }, // ETC2
		{   8, 4, 4, 16, 1, 1,  0, 0,  0,  0,  0,  0, uint8_t(bx::EncodingType::Unorm) }, // ETC2A
		{   4, 4, 4,  8, 1, 1,  0, 0,  0,  0,  0,  0, uint8_t(bx::EncodingType::Unorm) }, // ETC2A1
		{   2, 8, 4,  8, 2, 2,  0, 0,  0,  0,  0,  0, uint8_t(bx::EncodingType::Unorm) }, // PTC12
		{   4, 4, 4,  8, 2, 2,  0, 0,  0,  0,  0,  0, uint8_t(bx::EncodingType::Unorm) }, // PTC14
		{   2, 8, 4,  8, 2, 2,  0, 0,  0,  0,  0,  0, uint8_t(bx::EncodingType::Unorm) }, // PTC12A
		{   4, 4, 4,  8, 2, 2,  0, 0,  0,  0,  0,  0, uint8_t(bx::EncodingType::Unorm) }, // PTC14A
		{   2, 8, 4,  8, 2, 2,  0, 0,  0,  0,  0,  0, uint8_t(bx::EncodingType::Unorm) }, // PTC22
		{   4, 4, 4,  8, 2, 2,  0, 0,  0,  0,  0,  0, uint8_t(bx::EncodingType::Unorm) }, // PTC24
		{   0, 0, 0,  0, 0, 0,  0, 0,  0,  0,  0,  0, uint8_t(bx::EncodingType::Count) }, // Unknown
		{   1, 8, 1,  1, 1, 1,  0, 0,  1,  0,  0,  0, uint8_t(bx::EncodingType::Unorm) }, // R1
		{   8, 1, 1,  1, 1, 1,  0, 0,  0,  0,  0,  8, uint8_t(bx::EncodingType::Unorm) }, // A8
		{   8, 1, 1,  1, 1, 1,  0, 0,  8,  0,  0,  0, uint8_t(bx::EncodingType::Unorm) }, // R8
		{   8, 1, 1,  1, 1, 1,  0, 0,  8,  0,  0,  0, uint8_t(bx::EncodingType::Int  ) }, // R8I
		{   8, 1, 1,  1, 1, 1,  0, 0,  8,  0,  0,  0, uint8_t(bx::EncodingType::Uint ) }, // R8U
		{   8, 1, 1,  1, 1, 1,  0, 0,  8,  0,  0,  0, uint8_t(bx::EncodingType::Snorm) }, // R8S
		{  16, 1, 1,  2, 1, 1,  0, 0, 16,  0,  0,  0, uint8_t(bx::EncodingType::Unorm) }, // R16
		{  16, 1, 1,  2, 1, 1,  0, 0, 16,  0,  0,  0, uint8_t(bx::EncodingType::Int  ) }, // R16I
		{  16, 1, 1,  2, 1, 1,  0, 0, 16,  0,  0,  0, uint8_t(bx::EncodingType::Uint ) }, // R16U
		{  16, 1, 1,  2, 1, 1,  0, 0, 16,  0,  0,  0, uint8_t(bx::EncodingType::Float) }, // R16F
		{  16, 1, 1,  2, 1, 1,  0, 0, 16,  0,  0,  0, uint8_t(bx::EncodingType::Snorm) }, // R16S
		{  32, 1, 1,  4, 1, 1,  0, 0, 32,  0,  0,  0, uint8_t(bx::EncodingType::Int  ) }, // R32I
		{  32, 1, 1,  4, 1, 1,  0, 0, 32,  0,  0,  0, uint8_t(bx::EncodingType::Uint ) }, // R32U
		{  32, 1, 1,  4, 1, 1,  0, 0, 32,  0,  0,  0, uint8_t(bx::EncodingType::Float) }, // R32F
		{  16, 1, 1,  2, 1, 1,  0, 0,  8,  8,  0,  0, uint8_t(bx::EncodingType::Unorm) }, // RG8
		{  16, 1, 1,  2, 1, 1,  0, 0,  8,  8,  0,  0, uint8_t(bx::EncodingType::Int  ) }, // RG8I
		{  16, 1, 1,  2, 1, 1,  0, 0,  8,  8,  0,  0, uint8_t(bx::EncodingType::Uint ) }, // RG8U
		{  16, 1, 1,  2, 1, 1,  0, 0,  8,  8,  0,  0, uint8_t(bx::EncodingType::Snorm) }, // RG8S
		{  32, 1, 1,  4, 1, 1,  0, 0, 16, 16,  0,  0, uint8_t(bx::EncodingType::Unorm) }, // RG16
		{  32, 1, 1,  4, 1, 1,  0, 0, 16, 16,  0,  0, uint8_t(bx::EncodingType::Int  ) }, // RG16I
		{  32, 1, 1,  4, 1, 1,  0, 0, 16, 16,  0,  0, uint8_t(bx::EncodingType::Uint ) }, // RG16U
		{  32, 1, 1,  4, 1, 1,  0, 0, 16, 16,  0,  0, uint8_t(bx::EncodingType::Float) }, // RG16F
		{  32, 1, 1,  4, 1, 1,  0, 0, 16, 16,  0,  0, uint8_t(bx::EncodingType::Snorm) }, // RG16S
		{  64, 1, 1,  8, 1, 1,  0, 0, 32, 32,  0,  0, uint8_t(bx::EncodingType::Int  ) }, // RG32I
		{  64, 1, 1,  8, 1, 1,  0, 0, 32, 32,  0,  0, uint8_t(bx::EncodingType::Uint ) }, // RG32U
		{  64, 1, 1,  8, 1, 1,  0, 0, 32, 32,  0,  0, uint8_t(bx::EncodingType::Float) }, // RG32F
		{  24, 1, 1,  3, 1, 1,  0, 0,  8,  8,  8,  0, uint8_t(bx::EncodingType::Unorm) }, // RGB8
		{  24, 1, 1,  3, 1, 1,  0, 0,  8,  8,  8,  0, uint8_t(bx::EncodingType::Int  ) }, // RGB8I
		{  24, 1, 1,  3, 1, 1,  0, 0,  8,  8,  8,  0, uint8_t(bx::EncodingType::Uint ) }, // RGB8U
		{  24, 1, 1,  3, 1, 1,  0, 0,  8,  8,  8,  0, uint8_t(bx::EncodingType::Snorm) }, // RGB8S
		{  32, 1, 1,  4, 1, 1,  0, 0,  9,  9,  9,  5, uint8_t(bx::EncodingType::Float) }, // RGB9E5F
		{  32, 1, 1,  4, 1, 1,  0, 0,  8,  8,  8,  8, uint8_t(bx::EncodingType::Unorm) }, // BGRA8
		{  32, 1, 1,  4, 1, 1,  0, 0,  8,  8,  8,  8, uint8_t(bx::EncodingType::Unorm) }, // RGBA8
		{  32, 1, 1,  4, 1, 1,  0, 0,  8,  8,  8,  8, uint8_t(bx::EncodingType::Int  ) }, // RGBA8I
		{  32, 1, 1,  4, 1, 1,  0, 0,  8,  8,  8,  8, uint8_t(bx::EncodingType::Uint ) }, // RGBA8U
		{  32, 1, 1,  4, 1, 1,  0, 0,  8,  8,  8,  8, uint8_t(bx::EncodingType::Snorm) }, // RGBA8S
		{  64, 1, 1,  8, 1, 1,  0, 0, 16, 16, 16, 16, uint8_t(bx::EncodingType::Unorm) }, // RGBA16
		{  64, 1, 1,  8, 1, 1,  0, 0, 16, 16, 16, 16, uint8_t(bx::EncodingType::Int  ) }, // RGBA16I
		{  64, 1, 1,  8, 1, 1,  0, 0, 16, 16, 16, 16, uint8_t(bx::EncodingType::Uint ) }, // RGBA16U
		{  64, 1, 1,  8, 1, 1,  0, 0, 16, 16, 16, 16, uint8_t(bx::EncodingType::Float) }, // RGBA16F
		{  64, 1, 1,  8, 1, 1,  0, 0, 16, 16, 16, 16, uint8_t(bx::EncodingType::Snorm) }, // RGBA16S
		{ 128, 1, 1, 16, 1, 1,  0, 0, 32, 32, 32, 32, uint8_t(bx::EncodingType::Int  ) }, // RGBA32I
		{ 128, 1, 1, 16, 1, 1,  0, 0, 32, 32, 32, 32, uint8_t(bx::EncodingType::Uint ) }, // RGBA32U
		{ 128, 1, 1, 16, 1, 1,  0, 0, 32, 32, 32, 32, uint8_t(bx::EncodingType::Float) }, // RGBA32F
		{  16, 1, 1,  2, 1, 1,  0, 0,  5,  6,  5,  0, uint8_t(bx::EncodingType::Unorm) }, // R5G6B5
		{  16, 1, 1,  2, 1, 1,  0, 0,  4,  4,  4,  4, uint8_t(bx::EncodingType::Unorm) }, // RGBA4
		{  16, 1, 1,  2, 1, 1,  0, 0,  5,  5,  5,  1, uint8_t(bx::EncodingType::Unorm) }, // RGB5A1
		{  32, 1, 1,  4, 1, 1,  0, 0, 10, 10, 10,  2, uint8_t(bx::EncodingType::Unorm) }, // RGB10A2
		{  32, 1, 1,  4, 1, 1,  0, 0, 11, 11, 10,  0, uint8_t(bx::EncodingType::Unorm) }, // RG11B10F
		{   0, 0, 0,  0, 0, 0,  0, 0,  0,  0,  0,  0, uint8_t(bx::EncodingType::Count) }, // UnknownDepth
		{  16, 1, 1,  2, 1, 1, 16, 0,  0,  0,  0,  0, uint8_t(bx::EncodingType::Unorm) }, // D16
		{  24, 1, 1,  3, 1, 1, 24, 0,  0,  0,  0,  0, uint8_t(bx::EncodingType::Unorm) }, // D24
		{  32, 1, 1,  4, 1, 1, 24, 8,  0,  0,  0,  0, uint8_t(bx::EncodingType::Unorm) }, // D24S8
		{  32, 1, 1,  4, 1, 1, 32, 0,  0,  0,  0,  0, uint8_t(bx::EncodingType::Unorm) }, // D32
		{  16, 1, 1,  2, 1, 1, 16, 0,  0,  0,  0,  0, uint8_t(bx::EncodingType::Float) }, // D16F
		{  24, 1, 1,  3, 1, 1, 24, 0,  0,  0,  0,  0, uint8_t(bx::EncodingType::Float) }, // D24F
		{  32, 1, 1,  4, 1, 1, 32, 0,  0,  0,  0,  0, uint8_t(bx::EncodingType::Float) }, // D32F
		{   8, 1, 1,  1, 1, 1,  0, 8,  0,  0,  0,  0, uint8_t(bx::EncodingType::Unorm) }, // D0S8
	};
	BX_STATIC_ASSERT(TextureFormat::Count == BX_COUNTOF(s_imageBlockInfo) );

	static const char* s_textureFormatName[] =
	{
		"BC1",        // BC1
		"BC2",        // BC2
		"BC3",        // BC3
		"BC4",        // BC4
		"BC5",        // BC5
		"BC6H",       // BC6H
		"BC7",        // BC7
		"ETC1",       // ETC1
		"ETC2",       // ETC2
		"ETC2A",      // ETC2A
		"ETC2A1",     // ETC2A1
		"PTC12",      // PTC12
		"PTC14",      // PTC14
		"PTC12A",     // PTC12A
		"PTC14A",     // PTC14A
		"PTC22",      // PTC22
		"PTC24",      // PTC24
		"<unknown>",  // Unknown
		"R1",         // R1
		"A8",         // A8
		"R8",         // R8
		"R8I",        // R8I
		"R8U",        // R8U
		"R8S",        // R8S
		"R16",        // R16
		"R16I",       // R16I
		"R16U",       // R16U
		"R16F",       // R16F
		"R16S",       // R16S
		"R32I",       // R32I
		"R32U",       // R32U
		"R32F",       // R32F
		"RG8",        // RG8
		"RG8I",       // RG8I
		"RG8U",       // RG8U
		"RG8S",       // RG8S
		"RG16",       // RG16
		"RG16I",      // RG16I
		"RG16U",      // RG16U
		"RG16F",      // RG16F
		"RG16S",      // RG16S
		"RG32I",      // RG32I
		"RG32U",      // RG32U
		"RG32F",      // RG32F
		"RGB8",       // RGB8
		"RGB8I",      // RGB8I
		"RGB8U",      // RGB8U
		"RGB8S",      // RGB8S
		"RGB9E5",     // RGB9E5F
		"BGRA8",      // BGRA8
		"RGBA8",      // RGBA8
		"RGBA8I",     // RGBA8I
		"RGBA8U",     // RGBA8U
		"RGBA8S",     // RGBA8S
		"RGBA16",     // RGBA16
		"RGBA16I",    // RGBA16I
		"RGBA16U",    // RGBA16U
		"RGBA16F",    // RGBA16F
		"RGBA16S",    // RGBA16S
		"RGBA32I",    // RGBA32I
		"RGBA32U",    // RGBA32U
		"RGBA32F",    // RGBA32F
		"R5G6B5",     // R5G6B5
		"RGBA4",      // RGBA4
		"RGB5A1",     // RGB5A1
		"RGB10A2",    // RGB10A2
		"RG11B10F", // RG11B10F
		"<unknown>",  // UnknownDepth
		"D16",        // D16
		"D24",        // D24
		"D24S8",      // D24S8
		"D32",        // D32
		"D16F",       // D16F
		"D24F",       // D24F
		"D32F",       // D32F
		"D0S8",       // D0S8
	};
	BX_STATIC_ASSERT(TextureFormat::Count == BX_COUNTOF(s_textureFormatName) );

	bool isCompressed(TextureFormat::Enum _format)
	{
		return _format < TextureFormat::Unknown;
	}

	bool isColor(TextureFormat::Enum _format)
	{
		return _format > TextureFormat::Unknown
			&& _format < TextureFormat::UnknownDepth
			;
	}

	bool isDepth(TextureFormat::Enum _format)
	{
		return _format > TextureFormat::UnknownDepth
			&& _format < TextureFormat::Count
			;
	}

	bool isValid(TextureFormat::Enum _format)
	{
		return _format != TextureFormat::Unknown
			&& _format != TextureFormat::UnknownDepth
			&& _format != TextureFormat::Count
			;
	}

	uint8_t getBitsPerPixel(TextureFormat::Enum _format)
	{
		return s_imageBlockInfo[_format].bitsPerPixel;
	}

	const ImageBlockInfo& getBlockInfo(TextureFormat::Enum _format)
	{
		return s_imageBlockInfo[_format];
	}

	uint8_t getBlockSize(TextureFormat::Enum _format)
	{
		return s_imageBlockInfo[_format].blockSize;
	}

	const char* getName(TextureFormat::Enum _format)
	{
		return s_textureFormatName[_format];
	}

	TextureFormat::Enum getFormat(const char* _name)
	{
		for (uint32_t ii = 0; ii < TextureFormat::Count; ++ii)
		{
			const TextureFormat::Enum fmt = TextureFormat::Enum(ii);
			if (isValid(fmt) )
			{
				if (0 == bx::strCmpI(s_textureFormatName[ii], _name) )
				{
					return fmt;
				}
			}
		}

		return TextureFormat::Unknown;
	}

	uint8_t imageGetNumMips(TextureFormat::Enum _format, uint16_t _width, uint16_t _height, uint16_t _depth)
	{
		const ImageBlockInfo& blockInfo = getBlockInfo(_format);
		const uint16_t blockWidth  = blockInfo.blockWidth;
		const uint16_t blockHeight = blockInfo.blockHeight;
		const uint16_t minBlockX   = blockInfo.minBlockX;
		const uint16_t minBlockY   = blockInfo.minBlockY;

		_width  = bx::uint16_max(blockWidth  * minBlockX, ( (_width  + blockWidth  - 1) / blockWidth )*blockWidth);
		_height = bx::uint16_max(blockHeight * minBlockY, ( (_height + blockHeight - 1) / blockHeight)*blockHeight);
		_depth  = bx::uint16_max(1, _depth);

		uint8_t numMips = calcNumMips(true, _width, _height, _depth);

		return numMips;
	}

	uint32_t imageGetSize(TextureInfo* _info, uint16_t _width, uint16_t _height, uint16_t _depth, bool _cubeMap, bool _hasMips, uint16_t _numLayers, TextureFormat::Enum _format)
	{
		const ImageBlockInfo& blockInfo = getBlockInfo(_format);
		const uint8_t  bpp         = blockInfo.bitsPerPixel;
		const uint16_t blockWidth  = blockInfo.blockWidth;
		const uint16_t blockHeight = blockInfo.blockHeight;
		const uint16_t minBlockX   = blockInfo.minBlockX;
		const uint16_t minBlockY   = blockInfo.minBlockY;

		_width  = bx::uint16_max(blockWidth  * minBlockX, ( (_width  + blockWidth  - 1) / blockWidth)*blockWidth);
		_height = bx::uint16_max(blockHeight * minBlockY, ( (_height + blockHeight - 1) / blockHeight)*blockHeight);
		_depth  = bx::uint16_max(1, _depth);
		const uint8_t  numMips = calcNumMips(_hasMips, _width, _height, _depth);
		const uint32_t sides   = _cubeMap ? 6 : 1;

		uint32_t width  = _width;
		uint32_t height = _height;
		uint32_t depth  = _depth;
		uint32_t size   = 0;

		for (uint32_t lod = 0; lod < numMips; ++lod)
		{
			width  = bx::uint32_max(blockWidth  * minBlockX, ( (width  + blockWidth  - 1) / blockWidth )*blockWidth);
			height = bx::uint32_max(blockHeight * minBlockY, ( (height + blockHeight - 1) / blockHeight)*blockHeight);
			depth  = bx::uint32_max(1, depth);

			size += uint32_t(uint64_t(width*height*depth)*bpp/8 * sides);

			width  >>= 1;
			height >>= 1;
			depth  >>= 1;
		}

		size *= _numLayers;

		if (NULL != _info)
		{
			_info->format  = _format;
			_info->width   = _width;
			_info->height  = _height;
			_info->depth   = _depth;
			_info->numMips = numMips;
			_info->numLayers = _numLayers;
			_info->cubeMap   = _cubeMap;
			_info->storageSize  = size;
			_info->bitsPerPixel = bpp;
		}

		return size;
	}

	void imageSolid(void* _dst, uint32_t _width, uint32_t _height, uint32_t _solid)
	{
		uint32_t* dst = (uint32_t*)_dst;
		for (uint32_t ii = 0, num = _width*_height; ii < num; ++ii)
		{
			*dst++ = _solid;
		}
	}

	void imageCheckerboard(void* _dst, uint32_t _width, uint32_t _height, uint32_t _step, uint32_t _0, uint32_t _1)
	{
		uint32_t* dst = (uint32_t*)_dst;
		for (uint32_t yy = 0; yy < _height; ++yy)
		{
			for (uint32_t xx = 0; xx < _width; ++xx)
			{
				uint32_t abgr = ( (xx/_step)&1) ^ ( (yy/_step)&1) ? _1 : _0;
				*dst++ = abgr;
			}
		}
	}

	void imageRgba8Downsample2x2Ref(void* _dst, uint32_t _width, uint32_t _height, uint32_t _depth, uint32_t _srcPitch, const void* _src)
	{
		const uint32_t dstWidth  = _width/2;
		const uint32_t dstHeight = _height/2;

		if (0 == dstWidth
		||  0 == dstHeight)
		{
			return;
		}

		uint8_t* dst = (uint8_t*)_dst;
		const uint8_t* src = (const uint8_t*)_src;

		for (uint32_t zz = 0; zz < _depth; ++zz)
		{
			for (uint32_t yy = 0, ystep = _srcPitch*2; yy < dstHeight; ++yy, src += ystep)
			{
				const uint8_t* rgba = src;
				for (uint32_t xx = 0; xx < dstWidth; ++xx, rgba += 8, dst += 4)
				{
					float rr = bx::fpow(rgba[          0], 2.2f);
					float gg = bx::fpow(rgba[          1], 2.2f);
					float bb = bx::fpow(rgba[          2], 2.2f);
					float aa =          rgba[          3];
					rr      += bx::fpow(rgba[          4], 2.2f);
					gg      += bx::fpow(rgba[          5], 2.2f);
					bb      += bx::fpow(rgba[          6], 2.2f);
					aa      +=          rgba[          7];
					rr      += bx::fpow(rgba[_srcPitch+0], 2.2f);
					gg      += bx::fpow(rgba[_srcPitch+1], 2.2f);
					bb      += bx::fpow(rgba[_srcPitch+2], 2.2f);
					aa      +=          rgba[_srcPitch+3];
					rr      += bx::fpow(rgba[_srcPitch+4], 2.2f);
					gg      += bx::fpow(rgba[_srcPitch+5], 2.2f);
					bb      += bx::fpow(rgba[_srcPitch+6], 2.2f);
					aa      +=          rgba[_srcPitch+7];

					rr *= 0.25f;
					gg *= 0.25f;
					bb *= 0.25f;
					aa *= 0.25f;
					rr = bx::fpow(rr, 1.0f/2.2f);
					gg = bx::fpow(gg, 1.0f/2.2f);
					bb = bx::fpow(bb, 1.0f/2.2f);
					dst[0] = (uint8_t)rr;
					dst[1] = (uint8_t)gg;
					dst[2] = (uint8_t)bb;
					dst[3] = (uint8_t)aa;
				}
			}
		}
	}

	void imageRgba8Downsample2x2(void* _dst, uint32_t _width, uint32_t _height, uint32_t _depth, uint32_t _srcPitch, const void* _src)
	{
		const uint32_t dstWidth  = _width/2;
		const uint32_t dstHeight = _height/2;

		if (0 == dstWidth
		||  0 == dstHeight)
		{
			return;
		}

		uint8_t* dst = (uint8_t*)_dst;
		const uint8_t* src = (const uint8_t*)_src;

		using namespace bx;
		const simd128_t unpack = simd_ld(1.0f, 1.0f/256.0f, 1.0f/65536.0f, 1.0f/16777216.0f);
		const simd128_t pack   = simd_ld(1.0f, 256.0f*0.5f, 65536.0f, 16777216.0f*0.5f);
		const simd128_t umask  = simd_ild(0xff, 0xff00, 0xff0000, 0xff000000);
		const simd128_t pmask  = simd_ild(0xff, 0x7f80, 0xff0000, 0x7f800000);
		const simd128_t wflip  = simd_ild(0, 0, 0, 0x80000000);
		const simd128_t wadd   = simd_ld(0.0f, 0.0f, 0.0f, 32768.0f*65536.0f);
		const simd128_t gamma  = simd_ld(1.0f/2.2f, 1.0f/2.2f, 1.0f/2.2f, 1.0f);
		const simd128_t linear = simd_ld(2.2f, 2.2f, 2.2f, 1.0f);
		const simd128_t quater = simd_splat(0.25f);

		for (uint32_t zz = 0; zz < _depth; ++zz)
		{
			for (uint32_t yy = 0, ystep = _srcPitch*2; yy < dstHeight; ++yy, src += ystep)
			{
				const uint8_t* rgba = src;
				for (uint32_t xx = 0; xx < dstWidth; ++xx, rgba += 8, dst += 4)
				{
					const simd128_t abgr0  = simd_splat(rgba);
					const simd128_t abgr1  = simd_splat(rgba+4);
					const simd128_t abgr2  = simd_splat(rgba+_srcPitch);
					const simd128_t abgr3  = simd_splat(rgba+_srcPitch+4);

					const simd128_t abgr0m = simd_and(abgr0, umask);
					const simd128_t abgr1m = simd_and(abgr1, umask);
					const simd128_t abgr2m = simd_and(abgr2, umask);
					const simd128_t abgr3m = simd_and(abgr3, umask);
					const simd128_t abgr0x = simd_xor(abgr0m, wflip);
					const simd128_t abgr1x = simd_xor(abgr1m, wflip);
					const simd128_t abgr2x = simd_xor(abgr2m, wflip);
					const simd128_t abgr3x = simd_xor(abgr3m, wflip);
					const simd128_t abgr0f = simd_itof(abgr0x);
					const simd128_t abgr1f = simd_itof(abgr1x);
					const simd128_t abgr2f = simd_itof(abgr2x);
					const simd128_t abgr3f = simd_itof(abgr3x);
					const simd128_t abgr0c = simd_add(abgr0f, wadd);
					const simd128_t abgr1c = simd_add(abgr1f, wadd);
					const simd128_t abgr2c = simd_add(abgr2f, wadd);
					const simd128_t abgr3c = simd_add(abgr3f, wadd);
					const simd128_t abgr0n = simd_mul(abgr0c, unpack);
					const simd128_t abgr1n = simd_mul(abgr1c, unpack);
					const simd128_t abgr2n = simd_mul(abgr2c, unpack);
					const simd128_t abgr3n = simd_mul(abgr3c, unpack);

					const simd128_t abgr0l = simd_pow(abgr0n, linear);
					const simd128_t abgr1l = simd_pow(abgr1n, linear);
					const simd128_t abgr2l = simd_pow(abgr2n, linear);
					const simd128_t abgr3l = simd_pow(abgr3n, linear);

					const simd128_t sum0   = simd_add(abgr0l, abgr1l);
					const simd128_t sum1   = simd_add(abgr2l, abgr3l);
					const simd128_t sum2   = simd_add(sum0, sum1);
					const simd128_t avg0   = simd_mul(sum2, quater);
					const simd128_t avg1   = simd_pow(avg0, gamma);

					const simd128_t avg2   = simd_mul(avg1, pack);
					const simd128_t ftoi0  = simd_ftoi(avg2);
					const simd128_t ftoi1  = simd_and(ftoi0, pmask);
					const simd128_t zwxy   = simd_swiz_zwxy(ftoi1);
					const simd128_t tmp0   = simd_or(ftoi1, zwxy);
					const simd128_t yyyy   = simd_swiz_yyyy(tmp0);
					const simd128_t tmp1   = simd_iadd(yyyy, yyyy);
					const simd128_t result = simd_or(tmp0, tmp1);

					simd_stx(dst, result);
				}
			}
		}
	}

	void imageRgba32fToLinear(void* _dst, uint32_t _width, uint32_t _height, uint32_t _depth, uint32_t _srcPitch, const void* _src)
	{
		      uint8_t* dst = (      uint8_t*)_dst;
		const uint8_t* src = (const uint8_t*)_src;

		for (uint32_t zz = 0; zz < _depth; ++zz)
		{
			for (uint32_t yy = 0; yy < _height; ++yy, src += _srcPitch, dst += _width*16)
			{
				for (uint32_t xx = 0; xx < _width; ++xx)
				{
					const uint32_t offset = xx * 16;
						  float* fd = (      float*)(dst + offset);
					const float* fs = (const float*)(src + offset);

					fd[0] = bx::fpow(fs[0], 1.0f/2.2f);
					fd[1] = bx::fpow(fs[1], 1.0f/2.2f);
					fd[2] = bx::fpow(fs[2], 1.0f/2.2f);
					fd[3] =          fs[3];
				}
			}
		}
	}

	void imageRgba32fToGamma(void* _dst, uint32_t _width, uint32_t _height, uint32_t _depth, uint32_t _srcPitch, const void* _src)
	{
		      uint8_t* dst = (      uint8_t*)_dst;
		const uint8_t* src = (const uint8_t*)_src;

		for (uint32_t zz = 0; zz < _depth; ++zz)
		{
			for (uint32_t yy = 0; yy < _height; ++yy, src += _srcPitch, dst += _width*16)
			{
				for (uint32_t xx = 0; xx < _width; ++xx)
				{
					const uint32_t offset = xx * 16;
						  float* fd = (      float*)(dst + offset);
					const float* fs = (const float*)(src + offset);

					fd[0] = bx::fpow(fs[0], 2.2f);
					fd[1] = bx::fpow(fs[1], 2.2f);
					fd[2] = bx::fpow(fs[2], 2.2f);
					fd[3] =          fs[3];
				}
			}
		}
	}

	void imageRgba32fLinearDownsample2x2Ref(void* _dst, uint32_t _width, uint32_t _height, uint32_t _depth, uint32_t _srcPitch, const void* _src)
	{
		const uint32_t dstWidth  = _width/2;
		const uint32_t dstHeight = _height/2;

		if (0 == dstWidth
		||  0 == dstHeight)
		{
			return;
		}

		const uint8_t* src = (const uint8_t*)_src;
		uint8_t* dst = (uint8_t*)_dst;

		for (uint32_t zz = 0; zz < _depth; ++zz)
		{
			for (uint32_t yy = 0, ystep = _srcPitch*2; yy < dstHeight; ++yy, src += ystep)
			{
				const float* rgba0 = (const float*)&src[0];
				const float* rgba1 = (const float*)&src[_srcPitch];
				for (uint32_t xx = 0; xx < dstWidth; ++xx, rgba0 += 8, rgba1 += 8, dst += 16)
				{
					float xyz[4];

					xyz[0]  = rgba0[0];
					xyz[1]  = rgba0[1];
					xyz[2]  = rgba0[2];
					xyz[3]  = rgba0[3];

					xyz[0] += rgba0[4];
					xyz[1] += rgba0[5];
					xyz[2] += rgba0[6];
					xyz[3] += rgba0[7];

					xyz[0] += rgba1[0];
					xyz[1] += rgba1[1];
					xyz[2] += rgba1[2];
					xyz[3] += rgba1[3];

					xyz[0] += rgba1[4];
					xyz[1] += rgba1[5];
					xyz[2] += rgba1[6];
					xyz[3] += rgba1[7];

					xyz[0] *= 0.25f;
					xyz[1] *= 0.25f;
					xyz[2] *= 0.25f;
					xyz[3] *= 0.25f;

					bx::packRgba32F(dst, xyz);
				}
			}
		}
	}

	void imageRgba32fLinearDownsample2x2(void* _dst, uint32_t _width, uint32_t _height, uint32_t _depth, uint32_t _srcPitch, const void* _src)
	{
		imageRgba32fLinearDownsample2x2Ref(_dst, _width, _height, _depth, _srcPitch, _src);
	}

	void imageRgba32fDownsample2x2NormalMapRef(void* _dst, uint32_t _width, uint32_t _height, uint32_t _srcPitch, const void* _src)
	{
		const uint32_t dstWidth  = _width/2;
		const uint32_t dstHeight = _height/2;

		if (0 == dstWidth
		||  0 == dstHeight)
		{
			return;
		}

		const uint8_t* src = (const uint8_t*)_src;
		uint8_t* dst = (uint8_t*)_dst;

		for (uint32_t yy = 0, ystep = _srcPitch*2; yy < dstHeight; ++yy, src += ystep)
		{
			const float* rgba0 = (const float*)&src[0];
			const float* rgba1 = (const float*)&src[_srcPitch];
			for (uint32_t xx = 0; xx < dstWidth; ++xx, rgba0 += 8, rgba1 += 8, dst += 16)
			{
				float xyz[3];

				xyz[0]  = rgba0[0];
				xyz[1]  = rgba0[1];
				xyz[2]  = rgba0[2];

				xyz[0] += rgba0[4];
				xyz[1] += rgba0[5];
				xyz[2] += rgba0[6];

				xyz[0] += rgba1[0];
				xyz[1] += rgba1[1];
				xyz[2] += rgba1[2];

				xyz[0] += rgba1[4];
				xyz[1] += rgba1[5];
				xyz[2] += rgba1[6];

				bx::vec3Norm( (float*)dst, xyz);
			}
		}
	}

	void imageRgba32fDownsample2x2NormalMap(void* _dst, uint32_t _width, uint32_t _height, uint32_t _srcPitch, const void* _src)
	{
		imageRgba32fDownsample2x2NormalMapRef(_dst, _width, _height, _srcPitch, _src);
	}

	void imageSwizzleBgra8Ref(void* _dst, uint32_t _dstPitch, uint32_t _width, uint32_t _height, const void* _src, uint32_t _srcPitch)
	{
		const uint8_t* srcData = (uint8_t*) _src;
		uint8_t* dstData = (uint8_t*)_dst;

		for (uint32_t yy = 0; yy < _height; ++yy, srcData += _srcPitch, dstData += _dstPitch)
		{
			const uint8_t* src = srcData;
			uint8_t* dst = dstData;

			for (uint32_t xx = 0; xx < _width; ++xx, src += 4, dst += 4)
			{
				uint8_t rr = src[0];
				uint8_t gg = src[1];
				uint8_t bb = src[2];
				uint8_t aa = src[3];
				dst[0] = bb;
				dst[1] = gg;
				dst[2] = rr;
				dst[3] = aa;
			}
		}
	}

	void imageSwizzleBgra8(void* _dst, uint32_t _dstPitch, uint32_t _width, uint32_t _height, const void* _src, uint32_t _srcPitch)
	{
		// Test can we do four 4-byte pixels at the time.
		if (0 != (_width&0x3)
		||  _width < 4
		||  !bx::isAligned(_src, 16)
		||  !bx::isAligned(_dst, 16) )
		{
			BX_WARN(false, "Image swizzle is taking slow path.");
			BX_WARN(bx::isAligned(_src, 16), "Source %p is not 16-byte aligned.", _src);
			BX_WARN(bx::isAligned(_dst, 16), "Destination %p is not 16-byte aligned.", _dst);
			BX_WARN(_width < 4, "Image width must be multiple of 4 (width %d).", _width);
			imageSwizzleBgra8Ref(_dst, _dstPitch, _width, _height, _src, _srcPitch);
			return;
		}

		using namespace bx;

		const simd128_t mf0f0 = simd_isplat(0xff00ff00);
		const simd128_t m0f0f = simd_isplat(0x00ff00ff);
		const uint32_t  width = _width/4;

		const uint8_t* srcData = (uint8_t*) _src;
		uint8_t* dstData = (uint8_t*)_dst;

		for (uint32_t yy = 0; yy < _height; ++yy, srcData += _srcPitch, dstData += _dstPitch)
		{
			const uint8_t* src = srcData;
			uint8_t* dst = dstData;

			for (uint32_t xx = 0; xx < width; ++xx, src += 16, dst += 16)
			{
				const simd128_t tabgr = simd_ld(src);
				const simd128_t t00ab = simd_srl(tabgr, 16);
				const simd128_t tgr00 = simd_sll(tabgr, 16);
				const simd128_t tgrab = simd_or(t00ab, tgr00);
				const simd128_t ta0g0 = simd_and(tabgr, mf0f0);
				const simd128_t t0r0b = simd_and(tgrab, m0f0f);
				const simd128_t targb = simd_or(ta0g0, t0r0b);
				simd_st(dst, targb);
			}
		}
	}

	void imageCopy(void* _dst, uint32_t _height, uint32_t _srcPitch, const void* _src, uint32_t _dstPitch)
	{
		const uint32_t pitch = bx::uint32_min(_srcPitch, _dstPitch);
		const uint8_t* src = (uint8_t*)_src;
		uint8_t* dst = (uint8_t*)_dst;

		bx::memCopy(dst, src, pitch, _height, _srcPitch, _dstPitch);
	}

	void imageCopy(void* _dst, uint32_t _width, uint32_t _height, uint32_t _bpp, uint32_t _srcPitch, const void* _src)
	{
		const uint32_t dstPitch = _width*_bpp/8;
		imageCopy(_dst, _height, _srcPitch, _src, dstPitch);
	}

	struct PackUnpack
	{
		PackFn pack;
		UnpackFn unpack;
	};

	static const PackUnpack s_packUnpack[] =
	{
		{ NULL,               NULL                 }, // BC1
		{ NULL,               NULL                 }, // BC2
		{ NULL,               NULL                 }, // BC3
		{ NULL,               NULL                 }, // BC4
		{ NULL,               NULL                 }, // BC5
		{ NULL,               NULL                 }, // BC6H
		{ NULL,               NULL                 }, // BC7
		{ NULL,               NULL                 }, // ETC1
		{ NULL,               NULL                 }, // ETC2
		{ NULL,               NULL                 }, // ETC2A
		{ NULL,               NULL                 }, // ETC2A1
		{ NULL,               NULL                 }, // PTC12
		{ NULL,               NULL                 }, // PTC14
		{ NULL,               NULL                 }, // PTC12A
		{ NULL,               NULL                 }, // PTC14A
		{ NULL,               NULL                 }, // PTC22
		{ NULL,               NULL                 }, // PTC24
		{ NULL,               NULL                 }, // Unknown
		{ NULL,               NULL                 }, // R1
		{ bx::packR8,         bx::unpackR8         }, // A8
		{ bx::packR8,         bx::unpackR8         }, // R8
		{ bx::packR8I,        bx::unpackR8I        }, // R8I
		{ bx::packR8U,        bx::unpackR8U        }, // R8U
		{ bx::packR8S,        bx::unpackR8S        }, // R8S
		{ bx::packR16,        bx::unpackR16        }, // R16
		{ bx::packR16I,       bx::unpackR16I       }, // R16I
		{ bx::packR16U,       bx::unpackR16U       }, // R16U
		{ bx::packR16F,       bx::unpackR16F       }, // R16F
		{ bx::packR16S,       bx::unpackR16S       }, // R16S
		{ bx::packR32I,       bx::unpackR32I       }, // R32I
		{ bx::packR32U,       bx::unpackR32U       }, // R32U
		{ bx::packR32F,       bx::unpackR32F       }, // R32F
		{ bx::packRg8,        bx::unpackRg8        }, // RG8
		{ bx::packRg8I,       bx::unpackRg8I       }, // RG8I
		{ bx::packRg8U,       bx::unpackRg8U       }, // RG8U
		{ bx::packRg8S,       bx::unpackRg8S       }, // RG8S
		{ bx::packRg16,       bx::unpackRg16       }, // RG16
		{ bx::packRg16I,      bx::unpackRg16I      }, // RG16I
		{ bx::packRg16U,      bx::unpackRg16U      }, // RG16U
		{ bx::packRg16F,      bx::unpackRg16F      }, // RG16F
		{ bx::packRg16S,      bx::unpackRg16S      }, // RG16S
		{ bx::packRg32I,      bx::unpackRg32I      }, // RG32I
		{ bx::packRg32U,      bx::unpackRg32U      }, // RG32U
		{ bx::packRg32F,      bx::unpackRg32F      }, // RG32F
		{ bx::packRgb8,       bx::unpackRgb8       }, // RGB8
		{ bx::packRgb8S,      bx::unpackRgb8S      }, // RGB8S
		{ bx::packRgb8I,      bx::unpackRgb8I      }, // RGB8I
		{ bx::packRgb8U,      bx::unpackRgb8U      }, // RGB8U
		{ bx::packRgb9E5F,    bx::unpackRgb9E5F    }, // RGB9E5F
		{ bx::packBgra8,      bx::unpackBgra8      }, // BGRA8
		{ bx::packRgba8,      bx::unpackRgba8      }, // RGBA8
		{ bx::packRgba8I,     bx::unpackRgba8I     }, // RGBA8I
		{ bx::packRgba8U,     bx::unpackRgba8U     }, // RGBA8U
		{ bx::packRgba8S,     bx::unpackRgba8S     }, // RGBA8S
		{ bx::packRgba16,     bx::unpackRgba16     }, // RGBA16
		{ bx::packRgba16I,    bx::unpackRgba16I    }, // RGBA16I
		{ bx::packRgba16U,    bx::unpackRgba16U    }, // RGBA16U
		{ bx::packRgba16F,    bx::unpackRgba16F    }, // RGBA16F
		{ bx::packRgba16S,    bx::unpackRgba16S    }, // RGBA16S
		{ bx::packRgba32I,    bx::unpackRgba32I    }, // RGBA32I
		{ bx::packRgba32U,    bx::unpackRgba32U    }, // RGBA32U
		{ bx::packRgba32F,    bx::unpackRgba32F    }, // RGBA32F
		{ bx::packR5G6B5,     bx::unpackR5G6B5     }, // R5G6B5
		{ bx::packRgba4,      bx::unpackRgba4      }, // RGBA4
		{ bx::packRgb5a1,     bx::unpackRgb5a1     }, // RGB5A1
		{ bx::packRgb10A2,    bx::unpackRgb10A2    }, // RGB10A2
		{ bx::packRG11B10F,   bx::unpackRG11B10F   }, // RG11B10F
		{ NULL,               NULL                 }, // UnknownDepth
		{ bx::packR16,        bx::unpackR16        }, // D16
		{ bx::packR24,        bx::unpackR24        }, // D24
		{ bx::packR24G8,      bx::unpackR24G8      }, // D24S8
		{ NULL,               NULL                 }, // D32
		{ bx::packR16F,       bx::unpackR16F       }, // D16F
		{ NULL,               NULL                 }, // D24F
		{ bx::packR32F,       bx::unpackR32F       }, // D32F
		{ bx::packR8,         bx::unpackR8         }, // D0S8
	};
	BX_STATIC_ASSERT(TextureFormat::Count ==       BX_COUNTOF(s_packUnpack) );

	PackFn getPack(TextureFormat::Enum _format)
	{
		return s_packUnpack[_format].pack;
	}

	UnpackFn getUnpack(TextureFormat::Enum _format)
	{
		return s_packUnpack[_format].unpack;
	}

	bool imageConvert(TextureFormat::Enum _dstFormat, TextureFormat::Enum _srcFormat)
	{
		UnpackFn unpack = s_packUnpack[_srcFormat].unpack;
		PackFn   pack   = s_packUnpack[_dstFormat].pack;
		return NULL != pack
			&& NULL != unpack
			;
	}

	void imageConvert(void* _dst, uint32_t _bpp, PackFn _pack, const void* _src, UnpackFn _unpack, uint32_t _size)
	{
		const uint8_t* src = (uint8_t*)_src;
		uint8_t* dst = (uint8_t*)_dst;

		const uint32_t size = _size * 8 / _bpp;

		for (uint32_t ii = 0; ii < size; ++ii)
		{
			float rgba[4];
			_unpack(rgba, &src[ii*_bpp/8]);
			_pack(&dst[ii*_bpp/8], rgba);
		}
	}

	void imageConvert(void* _dst, uint32_t _dstBpp, PackFn _pack, const void* _src, uint32_t _srcBpp, UnpackFn _unpack, uint32_t _width, uint32_t _height, uint32_t _depth, uint32_t _srcPitch)
	{
		const uint8_t* src = (uint8_t*)_src;
		uint8_t* dst = (uint8_t*)_dst;

		const uint32_t dstPitch = _width * _dstBpp / 8;

		for (uint32_t zz = 0; zz < _depth; ++zz)
		{
			for (uint32_t yy = 0; yy < _height; ++yy, src += _srcPitch, dst += dstPitch)
			{
				for (uint32_t xx = 0; xx < _width; ++xx)
				{
					float rgba[4];
					_unpack(rgba, &src[xx*_srcBpp/8]);
					_pack(&dst[xx*_dstBpp/8], rgba);
				}
			}
		}
	}

	bool imageConvert(void* _dst, TextureFormat::Enum _dstFormat, const void* _src, TextureFormat::Enum _srcFormat, uint32_t _width, uint32_t _height, uint32_t _depth, uint32_t _srcPitch)
	{
		UnpackFn unpack = s_packUnpack[_srcFormat].unpack;
		PackFn   pack   = s_packUnpack[_dstFormat].pack;
		if (NULL == pack
		||  NULL == unpack)
		{
			return false;
		}

		const uint32_t srcBpp = s_imageBlockInfo[_srcFormat].bitsPerPixel;
		const uint32_t dstBpp = s_imageBlockInfo[_dstFormat].bitsPerPixel;
		imageConvert(_dst, dstBpp, pack, _src, srcBpp, unpack, _width, _height, _depth, _srcPitch);

		return true;
	}

	bool imageConvert(void* _dst, TextureFormat::Enum _dstFormat, const void* _src, TextureFormat::Enum _srcFormat, uint32_t _width, uint32_t _height, uint32_t _depth)
	{
		const uint32_t srcBpp = s_imageBlockInfo[_srcFormat].bitsPerPixel;

		if (_dstFormat == _srcFormat)
		{
			bx::memCopy(_dst, _src, _width*_height*_depth*srcBpp/8);
			return true;
		}

		return imageConvert(_dst, _dstFormat, _src, _srcFormat, _width, _height, _depth, _width*srcBpp/8);
	}

	ImageContainer* imageConvert(bx::AllocatorI* _allocator, TextureFormat::Enum _dstFormat, const ImageContainer& _input)
	{
		ImageContainer* output = imageAlloc(_allocator
			, _dstFormat
			, uint16_t(_input.m_width)
			, uint16_t(_input.m_height)
			, uint16_t(_input.m_depth)
			, _input.m_numLayers
			, _input.m_cubeMap
			, 1 < _input.m_numMips
			);

		const uint16_t numSides = _input.m_numLayers * (_input.m_cubeMap ? 6 : 1);

		for (uint16_t side = 0; side < numSides; ++side)
		{
			for (uint8_t lod = 0, num = _input.m_numMips; lod < num; ++lod)
			{
				ImageMip mip;
				if (imageGetRawData(_input, side, lod, _input.m_data, _input.m_size, mip) )
				{
					ImageMip dstMip;
					imageGetRawData(*output, side, lod, output->m_data, output->m_size, dstMip);
					uint8_t* dstData = const_cast<uint8_t*>(dstMip.m_data);

					bool ok = imageConvert(dstData
							, _dstFormat
							, mip.m_data
							, mip.m_format
							, mip.m_width
							, mip.m_height
							, mip.m_depth
							);
					BX_CHECK(ok, "Conversion from %s to %s failed!"
							, getName(_input.m_format)
							, getName(output->m_format)
							);
					BX_UNUSED(ok);
				}
			}
		}

		return output;
	}

	typedef bool (*ParseFn)(ImageContainer&, bx::ReaderSeekerI*, bx::Error*);

	template<uint32_t magicT, ParseFn parseFnT>
	ImageContainer* imageParseT(bx::AllocatorI* _allocator, const void* _src, uint32_t _size, bx::Error* _err)
	{
		bx::MemoryReader reader(_src, _size);

		uint32_t magic;
		bx::read(&reader, magic);

		ImageContainer imageContainer;
		if (magicT != magic
		|| !parseFnT(imageContainer, &reader, _err) )
		{
			return NULL;
		}

		ImageContainer* output = imageAlloc(_allocator
			, imageContainer.m_format
			, uint16_t(imageContainer.m_width)
			, uint16_t(imageContainer.m_height)
			, uint16_t(imageContainer.m_depth)
			, imageContainer.m_numLayers
			, imageContainer.m_cubeMap
			, 1 < imageContainer.m_numMips
			);

		const uint16_t numSides = imageContainer.m_numLayers * (imageContainer.m_cubeMap ? 6 : 1);

		for (uint16_t side = 0; side < numSides; ++side)
		{
			for (uint8_t lod = 0, num = imageContainer.m_numMips; lod < num; ++lod)
			{
				ImageMip dstMip;
				if (imageGetRawData(*output, side, lod, output->m_data, output->m_size, dstMip) )
				{
					ImageMip mip;
					if (imageGetRawData(imageContainer, side, lod, _src, _size, mip) )
					{
						uint8_t* dstData = const_cast<uint8_t*>(dstMip.m_data);
						bx::memCopy(dstData, mip.m_data, mip.m_size);
					}
				}
			}
		}

		return output;
	}

	uint8_t bitRangeConvert(uint32_t _in, uint32_t _from, uint32_t _to)
	{
		using namespace bx;
		uint32_t tmp0   = uint32_sll(1, _to);
		uint32_t tmp1   = uint32_sll(1, _from);
		uint32_t tmp2   = uint32_dec(tmp0);
		uint32_t tmp3   = uint32_dec(tmp1);
		uint32_t tmp4   = uint32_mul(_in, tmp2);
		uint32_t tmp5   = uint32_add(tmp3, tmp4);
		uint32_t tmp6   = uint32_srl(tmp5, _from);
		uint32_t tmp7   = uint32_add(tmp5, tmp6);
		uint32_t result = uint32_srl(tmp7, _from);

		return uint8_t(result);
	}

	void decodeBlockDxt(uint8_t _dst[16*4], const uint8_t _src[8])
	{
		uint8_t colors[4*3];

		uint32_t c0 = _src[0] | (_src[1] << 8);
		colors[0] = bitRangeConvert( (c0>> 0)&0x1f, 5, 8);
		colors[1] = bitRangeConvert( (c0>> 5)&0x3f, 6, 8);
		colors[2] = bitRangeConvert( (c0>>11)&0x1f, 5, 8);

		uint32_t c1 = _src[2] | (_src[3] << 8);
		colors[3] = bitRangeConvert( (c1>> 0)&0x1f, 5, 8);
		colors[4] = bitRangeConvert( (c1>> 5)&0x3f, 6, 8);
		colors[5] = bitRangeConvert( (c1>>11)&0x1f, 5, 8);

		colors[6] = (2*colors[0] + colors[3]) / 3;
		colors[7] = (2*colors[1] + colors[4]) / 3;
		colors[8] = (2*colors[2] + colors[5]) / 3;

		colors[ 9] = (colors[0] + 2*colors[3]) / 3;
		colors[10] = (colors[1] + 2*colors[4]) / 3;
		colors[11] = (colors[2] + 2*colors[5]) / 3;

		for (uint32_t ii = 0, next = 8*4; ii < 16*4; ii += 4, next += 2)
		{
			int idx = ( (_src[next>>3] >> (next & 7) ) & 3) * 3;
			_dst[ii+0] = colors[idx+0];
			_dst[ii+1] = colors[idx+1];
			_dst[ii+2] = colors[idx+2];
		}
	}

	void decodeBlockDxt1(uint8_t _dst[16*4], const uint8_t _src[8])
	{
		uint8_t colors[4*4];

		uint32_t c0 = _src[0] | (_src[1] << 8);
		colors[0] = bitRangeConvert( (c0>> 0)&0x1f, 5, 8);
		colors[1] = bitRangeConvert( (c0>> 5)&0x3f, 6, 8);
		colors[2] = bitRangeConvert( (c0>>11)&0x1f, 5, 8);
		colors[3] = 255;

		uint32_t c1 = _src[2] | (_src[3] << 8);
		colors[4] = bitRangeConvert( (c1>> 0)&0x1f, 5, 8);
		colors[5] = bitRangeConvert( (c1>> 5)&0x3f, 6, 8);
		colors[6] = bitRangeConvert( (c1>>11)&0x1f, 5, 8);
		colors[7] = 255;

		if (c0 > c1)
		{
			colors[ 8] = (2*colors[0] + colors[4]) / 3;
			colors[ 9] = (2*colors[1] + colors[5]) / 3;
			colors[10] = (2*colors[2] + colors[6]) / 3;
			colors[11] = 255;

			colors[12] = (colors[0] + 2*colors[4]) / 3;
			colors[13] = (colors[1] + 2*colors[5]) / 3;
			colors[14] = (colors[2] + 2*colors[6]) / 3;
			colors[15] = 255;
		}
		else
		{
			colors[ 8] = (colors[0] + colors[4]) / 2;
			colors[ 9] = (colors[1] + colors[5]) / 2;
			colors[10] = (colors[2] + colors[6]) / 2;
			colors[11] = 255;

			colors[12] = 0;
			colors[13] = 0;
			colors[14] = 0;
			colors[15] = 0;
		}

		for (uint32_t ii = 0, next = 8*4; ii < 16*4; ii += 4, next += 2)
		{
			int idx = ( (_src[next>>3] >> (next & 7) ) & 3) * 4;
			_dst[ii+0] = colors[idx+0];
			_dst[ii+1] = colors[idx+1];
			_dst[ii+2] = colors[idx+2];
			_dst[ii+3] = colors[idx+3];
		}
	}

	void decodeBlockDxt23A(uint8_t _dst[16*4], const uint8_t _src[8])
	{
		for (uint32_t ii = 0, next = 0; ii < 16*4; ii += 4, next += 4)
		{
			uint32_t c0 = (_src[next>>3] >> (next&7) ) & 0xf;
			_dst[ii] = bitRangeConvert(c0, 4, 8);
		}
	}

	void decodeBlockDxt45A(uint8_t _dst[16*4], const uint8_t _src[8])
	{
		uint8_t alpha[8];
		alpha[0] = _src[0];
		alpha[1] = _src[1];

		if (alpha[0] > alpha[1])
		{
			alpha[2] = (6*alpha[0] + 1*alpha[1]) / 7;
			alpha[3] = (5*alpha[0] + 2*alpha[1]) / 7;
			alpha[4] = (4*alpha[0] + 3*alpha[1]) / 7;
			alpha[5] = (3*alpha[0] + 4*alpha[1]) / 7;
			alpha[6] = (2*alpha[0] + 5*alpha[1]) / 7;
			alpha[7] = (1*alpha[0] + 6*alpha[1]) / 7;
		}
		else
		{
			alpha[2] = (4*alpha[0] + 1*alpha[1]) / 5;
			alpha[3] = (3*alpha[0] + 2*alpha[1]) / 5;
			alpha[4] = (2*alpha[0] + 3*alpha[1]) / 5;
			alpha[5] = (1*alpha[0] + 4*alpha[1]) / 5;
			alpha[6] = 0;
			alpha[7] = 255;
		}

		uint32_t idx0 = _src[2];
		uint32_t idx1 = _src[5];
		idx0 |= uint32_t(_src[3])<<8;
		idx1 |= uint32_t(_src[6])<<8;
		idx0 |= uint32_t(_src[4])<<16;
		idx1 |= uint32_t(_src[7])<<16;
		for (uint32_t ii = 0; ii < 8*4; ii += 4)
		{
			_dst[ii]    = alpha[idx0&7];
			_dst[ii+32] = alpha[idx1&7];
			idx0 >>= 3;
			idx1 >>= 3;
		}
	}

	static const int32_t s_etc1Mod[8][4] =
	{
		{  2,   8,  -2,   -8},
		{  5,  17,  -5,  -17},
		{  9,  29,  -9,  -29},
		{ 13,  42, -13,  -42},
		{ 18,  60, -18,  -60},
		{ 24,  80, -24,  -80},
		{ 33, 106, -33, -106},
		{ 47, 183, -47, -183},
	};

	static const uint8_t s_etc2Mod[8] = { 3, 6, 11, 16, 23, 32, 41, 64 };

	uint8_t uint8_sat(int32_t _a)
	{
		using namespace bx;
		const uint32_t min    = uint32_imin(_a, 255);
		const uint32_t result = uint32_imax(min, 0);
		return (uint8_t)result;
	}

	uint8_t uint8_satadd(int32_t _a, int32_t _b)
	{
		const int32_t add = _a + _b;
		return uint8_sat(add);
	}

	void decodeBlockEtc2ModeT(uint8_t _dst[16*4], const uint8_t _src[8])
	{
		uint8_t rgb[16];

		// 0       1       2       3       4       5       6       7
		// 7654321076543210765432107654321076543210765432107654321076543210
		// ...rr.rrggggbbbbrrrrggggbbbbDDD.mmmmmmmmmmmmmmmmllllllllllllllll
		//    ^            ^           ^   ^               ^
		//    +-- c0       +-- c1      |   +-- msb         +-- lsb
		//                             +-- dist

		rgb[ 0] = ( (_src[0] >> 1) & 0xc)
			    |   (_src[0]       & 0x3)
			    ;
		rgb[ 1] = _src[1] >> 4;
		rgb[ 2] = _src[1] & 0xf;

		rgb[ 8] = _src[2] >> 4;
		rgb[ 9] = _src[2] & 0xf;
		rgb[10] = _src[3] >> 4;

		rgb[ 0] = bitRangeConvert(rgb[ 0], 4, 8);
		rgb[ 1] = bitRangeConvert(rgb[ 1], 4, 8);
		rgb[ 2] = bitRangeConvert(rgb[ 2], 4, 8);
		rgb[ 8] = bitRangeConvert(rgb[ 8], 4, 8);
		rgb[ 9] = bitRangeConvert(rgb[ 9], 4, 8);
		rgb[10] = bitRangeConvert(rgb[10], 4, 8);

		uint8_t dist = (_src[3] >> 1) & 0x7;
		int32_t mod = s_etc2Mod[dist];

		rgb[ 4] = uint8_satadd(rgb[ 8],  mod);
		rgb[ 5] = uint8_satadd(rgb[ 9],  mod);
		rgb[ 6] = uint8_satadd(rgb[10],  mod);

		rgb[12] = uint8_satadd(rgb[ 8], -mod);
		rgb[13] = uint8_satadd(rgb[ 9], -mod);
		rgb[14] = uint8_satadd(rgb[10], -mod);

		uint32_t indexMsb = (_src[4]<<8) | _src[5];
		uint32_t indexLsb = (_src[6]<<8) | _src[7];

		for (uint32_t ii = 0; ii < 16; ++ii)
		{
			const uint32_t idx  = (ii&0xc) | ( (ii & 0x3)<<4);
			const uint32_t lsbi = indexLsb & 1;
			const uint32_t msbi = (indexMsb & 1)<<1;
			const uint32_t pal  = (lsbi | msbi)<<2;

			_dst[idx + 0] = rgb[pal+2];
			_dst[idx + 1] = rgb[pal+1];
			_dst[idx + 2] = rgb[pal+0];
			_dst[idx + 3] = 255;

			indexLsb >>= 1;
			indexMsb >>= 1;
		}
	}

	void decodeBlockEtc2ModeH(uint8_t _dst[16*4], const uint8_t _src[8])
	{
		uint8_t rgb[16];

		// 0       1       2       3       4       5       6       7
		// 7654321076543210765432107654321076543210765432107654321076543210
		// .rrrrggg...gb.bbbrrrrggggbbbbDD.mmmmmmmmmmmmmmmmllllllllllllllll
		//  ^               ^           ^  ^               ^
		//  +-- c0          +-- c1      |  +-- msb         +-- lsb
		//                              +-- dist

		rgb[ 0] =   (_src[0] >> 3) & 0xf;
		rgb[ 1] = ( (_src[0] << 1) & 0xe)
				| ( (_src[1] >> 4) & 0x1)
				;
		rgb[ 2] =   (_src[1]       & 0x8)
				| ( (_src[1] << 1) & 0x6)
				|   (_src[2] >> 7)
				;

		rgb[ 8] =   (_src[2] >> 3) & 0xf;
		rgb[ 9] = ( (_src[2] << 1) & 0xe)
				|   (_src[3] >> 7)
				;
		rgb[10] = (_src[2] >> 3) & 0xf;

		rgb[ 0] = bitRangeConvert(rgb[ 0], 4, 8);
		rgb[ 1] = bitRangeConvert(rgb[ 1], 4, 8);
		rgb[ 2] = bitRangeConvert(rgb[ 2], 4, 8);
		rgb[ 8] = bitRangeConvert(rgb[ 8], 4, 8);
		rgb[ 9] = bitRangeConvert(rgb[ 9], 4, 8);
		rgb[10] = bitRangeConvert(rgb[10], 4, 8);

		uint32_t col0 = uint32_t(rgb[0]<<16) | uint32_t(rgb[1]<<8) | uint32_t(rgb[ 2]);
		uint32_t col1 = uint32_t(rgb[8]<<16) | uint32_t(rgb[9]<<8) | uint32_t(rgb[10]);
		uint8_t  dist = (_src[3] & 0x6) | (col0 >= col1);
		int32_t  mod  = s_etc2Mod[dist];

		rgb[ 4] = uint8_satadd(rgb[ 0], -mod);
		rgb[ 5] = uint8_satadd(rgb[ 1], -mod);
		rgb[ 6] = uint8_satadd(rgb[ 2], -mod);

		rgb[ 0] = uint8_satadd(rgb[ 0],  mod);
		rgb[ 1] = uint8_satadd(rgb[ 1],  mod);
		rgb[ 2] = uint8_satadd(rgb[ 2],  mod);

		rgb[12] = uint8_satadd(rgb[ 8], -mod);
		rgb[13] = uint8_satadd(rgb[ 9], -mod);
		rgb[14] = uint8_satadd(rgb[10], -mod);

		rgb[ 8] = uint8_satadd(rgb[ 8],  mod);
		rgb[ 9] = uint8_satadd(rgb[ 9],  mod);
		rgb[10] = uint8_satadd(rgb[10],  mod);

		uint32_t indexMsb = (_src[4]<<8) | _src[5];
		uint32_t indexLsb = (_src[6]<<8) | _src[7];

		for (uint32_t ii = 0; ii < 16; ++ii)
		{
			const uint32_t idx  = (ii&0xc) | ( (ii & 0x3)<<4);
			const uint32_t lsbi = indexLsb & 1;
			const uint32_t msbi = (indexMsb & 1)<<1;
			const uint32_t pal  = (lsbi | msbi)<<2;

			_dst[idx + 0] = rgb[pal+2];
			_dst[idx + 1] = rgb[pal+1];
			_dst[idx + 2] = rgb[pal+0];
			_dst[idx + 3] = 255;

			indexLsb >>= 1;
			indexMsb >>= 1;
		}
	}

	void decodeBlockEtc2ModePlanar(uint8_t _dst[16*4], const uint8_t _src[8])
	{
		// 0       1       2       3       4       5       6       7
		// 7654321076543210765432107654321076543210765432107654321076543210
		// .rrrrrrg.ggggggb...bb.bbbrrrrr.rgggggggbbbbbbrrrrrrgggggggbbbbbb
		//  ^                       ^                   ^
		//  +-- c0                  +-- cH              +-- cV

		uint8_t c0[3];
		uint8_t cH[3];
		uint8_t cV[3];

		c0[0] =   (_src[0] >> 1) & 0x3f;
		c0[1] = ( (_src[0] & 1) << 6)
			  | ( (_src[1] >> 1) & 0x3f)
			  ;
		c0[2] = ( (_src[1] & 1) << 5)
			  | ( (_src[2] & 0x18) )
			  | ( (_src[2] << 1) & 6)
			  | ( (_src[3] >> 7) )
			  ;

		cH[0] = ( (_src[3] >> 1) & 0x3e)
			  | (_src[3] & 1)
			  ;
		cH[1] = _src[4] >> 1;
		cH[2] = ( (_src[4] & 1) << 5)
			  | (_src[5] >> 3)
			  ;

		cV[0] = ( (_src[5] & 0x7) << 3)
			  | (_src[6] >> 5)
			  ;
		cV[1] = ( (_src[6] & 0x1f) << 2)
			  | (_src[7] >> 5)
			  ;
		cV[2] = _src[7] & 0x3f;

		c0[0] = bitRangeConvert(c0[0], 6, 8);
		c0[1] = bitRangeConvert(c0[1], 7, 8);
		c0[2] = bitRangeConvert(c0[2], 6, 8);

		cH[0] = bitRangeConvert(cH[0], 6, 8);
		cH[1] = bitRangeConvert(cH[1], 7, 8);
		cH[2] = bitRangeConvert(cH[2], 6, 8);

		cV[0] = bitRangeConvert(cV[0], 6, 8);
		cV[1] = bitRangeConvert(cV[1], 7, 8);
		cV[2] = bitRangeConvert(cV[2], 6, 8);

		int16_t dy[3];
		dy[0] = cV[0] - c0[0];
		dy[1] = cV[1] - c0[1];
		dy[2] = cV[2] - c0[2];

		int16_t sx[3];
		sx[0] = int16_t(c0[0])<<2;
		sx[1] = int16_t(c0[1])<<2;
		sx[2] = int16_t(c0[2])<<2;

		int16_t ex[3];
		ex[0] = int16_t(cH[0])<<2;
		ex[1] = int16_t(cH[1])<<2;
		ex[2] = int16_t(cH[2])<<2;

		for (int32_t vv = 0; vv < 4; ++vv)
		{
			int16_t dx[3];
			dx[0] = (ex[0] - sx[0])>>2;
			dx[1] = (ex[1] - sx[1])>>2;
			dx[2] = (ex[2] - sx[2])>>2;

			for (int32_t hh = 0; hh < 4; ++hh)
			{
				const uint32_t idx  = (vv<<4) + (hh<<2);

				_dst[idx + 0] = uint8_sat( (sx[2] + dx[2]*hh)>>2);
				_dst[idx + 1] = uint8_sat( (sx[1] + dx[1]*hh)>>2);
				_dst[idx + 2] = uint8_sat( (sx[0] + dx[0]*hh)>>2);
				_dst[idx + 3] = 255;
			}

			sx[0] += dy[0];
			sx[1] += dy[1];
			sx[2] += dy[2];

			ex[0] += dy[0];
			ex[1] += dy[1];
			ex[2] += dy[2];
		}
	}

	void decodeBlockEtc12(uint8_t _dst[16*4], const uint8_t _src[8])
	{
		bool flipBit = 0 != (_src[3] & 0x1);
		bool diffBit = 0 != (_src[3] & 0x2);

		uint8_t rgb[8];

		if (diffBit)
		{
			rgb[0]  = _src[0] >> 3;
			rgb[1]  = _src[1] >> 3;
			rgb[2]  = _src[2] >> 3;

			int8_t diff[3];
			diff[0] = int8_t( (_src[0] & 0x7)<<5)>>5;
			diff[1] = int8_t( (_src[1] & 0x7)<<5)>>5;
			diff[2] = int8_t( (_src[2] & 0x7)<<5)>>5;

			int8_t rr = rgb[0] + diff[0];
			int8_t gg = rgb[1] + diff[1];
			int8_t bb = rgb[2] + diff[2];

			// Etc2 3-modes
			if (rr < 0 || rr > 31)
			{
				decodeBlockEtc2ModeT(_dst, _src);
				return;
			}
			if (gg < 0 || gg > 31)
			{
				decodeBlockEtc2ModeH(_dst, _src);
				return;
			}
			if (bb < 0 || bb > 31)
			{
				decodeBlockEtc2ModePlanar(_dst, _src);
				return;
			}

			// Etc1
			rgb[0] = bitRangeConvert(rgb[0], 5, 8);
			rgb[1] = bitRangeConvert(rgb[1], 5, 8);
			rgb[2] = bitRangeConvert(rgb[2], 5, 8);
			rgb[4] = bitRangeConvert(rr, 5, 8);
			rgb[5] = bitRangeConvert(gg, 5, 8);
			rgb[6] = bitRangeConvert(bb, 5, 8);
		}
		else
		{
			rgb[0] = _src[0] >> 4;
			rgb[1] = _src[1] >> 4;
			rgb[2] = _src[2] >> 4;

			rgb[4] = _src[0] & 0xf;
			rgb[5] = _src[1] & 0xf;
			rgb[6] = _src[2] & 0xf;

			rgb[0] = bitRangeConvert(rgb[0], 4, 8);
			rgb[1] = bitRangeConvert(rgb[1], 4, 8);
			rgb[2] = bitRangeConvert(rgb[2], 4, 8);
			rgb[4] = bitRangeConvert(rgb[4], 4, 8);
			rgb[5] = bitRangeConvert(rgb[5], 4, 8);
			rgb[6] = bitRangeConvert(rgb[6], 4, 8);
		}

		uint32_t table[2];
		table[0] = (_src[3] >> 5) & 0x7;
		table[1] = (_src[3] >> 2) & 0x7;

		uint32_t indexMsb = (_src[4]<<8) | _src[5];
		uint32_t indexLsb = (_src[6]<<8) | _src[7];

		if (flipBit)
		{
			for (uint32_t ii = 0; ii < 16; ++ii)
			{
				const uint32_t block = (ii>>1)&1;
				const uint32_t color = block<<2;
				const uint32_t idx   = (ii&0xc) | ( (ii & 0x3)<<4);
				const uint32_t lsbi  = indexLsb & 1;
				const uint32_t msbi  = (indexMsb & 1)<<1;
				const  int32_t mod   = s_etc1Mod[table[block] ][lsbi | msbi];

				_dst[idx + 0] = uint8_satadd(rgb[color+2], mod);
				_dst[idx + 1] = uint8_satadd(rgb[color+1], mod);
				_dst[idx + 2] = uint8_satadd(rgb[color+0], mod);
				_dst[idx + 3] = 255;

				indexLsb >>= 1;
				indexMsb >>= 1;
			}
		}
		else
		{
			for (uint32_t ii = 0; ii < 16; ++ii)
			{
				const uint32_t block = ii>>3;
				const uint32_t color = block<<2;
				const uint32_t idx   = (ii&0xc) | ( (ii & 0x3)<<4);
				const uint32_t lsbi  = indexLsb & 1;
				const uint32_t msbi  = (indexMsb & 1)<<1;
				const  int32_t mod   = s_etc1Mod[table[block] ][lsbi | msbi];

				_dst[idx + 0] = uint8_satadd(rgb[color+2], mod);
				_dst[idx + 1] = uint8_satadd(rgb[color+1], mod);
				_dst[idx + 2] = uint8_satadd(rgb[color+0], mod);
				_dst[idx + 3] = 255;

				indexLsb >>= 1;
				indexMsb >>= 1;
			}
		}
	}

	static const uint8_t s_pvrtcFactors[16][4] =
	{
		{  4,  4,  4,  4 },
		{  2,  6,  2,  6 },
		{  8,  0,  8,  0 },
		{  6,  2,  6,  2 },

		{  2,  2,  6,  6 },
		{  1,  3,  3,  9 },
		{  4,  0, 12,  0 },
		{  3,  1,  9,  3 },

		{  8,  8,  0,  0 },
		{  4, 12,  0,  0 },
		{ 16,  0,  0,  0 },
		{ 12,  4,  0,  0 },

		{  6,  6,  2,  2 },
		{  3,  9,  1,  3 },
		{ 12,  0,  4,  0 },
		{  9,  3,  3,  1 },
	};

	static const uint8_t s_pvrtcWeights[8][4] =
	{
		{ 8, 0, 8, 0 },
		{ 5, 3, 5, 3 },
		{ 3, 5, 3, 5 },
		{ 0, 8, 0, 8 },

		{ 8, 0, 8, 0 },
		{ 4, 4, 4, 4 },
		{ 4, 4, 4, 4 },
		{ 0, 8, 0, 8 },
	};

	uint32_t morton2d(uint32_t _x, uint32_t _y)
	{
		using namespace bx;
		const uint32_t tmpx   = uint32_part1by1(_x);
		const uint32_t xbits  = uint32_sll(tmpx, 1);
		const uint32_t ybits  = uint32_part1by1(_y);
		const uint32_t result = uint32_or(xbits, ybits);
		return result;
	}

	uint32_t getColor(const uint8_t _src[8])
	{
		return 0
			| _src[7]<<24
			| _src[6]<<16
			| _src[5]<<8
			| _src[4]
			;
	}

	void decodeBlockPtc14RgbAddA(uint32_t _block, uint32_t* _r, uint32_t* _g, uint32_t* _b, uint8_t _factor)
	{
		if (0 != (_block & (1<<15) ) )
		{
			*_r += bitRangeConvert( (_block >> 10) & 0x1f, 5, 8) * _factor;
			*_g += bitRangeConvert( (_block >>  5) & 0x1f, 5, 8) * _factor;
			*_b += bitRangeConvert( (_block >>  1) & 0x0f, 4, 8) * _factor;
		}
		else
		{
			*_r += bitRangeConvert( (_block >>  8) &  0xf, 4, 8) * _factor;
			*_g += bitRangeConvert( (_block >>  4) &  0xf, 4, 8) * _factor;
			*_b += bitRangeConvert( (_block >>  1) &  0x7, 3, 8) * _factor;
		}
	}

	void decodeBlockPtc14RgbAddB(uint32_t _block, uint32_t* _r, uint32_t* _g, uint32_t* _b, uint8_t _factor)
	{
		if (0 != (_block & (1<<31) ) )
		{
			*_r += bitRangeConvert( (_block >> 26) & 0x1f, 5, 8) * _factor;
			*_g += bitRangeConvert( (_block >> 21) & 0x1f, 5, 8) * _factor;
			*_b += bitRangeConvert( (_block >> 16) & 0x1f, 5, 8) * _factor;
		}
		else
		{
			*_r += bitRangeConvert( (_block >> 24) &  0xf, 4, 8) * _factor;
			*_g += bitRangeConvert( (_block >> 20) &  0xf, 4, 8) * _factor;
			*_b += bitRangeConvert( (_block >> 16) &  0xf, 4, 8) * _factor;
		}
	}

	void decodeBlockPtc14(uint8_t _dst[16*4], const uint8_t* _src, uint32_t _x, uint32_t _y, uint32_t _width, uint32_t _height)
	{
		// 0       1       2       3       4       5       6       7
		// 7654321076543210765432107654321076543210765432107654321076543210
		// mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmyrrrrrgggggbbbbbxrrrrrgggggbbbbp
		// ^                               ^^              ^^             ^
		// +-- modulation data             |+- B color     |+- A color    |
		//                                 +-- B opaque    +-- A opaque   |
		//                                           alpha punchthrough --+

		const uint8_t* bc = &_src[morton2d(_x, _y) * 8];

		uint32_t mod = 0
			| bc[3]<<24
			| bc[2]<<16
			| bc[1]<<8
			| bc[0]
			;

		const bool punchthrough = !!(bc[7] & 1);
		const uint8_t* weightTable = s_pvrtcWeights[4 * punchthrough];
		const uint8_t* factorTable = s_pvrtcFactors[0];

		for (int yy = 0; yy < 4; ++yy)
		{
			const uint32_t yOffset = (yy < 2) ? -1 : 0;
			const uint32_t y0 = (_y + yOffset) % _height;
			const uint32_t y1 = (y0 +       1) % _height;

			for (int xx = 0; xx < 4; ++xx)
			{
				const uint32_t xOffset = (xx < 2) ? -1 : 0;
				const uint32_t x0 = (_x + xOffset) % _width;
				const uint32_t x1 = (x0 +       1) % _width;

				const uint32_t bc0 = getColor(&_src[morton2d(x0, y0) * 8]);
				const uint32_t bc1 = getColor(&_src[morton2d(x1, y0) * 8]);
				const uint32_t bc2 = getColor(&_src[morton2d(x0, y1) * 8]);
				const uint32_t bc3 = getColor(&_src[morton2d(x1, y1) * 8]);

				const uint8_t f0 = factorTable[0];
				const uint8_t f1 = factorTable[1];
				const uint8_t f2 = factorTable[2];
				const uint8_t f3 = factorTable[3];

				uint32_t ar = 0, ag = 0, ab = 0;
				decodeBlockPtc14RgbAddA(bc0, &ar, &ag, &ab, f0);
				decodeBlockPtc14RgbAddA(bc1, &ar, &ag, &ab, f1);
				decodeBlockPtc14RgbAddA(bc2, &ar, &ag, &ab, f2);
				decodeBlockPtc14RgbAddA(bc3, &ar, &ag, &ab, f3);

				uint32_t br = 0, bg = 0, bb = 0;
				decodeBlockPtc14RgbAddB(bc0, &br, &bg, &bb, f0);
				decodeBlockPtc14RgbAddB(bc1, &br, &bg, &bb, f1);
				decodeBlockPtc14RgbAddB(bc2, &br, &bg, &bb, f2);
				decodeBlockPtc14RgbAddB(bc3, &br, &bg, &bb, f3);

				const uint8_t* weight = &weightTable[(mod & 3)*4];
				const uint8_t wa = weight[0];
				const uint8_t wb = weight[1];

				_dst[(yy*4 + xx)*4+0] = uint8_t( (ab * wa + bb * wb) >> 7);
				_dst[(yy*4 + xx)*4+1] = uint8_t( (ag * wa + bg * wb) >> 7);
				_dst[(yy*4 + xx)*4+2] = uint8_t( (ar * wa + br * wb) >> 7);
				_dst[(yy*4 + xx)*4+3] = 255;

				mod >>= 2;
				factorTable += 4;
			}
		}
	}

	void decodeBlockPtc14ARgbaAddA(uint32_t _block, uint32_t* _r, uint32_t* _g, uint32_t* _b, uint32_t* _a, uint8_t _factor)
	{
		if (0 != (_block & (1<<15) ) )
		{
			*_r += bitRangeConvert( (_block >> 10) & 0x1f, 5, 8) * _factor;
			*_g += bitRangeConvert( (_block >>  5) & 0x1f, 5, 8) * _factor;
			*_b += bitRangeConvert( (_block >>  1) & 0x0f, 4, 8) * _factor;
			*_a += 255 * _factor;
		}
		else
		{
			*_r += bitRangeConvert( (_block >>  8) &  0xf, 4, 8) * _factor;
			*_g += bitRangeConvert( (_block >>  4) &  0xf, 4, 8) * _factor;
			*_b += bitRangeConvert( (_block >>  1) &  0x7, 3, 8) * _factor;
			*_a += bitRangeConvert( (_block >> 12) &  0x7, 3, 8) * _factor;
		}
	}

	void decodeBlockPtc14ARgbaAddB(uint32_t _block, uint32_t* _r, uint32_t* _g, uint32_t* _b, uint32_t* _a, uint8_t _factor)
	{
		if (0 != (_block & (1<<31) ) )
		{
			*_r += bitRangeConvert( (_block >> 26) & 0x1f, 5, 8) * _factor;
			*_g += bitRangeConvert( (_block >> 21) & 0x1f, 5, 8) * _factor;
			*_b += bitRangeConvert( (_block >> 16) & 0x1f, 5, 8) * _factor;
			*_a += 255 * _factor;
		}
		else
		{
			*_r += bitRangeConvert( (_block >> 24) &  0xf, 4, 8) * _factor;
			*_g += bitRangeConvert( (_block >> 20) &  0xf, 4, 8) * _factor;
			*_b += bitRangeConvert( (_block >> 16) &  0xf, 4, 8) * _factor;
			*_a += bitRangeConvert( (_block >> 28) &  0x7, 3, 8) * _factor;
		}
	}

	void decodeBlockPtc14A(uint8_t _dst[16*4], const uint8_t* _src, uint32_t _x, uint32_t _y, uint32_t _width, uint32_t _height)
	{
		// 0       1       2       3       4       5       6       7
		// 7654321076543210765432107654321076543210765432107654321076543210
		// mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmyrrrrrgggggbbbbbxrrrrrgggggbbbbp
		// ^                               ^^              ^^             ^
		// +-- modulation data             |+- B color     |+- A color    |
		//                                 +-- B opaque    +-- A opaque   |
		//                                           alpha punchthrough --+

		const uint8_t* bc = &_src[morton2d(_x, _y) * 8];

		uint32_t mod = 0
			| bc[3]<<24
			| bc[2]<<16
			| bc[1]<<8
			| bc[0]
			;

		const bool punchthrough = !!(bc[7] & 1);
		const uint8_t* weightTable = s_pvrtcWeights[4 * punchthrough];
		const uint8_t* factorTable = s_pvrtcFactors[0];

		for (int yy = 0; yy < 4; ++yy)
		{
			const uint32_t yOffset = (yy < 2) ? -1 : 0;
			const uint32_t y0 = (_y + yOffset) % _height;
			const uint32_t y1 = (y0 +       1) % _height;

			for (int xx = 0; xx < 4; ++xx)
			{
				const uint32_t xOffset = (xx < 2) ? -1 : 0;
				const uint32_t x0 = (_x + xOffset) % _width;
				const uint32_t x1 = (x0 +       1) % _width;

				const uint32_t bc0 = getColor(&_src[morton2d(x0, y0) * 8]);
				const uint32_t bc1 = getColor(&_src[morton2d(x1, y0) * 8]);
				const uint32_t bc2 = getColor(&_src[morton2d(x0, y1) * 8]);
				const uint32_t bc3 = getColor(&_src[morton2d(x1, y1) * 8]);

				const uint8_t f0 = factorTable[0];
				const uint8_t f1 = factorTable[1];
				const uint8_t f2 = factorTable[2];
				const uint8_t f3 = factorTable[3];

				uint32_t ar = 0, ag = 0, ab = 0, aa = 0;
				decodeBlockPtc14ARgbaAddA(bc0, &ar, &ag, &ab, &aa, f0);
				decodeBlockPtc14ARgbaAddA(bc1, &ar, &ag, &ab, &aa, f1);
				decodeBlockPtc14ARgbaAddA(bc2, &ar, &ag, &ab, &aa, f2);
				decodeBlockPtc14ARgbaAddA(bc3, &ar, &ag, &ab, &aa, f3);

				uint32_t br = 0, bg = 0, bb = 0, ba = 0;
				decodeBlockPtc14ARgbaAddB(bc0, &br, &bg, &bb, &ba, f0);
				decodeBlockPtc14ARgbaAddB(bc1, &br, &bg, &bb, &ba, f1);
				decodeBlockPtc14ARgbaAddB(bc2, &br, &bg, &bb, &ba, f2);
				decodeBlockPtc14ARgbaAddB(bc3, &br, &bg, &bb, &ba, f3);

				const uint8_t* weight = &weightTable[(mod & 3)*4];
				const uint8_t wa = weight[0];
				const uint8_t wb = weight[1];
				const uint8_t wc = weight[2];
				const uint8_t wd = weight[3];

				_dst[(yy*4 + xx)*4+0] = uint8_t( (ab * wa + bb * wb) >> 7);
				_dst[(yy*4 + xx)*4+1] = uint8_t( (ag * wa + bg * wb) >> 7);
				_dst[(yy*4 + xx)*4+2] = uint8_t( (ar * wa + br * wb) >> 7);
				_dst[(yy*4 + xx)*4+3] = uint8_t( (aa * wc + ba * wd) >> 7);

				mod >>= 2;
				factorTable += 4;
			}
		}
	}

	ImageContainer* imageAlloc(bx::AllocatorI* _allocator, TextureFormat::Enum _format, uint16_t _width, uint16_t _height, uint16_t _depth, uint16_t _numLayers, bool _cubeMap, bool _hasMips, const void* _data)
	{
		const ImageBlockInfo& blockInfo = getBlockInfo(_format);
		const uint16_t blockWidth  = blockInfo.blockWidth;
		const uint16_t blockHeight = blockInfo.blockHeight;
		const uint16_t minBlockX   = blockInfo.minBlockX;
		const uint16_t minBlockY   = blockInfo.minBlockY;

		_width     = bx::uint16_max(blockWidth  * minBlockX, ( (_width  + blockWidth  - 1) / blockWidth)*blockWidth);
		_height    = bx::uint16_max(blockHeight * minBlockY, ( (_height + blockHeight - 1) / blockHeight)*blockHeight);
		_depth     = bx::uint16_max(1, _depth);
		_numLayers = bx::uint16_max(1, _numLayers);

		const uint8_t numMips = _hasMips ? imageGetNumMips(_format, _width, _height, _depth) : 1;
		uint32_t size = imageGetSize(NULL, _width, _height, _depth, _cubeMap, _hasMips, _numLayers, _format);

		ImageContainer* imageContainer = (ImageContainer*)BX_ALLOC(_allocator, size + sizeof(ImageContainer) );

		imageContainer->m_allocator   = _allocator;
		imageContainer->m_data        = imageContainer + 1;
		imageContainer->m_format      = _format;
		imageContainer->m_orientation = Orientation::R0;
		imageContainer->m_size        = size;
		imageContainer->m_offset      = 0;
		imageContainer->m_width       = _width;
		imageContainer->m_height      = _height;
		imageContainer->m_depth       = _depth;
		imageContainer->m_numLayers   = _numLayers;
		imageContainer->m_numMips     = numMips;
		imageContainer->m_hasAlpha    = false;
		imageContainer->m_cubeMap     = _cubeMap;
		imageContainer->m_ktx         = false;
		imageContainer->m_ktxLE       = false;
		imageContainer->m_srgb        = false;

		if (NULL != _data)
		{
			bx::memCopy(imageContainer->m_data, _data, imageContainer->m_size);
		}

		return imageContainer;
	}

	void imageFree(ImageContainer* _imageContainer)
	{
		BX_FREE(_imageContainer->m_allocator, _imageContainer);
	}

// DDS
#define DDS_MAGIC             BX_MAKEFOURCC('D', 'D', 'S', ' ')
#define DDS_HEADER_SIZE       124

#define DDS_DXT1 BX_MAKEFOURCC('D', 'X', 'T', '1')
#define DDS_DXT2 BX_MAKEFOURCC('D', 'X', 'T', '2')
#define DDS_DXT3 BX_MAKEFOURCC('D', 'X', 'T', '3')
#define DDS_DXT4 BX_MAKEFOURCC('D', 'X', 'T', '4')
#define DDS_DXT5 BX_MAKEFOURCC('D', 'X', 'T', '5')
#define DDS_ATI1 BX_MAKEFOURCC('A', 'T', 'I', '1')
#define DDS_BC4U BX_MAKEFOURCC('B', 'C', '4', 'U')
#define DDS_ATI2 BX_MAKEFOURCC('A', 'T', 'I', '2')
#define DDS_BC5U BX_MAKEFOURCC('B', 'C', '5', 'U')
#define DDS_DX10 BX_MAKEFOURCC('D', 'X', '1', '0')

#define DDS_A8R8G8B8       21
#define DDS_R5G6B5         23
#define DDS_A1R5G5B5       25
#define DDS_A4R4G4B4       26
#define DDS_A2B10G10R10    31
#define DDS_G16R16         34
#define DDS_A2R10G10B10    35
#define DDS_A16B16G16R16   36
#define DDS_A8L8           51
#define DDS_R16F           111
#define DDS_G16R16F        112
#define DDS_A16B16G16R16F  113
#define DDS_R32F           114
#define DDS_G32R32F        115
#define DDS_A32B32G32R32F  116

#define DDS_FORMAT_R32G32B32A32_FLOAT  2
#define DDS_FORMAT_R32G32B32A32_UINT   3
#define DDS_FORMAT_R16G16B16A16_FLOAT  10
#define DDS_FORMAT_R16G16B16A16_UNORM  11
#define DDS_FORMAT_R16G16B16A16_UINT   12
#define DDS_FORMAT_R32G32_FLOAT        16
#define DDS_FORMAT_R32G32_UINT         17
#define DDS_FORMAT_R10G10B10A2_UNORM   24
#define DDS_FORMAT_R11G11B10_FLOAT     26
#define DDS_FORMAT_R8G8B8A8_UNORM      28
#define DDS_FORMAT_R8G8B8A8_UNORM_SRGB 29
#define DDS_FORMAT_R16G16_FLOAT        34
#define DDS_FORMAT_R16G16_UNORM        35
#define DDS_FORMAT_R32_FLOAT           41
#define DDS_FORMAT_R32_UINT            42
#define DDS_FORMAT_R8G8_UNORM          49
#define DDS_FORMAT_R16_FLOAT           54
#define DDS_FORMAT_R16_UNORM           56
#define DDS_FORMAT_R8_UNORM            61
#define DDS_FORMAT_R1_UNORM            66
#define DDS_FORMAT_BC1_UNORM           71
#define DDS_FORMAT_BC1_UNORM_SRGB      72
#define DDS_FORMAT_BC2_UNORM           74
#define DDS_FORMAT_BC2_UNORM_SRGB      75
#define DDS_FORMAT_BC3_UNORM           77
#define DDS_FORMAT_BC3_UNORM_SRGB      78
#define DDS_FORMAT_BC4_UNORM           80
#define DDS_FORMAT_BC5_UNORM           83
#define DDS_FORMAT_B5G6R5_UNORM        85
#define DDS_FORMAT_B5G5R5A1_UNORM      86
#define DDS_FORMAT_B8G8R8A8_UNORM      87
#define DDS_FORMAT_B8G8R8A8_UNORM_SRGB 91
#define DDS_FORMAT_BC6H_SF16           96
#define DDS_FORMAT_BC7_UNORM           98
#define DDS_FORMAT_BC7_UNORM_SRGB      99
#define DDS_FORMAT_B4G4R4A4_UNORM      115

#define DDS_DX10_DIMENSION_TEXTURE2D 3
#define DDS_DX10_DIMENSION_TEXTURE3D 4
#define DDS_DX10_MISC_TEXTURECUBE    4

#define DDSD_CAPS                  0x00000001
#define DDSD_HEIGHT                0x00000002
#define DDSD_WIDTH                 0x00000004
#define DDSD_PITCH                 0x00000008
#define DDSD_PIXELFORMAT           0x00001000
#define DDSD_MIPMAPCOUNT           0x00020000
#define DDSD_LINEARSIZE            0x00080000
#define DDSD_DEPTH                 0x00800000

#define DDPF_ALPHAPIXELS           0x00000001
#define DDPF_ALPHA                 0x00000002
#define DDPF_FOURCC                0x00000004
#define DDPF_INDEXED               0x00000020
#define DDPF_RGB                   0x00000040
#define DDPF_YUV                   0x00000200
#define DDPF_LUMINANCE             0x00020000
#define DDPF_BUMPDUDV              0x00080000

#define DDSCAPS_COMPLEX            0x00000008
#define DDSCAPS_TEXTURE            0x00001000
#define DDSCAPS_MIPMAP             0x00400000

#define DDSCAPS2_VOLUME            0x00200000
#define DDSCAPS2_CUBEMAP           0x00000200
#define DDSCAPS2_CUBEMAP_POSITIVEX 0x00000400
#define DDSCAPS2_CUBEMAP_NEGATIVEX 0x00000800
#define DDSCAPS2_CUBEMAP_POSITIVEY 0x00001000
#define DDSCAPS2_CUBEMAP_NEGATIVEY 0x00002000
#define DDSCAPS2_CUBEMAP_POSITIVEZ 0x00004000
#define DDSCAPS2_CUBEMAP_NEGATIVEZ 0x00008000

#define DSCAPS2_CUBEMAP_ALLSIDES (0      \
			| DDSCAPS2_CUBEMAP_POSITIVEX \
			| DDSCAPS2_CUBEMAP_NEGATIVEX \
			| DDSCAPS2_CUBEMAP_POSITIVEY \
			| DDSCAPS2_CUBEMAP_NEGATIVEY \
			| DDSCAPS2_CUBEMAP_POSITIVEZ \
			| DDSCAPS2_CUBEMAP_NEGATIVEZ \
			)

	struct TranslateDdsFormat
	{
		uint32_t m_format;
		TextureFormat::Enum m_textureFormat;
		bool m_srgb;
	};

	static const TranslateDdsFormat s_translateDdsFourccFormat[] =
	{
		{ DDS_DXT1,                  TextureFormat::BC1,     false },
		{ DDS_DXT2,                  TextureFormat::BC2,     false },
		{ DDS_DXT3,                  TextureFormat::BC2,     false },
		{ DDS_DXT4,                  TextureFormat::BC3,     false },
		{ DDS_DXT5,                  TextureFormat::BC3,     false },
		{ DDS_ATI1,                  TextureFormat::BC4,     false },
		{ DDS_BC4U,                  TextureFormat::BC4,     false },
		{ DDS_ATI2,                  TextureFormat::BC5,     false },
		{ DDS_BC5U,                  TextureFormat::BC5,     false },
		{ DDS_A16B16G16R16,          TextureFormat::RGBA16,  false },
		{ DDS_A16B16G16R16F,         TextureFormat::RGBA16F, false },
		{ DDPF_RGB|DDPF_ALPHAPIXELS, TextureFormat::BGRA8,   false },
		{ DDPF_INDEXED,              TextureFormat::R8,      false },
		{ DDPF_LUMINANCE,            TextureFormat::R8,      false },
		{ DDPF_ALPHA,                TextureFormat::R8,      false },
		{ DDS_R16F,                  TextureFormat::R16F,    false },
		{ DDS_R32F,                  TextureFormat::R32F,    false },
		{ DDS_A8L8,                  TextureFormat::RG8,     false },
		{ DDS_G16R16,                TextureFormat::RG16,    false },
		{ DDS_G16R16F,               TextureFormat::RG16F,   false },
		{ DDS_G32R32F,               TextureFormat::RG32F,   false },
		{ DDS_A8R8G8B8,              TextureFormat::BGRA8,   false },
		{ DDS_A16B16G16R16,          TextureFormat::RGBA16,  false },
		{ DDS_A16B16G16R16F,         TextureFormat::RGBA16F, false },
		{ DDS_A32B32G32R32F,         TextureFormat::RGBA32F, false },
		{ DDS_R5G6B5,                TextureFormat::R5G6B5,  false },
		{ DDS_A4R4G4B4,              TextureFormat::RGBA4,   false },
		{ DDS_A1R5G5B5,              TextureFormat::RGB5A1,  false },
		{ DDS_A2B10G10R10,           TextureFormat::RGB10A2, false },
	};

	static const TranslateDdsFormat s_translateDxgiFormat[] =
	{
		{ DDS_FORMAT_BC1_UNORM,           TextureFormat::BC1,        false },
		{ DDS_FORMAT_BC1_UNORM_SRGB,      TextureFormat::BC1,        true  },
		{ DDS_FORMAT_BC2_UNORM,           TextureFormat::BC2,        false },
		{ DDS_FORMAT_BC2_UNORM_SRGB,      TextureFormat::BC2,        true  },
		{ DDS_FORMAT_BC3_UNORM,           TextureFormat::BC3,        false },
		{ DDS_FORMAT_BC3_UNORM_SRGB,      TextureFormat::BC3,        true  },
		{ DDS_FORMAT_BC4_UNORM,           TextureFormat::BC4,        false },
		{ DDS_FORMAT_BC5_UNORM,           TextureFormat::BC5,        false },
		{ DDS_FORMAT_BC6H_SF16,           TextureFormat::BC6H,       false },
		{ DDS_FORMAT_BC7_UNORM,           TextureFormat::BC7,        false },
		{ DDS_FORMAT_BC7_UNORM_SRGB,      TextureFormat::BC7,        true  },

		{ DDS_FORMAT_R1_UNORM,            TextureFormat::R1,         false },
		{ DDS_FORMAT_R8_UNORM,            TextureFormat::R8,         false },
		{ DDS_FORMAT_R16_UNORM,           TextureFormat::R16,        false },
		{ DDS_FORMAT_R16_FLOAT,           TextureFormat::R16F,       false },
		{ DDS_FORMAT_R32_UINT,            TextureFormat::R32U,       false },
		{ DDS_FORMAT_R32_FLOAT,           TextureFormat::R32F,       false },
		{ DDS_FORMAT_R8G8_UNORM,          TextureFormat::RG8,        false },
		{ DDS_FORMAT_R16G16_UNORM,        TextureFormat::RG16,       false },
		{ DDS_FORMAT_R16G16_FLOAT,        TextureFormat::RG16F,      false },
		{ DDS_FORMAT_R32G32_UINT,         TextureFormat::RG32U,      false },
		{ DDS_FORMAT_R32G32_FLOAT,        TextureFormat::RG32F,      false },
		{ DDS_FORMAT_B8G8R8A8_UNORM,      TextureFormat::BGRA8,      false },
		{ DDS_FORMAT_B8G8R8A8_UNORM_SRGB, TextureFormat::BGRA8,      true  },
		{ DDS_FORMAT_R8G8B8A8_UNORM,      TextureFormat::RGBA8,      false },
		{ DDS_FORMAT_R8G8B8A8_UNORM_SRGB, TextureFormat::RGBA8,      true  },
		{ DDS_FORMAT_R16G16B16A16_UNORM,  TextureFormat::RGBA16,     false },
		{ DDS_FORMAT_R16G16B16A16_FLOAT,  TextureFormat::RGBA16F,    false },
		{ DDS_FORMAT_R32G32B32A32_UINT,   TextureFormat::RGBA32U,    false },
		{ DDS_FORMAT_R32G32B32A32_FLOAT,  TextureFormat::RGBA32F,    false },
		{ DDS_FORMAT_B5G6R5_UNORM,        TextureFormat::R5G6B5,     false },
		{ DDS_FORMAT_B4G4R4A4_UNORM,      TextureFormat::RGBA4,      false },
		{ DDS_FORMAT_B5G5R5A1_UNORM,      TextureFormat::RGB5A1,     false },
		{ DDS_FORMAT_R10G10B10A2_UNORM,   TextureFormat::RGB10A2,    false },
		{ DDS_FORMAT_R11G11B10_FLOAT,     TextureFormat::RG11B10F,   false },
	};

	struct TranslateDdsPixelFormat
	{
		uint32_t m_bitCount;
		uint32_t m_flags;
		uint32_t m_bitmask[4];
		TextureFormat::Enum m_textureFormat;
	};

	static const TranslateDdsPixelFormat s_translateDdsPixelFormat[] =
	{
		{  8, DDPF_LUMINANCE,            { 0x000000ff, 0x00000000, 0x00000000, 0x00000000 }, TextureFormat::R8      },
		{ 16, DDPF_BUMPDUDV,             { 0x000000ff, 0x0000ff00, 0x00000000, 0x00000000 }, TextureFormat::RG8S    },
		{ 16, DDPF_RGB,                  { 0x0000ffff, 0x00000000, 0x00000000, 0x00000000 }, TextureFormat::R16U    },
		{ 16, DDPF_RGB|DDPF_ALPHAPIXELS, { 0x00000f00, 0x000000f0, 0x0000000f, 0x0000f000 }, TextureFormat::RGBA4   },
		{ 16, DDPF_RGB,                  { 0x0000f800, 0x000007e0, 0x0000001f, 0x00000000 }, TextureFormat::R5G6B5  },
		{ 16, DDPF_RGB,                  { 0x00007c00, 0x000003e0, 0x0000001f, 0x00008000 }, TextureFormat::RGB5A1  },
		{ 24, DDPF_RGB,                  { 0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000 }, TextureFormat::RGB8    },
		{ 32, DDPF_RGB,                  { 0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000 }, TextureFormat::BGRA8   },
		{ 32, DDPF_RGB|DDPF_ALPHAPIXELS, { 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000 }, TextureFormat::RGBA8   },
		{ 32, DDPF_BUMPDUDV,             { 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000 }, TextureFormat::RGBA8S  },
		{ 32, DDPF_RGB,                  { 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000 }, TextureFormat::BGRA8   },
		{ 32, DDPF_RGB|DDPF_ALPHAPIXELS, { 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000 }, TextureFormat::BGRA8   }, // D3DFMT_A8R8G8B8
		{ 32, DDPF_RGB|DDPF_ALPHAPIXELS, { 0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000 }, TextureFormat::BGRA8   }, // D3DFMT_X8R8G8B8
		{ 32, DDPF_RGB|DDPF_ALPHAPIXELS, { 0x000003ff, 0x000ffc00, 0x3ff00000, 0xc0000000 }, TextureFormat::RGB10A2 },
		{ 32, DDPF_RGB,                  { 0x0000ffff, 0xffff0000, 0x00000000, 0x00000000 }, TextureFormat::RG16    },
		{ 32, DDPF_BUMPDUDV,             { 0x0000ffff, 0xffff0000, 0x00000000, 0x00000000 }, TextureFormat::RG16S   },
		{ 32, DDPF_RGB,                  { 0xffffffff, 0x00000000, 0x00000000, 0x00000000 }, TextureFormat::R32U    },
	};

	bool imageParseDds(ImageContainer& _imageContainer, bx::ReaderSeekerI* _reader, bx::Error* _err)
	{
		BX_ERROR_SCOPE(_err);
		int32_t total = 0;

		uint32_t headerSize;
		total += bx::read(_reader, headerSize, _err);

		if (!_err->isOk()
		||  headerSize < DDS_HEADER_SIZE)
		{
			return false;
		}

		uint32_t flags;
		total += bx::read(_reader, flags, _err);

		if (!_err->isOk() )
		{
			return false;
		}

		if ( (flags & (DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH|DDSD_PIXELFORMAT) ) != (DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH|DDSD_PIXELFORMAT) )
		{
			BX_ERROR_SET(_err, BIMG_ERROR, "DDS: Invalid flags.");
			return false;
		}

		uint32_t height;
		total += bx::read(_reader, height, _err);

		uint32_t width;
		total += bx::read(_reader, width, _err);

		uint32_t pitch;
		total += bx::read(_reader, pitch, _err);

		uint32_t depth;
		total += bx::read(_reader, depth, _err);

		uint32_t mips;
		total += bx::read(_reader, mips, _err);

		bx::skip(_reader, 44); // reserved
		total += 44;

		uint32_t pixelFormatSize;
		total += bx::read(_reader, pixelFormatSize, _err);

		uint32_t pixelFlags;
		total += bx::read(_reader, pixelFlags, _err);

		uint32_t fourcc;
		total += bx::read(_reader, fourcc, _err);

		uint32_t bitCount;
		total += bx::read(_reader, bitCount, _err);

		uint32_t bitmask[4];
		total += bx::read(_reader, bitmask, sizeof(bitmask), _err);

		uint32_t caps[4];
		total += bx::read(_reader, caps, _err);

		bx::skip(_reader, 4);
		total += 4; // reserved

		if (!_err->isOk() )
		{
			return false;
		}

		uint32_t dxgiFormat = 0;
		uint32_t arraySize = 1;
		if (DDPF_FOURCC == pixelFlags
		&&  DDS_DX10    == fourcc)
		{
			total += bx::read(_reader, dxgiFormat, _err);

			uint32_t dims;
			total += bx::read(_reader, dims, _err);

			uint32_t miscFlags;
			total += bx::read(_reader, miscFlags, _err);

			total += bx::read(_reader, arraySize, _err);

			uint32_t miscFlags2;
			total += bx::read(_reader, miscFlags2, _err);
		}

		if (!_err->isOk() )
		{
			return false;
		}

		if ( (caps[0] & DDSCAPS_TEXTURE) == 0)
		{
			BX_ERROR_SET(_err, BIMG_ERROR, "DDS: Unsupported caps.");
			return false;
		}

		bool cubeMap = 0 != (caps[1] & DDSCAPS2_CUBEMAP);
		if (cubeMap)
		{
			if ( (caps[1] & DSCAPS2_CUBEMAP_ALLSIDES) != DSCAPS2_CUBEMAP_ALLSIDES)
			{
				// partial cube map is not supported.
				BX_ERROR_SET(_err, BIMG_ERROR, "DDS: Incomplete cubemap.");
				return false;
			}
		}

		TextureFormat::Enum format = TextureFormat::Unknown;
		bool hasAlpha = pixelFlags & DDPF_ALPHAPIXELS;
		bool srgb = false;

		if (dxgiFormat == 0)
		{
			if (DDPF_FOURCC == (pixelFlags & DDPF_FOURCC) )
			{
				for (uint32_t ii = 0; ii < BX_COUNTOF(s_translateDdsFourccFormat); ++ii)
				{
					if (s_translateDdsFourccFormat[ii].m_format == fourcc)
					{
						format = s_translateDdsFourccFormat[ii].m_textureFormat;
						break;
					}
				}
			}
			else
			{
				for (uint32_t ii = 0; ii < BX_COUNTOF(s_translateDdsPixelFormat); ++ii)
				{
					const TranslateDdsPixelFormat& pf = s_translateDdsPixelFormat[ii];
					if (pf.m_bitCount   == bitCount
					&&  pf.m_flags      == pixelFlags
					&&  pf.m_bitmask[0] == bitmask[0]
					&&  pf.m_bitmask[1] == bitmask[1]
					&&  pf.m_bitmask[2] == bitmask[2]
					&&  pf.m_bitmask[3] == bitmask[3])
					{
						format = pf.m_textureFormat;
						break;
					}
				}
			}
		}
		else
		{
			for (uint32_t ii = 0; ii < BX_COUNTOF(s_translateDxgiFormat); ++ii)
			{
				if (s_translateDxgiFormat[ii].m_format == dxgiFormat)
				{
					format = s_translateDxgiFormat[ii].m_textureFormat;
					srgb = s_translateDxgiFormat[ii].m_srgb;
					break;
				}
			}
		}

		if (TextureFormat::Unknown == format)
		{
			BX_ERROR_SET(_err, BIMG_ERROR, "DDS: Unknown texture format.");
			return false;
		}

		_imageContainer.m_allocator   = NULL;
		_imageContainer.m_data        = NULL;
		_imageContainer.m_size        = 0;
		_imageContainer.m_offset      = (uint32_t)bx::seek(_reader);
		_imageContainer.m_width       = width;
		_imageContainer.m_height      = height;
		_imageContainer.m_depth       = depth;
		_imageContainer.m_format      = format;
		_imageContainer.m_orientation = Orientation::R0;
		_imageContainer.m_numLayers   = uint16_t(arraySize);
		_imageContainer.m_numMips     = uint8_t( (caps[0] & DDSCAPS_MIPMAP) ? mips : 1);
		_imageContainer.m_hasAlpha    = hasAlpha;
		_imageContainer.m_cubeMap     = cubeMap;
		_imageContainer.m_ktx         = false;
		_imageContainer.m_ktxLE       = false;
		_imageContainer.m_srgb        = srgb;

		return true;
	}

	ImageContainer* imageParseDds(bx::AllocatorI* _allocator, const void* _src, uint32_t _size, bx::Error* _err)
	{
		return imageParseT<DDS_MAGIC, imageParseDds>(_allocator, _src, _size, _err);
	}

// KTX
#define KTX_MAGIC       BX_MAKEFOURCC(0xAB, 'K', 'T', 'X')
#define KTX_HEADER_SIZE 64

#define KTX_ETC1_RGB8_OES                             0x8D64
#define KTX_COMPRESSED_R11_EAC                        0x9270
#define KTX_COMPRESSED_SIGNED_R11_EAC                 0x9271
#define KTX_COMPRESSED_RG11_EAC                       0x9272
#define KTX_COMPRESSED_SIGNED_RG11_EAC                0x9273
#define KTX_COMPRESSED_RGB8_ETC2                      0x9274
#define KTX_COMPRESSED_SRGB8_ETC2                     0x9275
#define KTX_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2  0x9276
#define KTX_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2 0x9277
#define KTX_COMPRESSED_RGBA8_ETC2_EAC                 0x9278
#define KTX_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC          0x9279
#define KTX_COMPRESSED_RGB_PVRTC_4BPPV1_IMG           0x8C00
#define KTX_COMPRESSED_RGB_PVRTC_2BPPV1_IMG           0x8C01
#define KTX_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG          0x8C02
#define KTX_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG          0x8C03
#define KTX_COMPRESSED_RGBA_PVRTC_2BPPV2_IMG          0x9137
#define KTX_COMPRESSED_RGBA_PVRTC_4BPPV2_IMG          0x9138
#define KTX_COMPRESSED_RGB_S3TC_DXT1_EXT              0x83F0
#define KTX_COMPRESSED_RGBA_S3TC_DXT1_EXT             0x83F1
#define KTX_COMPRESSED_RGBA_S3TC_DXT3_EXT             0x83F2
#define KTX_COMPRESSED_RGBA_S3TC_DXT5_EXT             0x83F3
#define KTX_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT       0x8C4D
#define KTX_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT       0x8C4E
#define KTX_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT       0x8C4F
#define KTX_COMPRESSED_LUMINANCE_LATC1_EXT            0x8C70
#define KTX_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT      0x8C72
#define KTX_COMPRESSED_RGBA_BPTC_UNORM_ARB            0x8E8C
#define KTX_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_ARB      0x8E8D
#define KTX_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_ARB      0x8E8E
#define KTX_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_ARB    0x8E8F
#define KTX_COMPRESSED_SRGB_PVRTC_2BPPV1_EXT          0x8A54
#define KTX_COMPRESSED_SRGB_PVRTC_4BPPV1_EXT          0x8A55
#define KTX_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV1_EXT    0x8A56
#define KTX_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV1_EXT    0x8A57

#define KTX_A8                                        0x803C
#define KTX_R8                                        0x8229
#define KTX_R16                                       0x822A
#define KTX_RG8                                       0x822B
#define KTX_RG16                                      0x822C
#define KTX_R16F                                      0x822D
#define KTX_R32F                                      0x822E
#define KTX_RG16F                                     0x822F
#define KTX_RG32F                                     0x8230
#define KTX_RGBA8                                     0x8058
#define KTX_RGBA16                                    0x805B
#define KTX_RGBA16F                                   0x881A
#define KTX_R32UI                                     0x8236
#define KTX_RG32UI                                    0x823C
#define KTX_RGBA32UI                                  0x8D70
#define KTX_RGBA32F                                   0x8814
#define KTX_RGB565                                    0x8D62
#define KTX_RGBA4                                     0x8056
#define KTX_RGB5_A1                                   0x8057
#define KTX_RGB10_A2                                  0x8059
#define KTX_R8I                                       0x8231
#define KTX_R8UI                                      0x8232
#define KTX_R16I                                      0x8233
#define KTX_R16UI                                     0x8234
#define KTX_R32I                                      0x8235
#define KTX_R32UI                                     0x8236
#define KTX_RG8I                                      0x8237
#define KTX_RG8UI                                     0x8238
#define KTX_RG16I                                     0x8239
#define KTX_RG16UI                                    0x823A
#define KTX_RG32I                                     0x823B
#define KTX_RG32UI                                    0x823C
#define KTX_R8_SNORM                                  0x8F94
#define KTX_RG8_SNORM                                 0x8F95
#define KTX_RGB8_SNORM                                0x8F96
#define KTX_RGBA8_SNORM                               0x8F97
#define KTX_R16_SNORM                                 0x8F98
#define KTX_RG16_SNORM                                0x8F99
#define KTX_RGB16_SNORM                               0x8F9A
#define KTX_RGBA16_SNORM                              0x8F9B
#define KTX_SRGB8                                     0x8C41
#define KTX_SRGB8_ALPHA8                              0x8C43
#define KTX_RGBA32UI                                  0x8D70
#define KTX_RGB32UI                                   0x8D71
#define KTX_RGBA16UI                                  0x8D76
#define KTX_RGB16UI                                   0x8D77
#define KTX_RGBA8UI                                   0x8D7C
#define KTX_RGB8UI                                    0x8D7D
#define KTX_RGBA32I                                   0x8D82
#define KTX_RGB32I                                    0x8D83
#define KTX_RGBA16I                                   0x8D88
#define KTX_RGB16I                                    0x8D89
#define KTX_RGBA8I                                    0x8D8E
#define KTX_RGB8                                      0x8051
#define KTX_RGB8I                                     0x8D8F
#define KTX_RGB9_E5                                   0x8C3D
#define KTX_R11F_G11F_B10F                            0x8C3A

#define KTX_ZERO                                      0
#define KTX_RED                                       0x1903
#define KTX_ALPHA                                     0x1906
#define KTX_RGB                                       0x1907
#define KTX_RGBA                                      0x1908
#define KTX_BGRA                                      0x80E1
#define KTX_RG                                        0x8227

#define KTX_BYTE                                      0x1400
#define KTX_UNSIGNED_BYTE                             0x1401
#define KTX_SHORT                                     0x1402
#define KTX_UNSIGNED_SHORT                            0x1403
#define KTX_INT                                       0x1404
#define KTX_UNSIGNED_INT                              0x1405
#define KTX_FLOAT                                     0x1406
#define KTX_HALF_FLOAT                                0x140B
#define KTX_UNSIGNED_INT_5_9_9_9_REV                  0x8C3E
#define KTX_UNSIGNED_SHORT_5_6_5                      0x8363
#define KTX_UNSIGNED_SHORT_4_4_4_4                    0x8033
#define KTX_UNSIGNED_SHORT_5_5_5_1                    0x8034
#define KTX_UNSIGNED_INT_2_10_10_10_REV               0x8368
#define KTX_UNSIGNED_INT_10F_11F_11F_REV              0x8C3B

	struct KtxFormatInfo
	{
		uint32_t m_internalFmt;
		uint32_t m_internalFmtSrgb;
		uint32_t m_fmt;
		uint32_t m_type;
	};

	static const KtxFormatInfo s_translateKtxFormat[] =
	{
		{ KTX_COMPRESSED_RGBA_S3TC_DXT1_EXT,            KTX_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT,        KTX_COMPRESSED_RGBA_S3TC_DXT1_EXT,            KTX_ZERO,                         }, // BC1
		{ KTX_COMPRESSED_RGBA_S3TC_DXT3_EXT,            KTX_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT,        KTX_COMPRESSED_RGBA_S3TC_DXT3_EXT,            KTX_ZERO,                         }, // BC2
		{ KTX_COMPRESSED_RGBA_S3TC_DXT5_EXT,            KTX_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT,        KTX_COMPRESSED_RGBA_S3TC_DXT5_EXT,            KTX_ZERO,                         }, // BC3
		{ KTX_COMPRESSED_LUMINANCE_LATC1_EXT,           KTX_ZERO,                                       KTX_COMPRESSED_LUMINANCE_LATC1_EXT,           KTX_ZERO,                         }, // BC4
		{ KTX_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT,     KTX_ZERO,                                       KTX_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT,     KTX_ZERO,                         }, // BC5
		{ KTX_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_ARB,     KTX_ZERO,                                       KTX_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_ARB,     KTX_ZERO,                         }, // BC6H
		{ KTX_COMPRESSED_RGBA_BPTC_UNORM_ARB,           KTX_ZERO,                                       KTX_COMPRESSED_RGBA_BPTC_UNORM_ARB,           KTX_ZERO,                         }, // BC7
		{ KTX_ETC1_RGB8_OES,                            KTX_ZERO,                                       KTX_ETC1_RGB8_OES,                            KTX_ZERO,                         }, // ETC1
		{ KTX_COMPRESSED_RGB8_ETC2,                     KTX_ZERO,                                       KTX_COMPRESSED_RGB8_ETC2,                     KTX_ZERO,                         }, // ETC2
		{ KTX_COMPRESSED_RGBA8_ETC2_EAC,                KTX_COMPRESSED_SRGB8_ETC2,                      KTX_COMPRESSED_RGBA8_ETC2_EAC,                KTX_ZERO,                         }, // ETC2A
		{ KTX_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2, KTX_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2,  KTX_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2, KTX_ZERO,                         }, // ETC2A1
		{ KTX_COMPRESSED_RGB_PVRTC_2BPPV1_IMG,          KTX_COMPRESSED_SRGB_PVRTC_2BPPV1_EXT,           KTX_COMPRESSED_RGB_PVRTC_2BPPV1_IMG,          KTX_ZERO,                         }, // PTC12
		{ KTX_COMPRESSED_RGB_PVRTC_4BPPV1_IMG,          KTX_COMPRESSED_SRGB_PVRTC_4BPPV1_EXT,           KTX_COMPRESSED_RGB_PVRTC_4BPPV1_IMG,          KTX_ZERO,                         }, // PTC14
		{ KTX_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG,         KTX_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV1_EXT,     KTX_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG,         KTX_ZERO,                         }, // PTC12A
		{ KTX_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG,         KTX_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV1_EXT,     KTX_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG,         KTX_ZERO,                         }, // PTC14A
		{ KTX_COMPRESSED_RGBA_PVRTC_2BPPV2_IMG,         KTX_ZERO,                                       KTX_COMPRESSED_RGBA_PVRTC_2BPPV2_IMG,         KTX_ZERO,                         }, // PTC22
		{ KTX_COMPRESSED_RGBA_PVRTC_4BPPV2_IMG,         KTX_ZERO,                                       KTX_COMPRESSED_RGBA_PVRTC_4BPPV2_IMG,         KTX_ZERO,                         }, // PTC24
		{ KTX_ZERO,                                     KTX_ZERO,                                       KTX_ZERO,                                     KTX_ZERO,                         }, // Unknown
		{ KTX_ZERO,                                     KTX_ZERO,                                       KTX_ZERO,                                     KTX_ZERO,                         }, // R1
		{ KTX_ALPHA,                                    KTX_ZERO,                                       KTX_ALPHA,                                    KTX_UNSIGNED_BYTE,                }, // A8
		{ KTX_R8,                                       KTX_ZERO,                                       KTX_RED,                                      KTX_UNSIGNED_BYTE,                }, // R8
		{ KTX_R8I,                                      KTX_ZERO,                                       KTX_RED,                                      KTX_BYTE,                         }, // R8S
		{ KTX_R8UI,                                     KTX_ZERO,                                       KTX_RED,                                      KTX_UNSIGNED_BYTE,                }, // R8S
		{ KTX_R8_SNORM,                                 KTX_ZERO,                                       KTX_RED,                                      KTX_BYTE,                         }, // R8S
		{ KTX_R16,                                      KTX_ZERO,                                       KTX_RED,                                      KTX_UNSIGNED_SHORT,               }, // R16
		{ KTX_R16I,                                     KTX_ZERO,                                       KTX_RED,                                      KTX_SHORT,                        }, // R16I
		{ KTX_R16UI,                                    KTX_ZERO,                                       KTX_RED,                                      KTX_UNSIGNED_SHORT,               }, // R16U
		{ KTX_R16F,                                     KTX_ZERO,                                       KTX_RED,                                      KTX_HALF_FLOAT,                   }, // R16F
		{ KTX_R16_SNORM,                                KTX_ZERO,                                       KTX_RED,                                      KTX_SHORT,                        }, // R16S
		{ KTX_R32I,                                     KTX_ZERO,                                       KTX_RED,                                      KTX_INT,                          }, // R32I
		{ KTX_R32UI,                                    KTX_ZERO,                                       KTX_RED,                                      KTX_UNSIGNED_INT,                 }, // R32U
		{ KTX_R32F,                                     KTX_ZERO,                                       KTX_RED,                                      KTX_FLOAT,                        }, // R32F
		{ KTX_RG8,                                      KTX_ZERO,                                       KTX_RG,                                       KTX_UNSIGNED_BYTE,                }, // RG8
		{ KTX_RG8I,                                     KTX_ZERO,                                       KTX_RG,                                       KTX_BYTE,                         }, // RG8I
		{ KTX_RG8UI,                                    KTX_ZERO,                                       KTX_RG,                                       KTX_UNSIGNED_BYTE,                }, // RG8U
		{ KTX_RG8_SNORM,                                KTX_ZERO,                                       KTX_RG,                                       KTX_BYTE,                         }, // RG8S
		{ KTX_RG16,                                     KTX_ZERO,                                       KTX_RG,                                       KTX_UNSIGNED_SHORT,               }, // RG16
		{ KTX_RG16I,                                    KTX_ZERO,                                       KTX_RG,                                       KTX_SHORT,                        }, // RG16
		{ KTX_RG16UI,                                   KTX_ZERO,                                       KTX_RG,                                       KTX_UNSIGNED_SHORT,               }, // RG16
		{ KTX_RG16F,                                    KTX_ZERO,                                       KTX_RG,                                       KTX_FLOAT,                        }, // RG16F
		{ KTX_RG16_SNORM,                               KTX_ZERO,                                       KTX_RG,                                       KTX_SHORT,                        }, // RG16S
		{ KTX_RG32I,                                    KTX_ZERO,                                       KTX_RG,                                       KTX_INT,                          }, // RG32I
		{ KTX_RG32UI,                                   KTX_ZERO,                                       KTX_RG,                                       KTX_UNSIGNED_INT,                 }, // RG32U
		{ KTX_RG32F,                                    KTX_ZERO,                                       KTX_RG,                                       KTX_FLOAT,                        }, // RG32F
		{ KTX_RGB8,                                     KTX_SRGB8,                                      KTX_RGB,                                      KTX_UNSIGNED_BYTE,                }, // RGB8
		{ KTX_RGB8I,                                    KTX_ZERO,                                       KTX_RGB,                                      KTX_BYTE,                         }, // RGB8I
		{ KTX_RGB8UI,                                   KTX_ZERO,                                       KTX_RGB,                                      KTX_UNSIGNED_BYTE,                }, // RGB8U
		{ KTX_RGB8_SNORM,                               KTX_ZERO,                                       KTX_RGB,                                      KTX_BYTE,                         }, // RGB8S
		{ KTX_RGB9_E5,                                  KTX_ZERO,                                       KTX_RGB,                                      KTX_UNSIGNED_INT_5_9_9_9_REV,     }, // RGB9E5F
		{ KTX_BGRA,                                     KTX_SRGB8_ALPHA8,                               KTX_BGRA,                                     KTX_UNSIGNED_BYTE,                }, // BGRA8
		{ KTX_RGBA8,                                    KTX_SRGB8_ALPHA8,                               KTX_RGBA,                                     KTX_UNSIGNED_BYTE,                }, // RGBA8
		{ KTX_RGBA8I,                                   KTX_ZERO,                                       KTX_RGBA,                                     KTX_BYTE,                         }, // RGBA8I
		{ KTX_RGBA8UI,                                  KTX_ZERO,                                       KTX_RGBA,                                     KTX_UNSIGNED_BYTE,                }, // RGBA8U
		{ KTX_RGBA8_SNORM,                              KTX_ZERO,                                       KTX_RGBA,                                     KTX_BYTE,                         }, // RGBA8S
		{ KTX_RGBA16,                                   KTX_ZERO,                                       KTX_RGBA,                                     KTX_UNSIGNED_SHORT,               }, // RGBA16
		{ KTX_RGBA16I,                                  KTX_ZERO,                                       KTX_RGBA,                                     KTX_SHORT,                        }, // RGBA16I
		{ KTX_RGBA16UI,                                 KTX_ZERO,                                       KTX_RGBA,                                     KTX_UNSIGNED_SHORT,               }, // RGBA16U
		{ KTX_RGBA16F,                                  KTX_ZERO,                                       KTX_RGBA,                                     KTX_HALF_FLOAT,                   }, // RGBA16F
		{ KTX_RGBA16_SNORM,                             KTX_ZERO,                                       KTX_RGBA,                                     KTX_SHORT,                        }, // RGBA16S
		{ KTX_RGBA32I,                                  KTX_ZERO,                                       KTX_RGBA,                                     KTX_INT,                          }, // RGBA32I
		{ KTX_RGBA32UI,                                 KTX_ZERO,                                       KTX_RGBA,                                     KTX_UNSIGNED_INT,                 }, // RGBA32U
		{ KTX_RGBA32F,                                  KTX_ZERO,                                       KTX_RGBA,                                     KTX_FLOAT,                        }, // RGBA32F
		{ KTX_RGB565,                                   KTX_ZERO,                                       KTX_RGB,                                      KTX_UNSIGNED_SHORT_5_6_5,         }, // R5G6B5
		{ KTX_RGBA4,                                    KTX_ZERO,                                       KTX_RGBA,                                     KTX_UNSIGNED_SHORT_4_4_4_4,       }, // RGBA4
		{ KTX_RGB5_A1,                                  KTX_ZERO,                                       KTX_RGBA,                                     KTX_UNSIGNED_SHORT_5_5_5_1,       }, // RGB5A1
		{ KTX_RGB10_A2,                                 KTX_ZERO,                                       KTX_RGBA,                                     KTX_UNSIGNED_INT_2_10_10_10_REV,  }, // RGB10A2
		{ KTX_R11F_G11F_B10F,                           KTX_ZERO,                                       KTX_RGB,                                      KTX_UNSIGNED_INT_10F_11F_11F_REV, }, // RG11B10F
	};
	BX_STATIC_ASSERT(TextureFormat::UnknownDepth == BX_COUNTOF(s_translateKtxFormat) );

	struct KtxFormatInfo2
	{
		uint32_t m_internalFmt;
		TextureFormat::Enum m_format;
	};

	static const KtxFormatInfo2 s_translateKtxFormat2[] =
	{
		{ KTX_A8,                           TextureFormat::A8    },
		{ KTX_RED,                          TextureFormat::R8    },
		{ KTX_RGB,                          TextureFormat::RGB8  },
		{ KTX_RGBA,                         TextureFormat::RGBA8 },
		{ KTX_COMPRESSED_RGB_S3TC_DXT1_EXT, TextureFormat::BC1   },
	};

	bool imageParseKtx(ImageContainer& _imageContainer, bx::ReaderSeekerI* _reader, bx::Error* _err)
	{
		BX_ERROR_SCOPE(_err);

		uint8_t identifier[8];
		bx::read(_reader, identifier);

		if (identifier[1] != '1'
		&&  identifier[2] != '1')
		{
			return false;
		}

		uint32_t endianness;
		bx::read(_reader, endianness);

		bool fromLittleEndian = 0x04030201 == endianness;

		uint32_t glType;
		bx::readHE(_reader, glType, fromLittleEndian);

		uint32_t glTypeSize;
		bx::readHE(_reader, glTypeSize, fromLittleEndian);

		uint32_t glFormat;
		bx::readHE(_reader, glFormat, fromLittleEndian);

		uint32_t glInternalFormat;
		bx::readHE(_reader, glInternalFormat, fromLittleEndian);

		uint32_t glBaseInternalFormat;
		bx::readHE(_reader, glBaseInternalFormat, fromLittleEndian);

		uint32_t width;
		bx::readHE(_reader, width, fromLittleEndian);

		uint32_t height;
		bx::readHE(_reader, height, fromLittleEndian);

		uint32_t depth;
		bx::readHE(_reader, depth, fromLittleEndian);

		uint32_t numberOfArrayElements;
		bx::readHE(_reader, numberOfArrayElements, fromLittleEndian);

		uint32_t numFaces;
		bx::readHE(_reader, numFaces, fromLittleEndian);

		uint32_t numMips;
		bx::readHE(_reader, numMips, fromLittleEndian);

		uint32_t metaDataSize;
		bx::readHE(_reader, metaDataSize, fromLittleEndian);

		// skip meta garbage...
		int64_t offset = bx::skip(_reader, metaDataSize);

		TextureFormat::Enum format = TextureFormat::Unknown;
		bool hasAlpha = false;

		for (uint32_t ii = 0; ii < BX_COUNTOF(s_translateKtxFormat); ++ii)
		{
			if (s_translateKtxFormat[ii].m_internalFmt == glInternalFormat)
			{
				format = TextureFormat::Enum(ii);
				break;
			}
		}

		if (TextureFormat::Unknown == format)
		{
			for (uint32_t ii = 0; ii < BX_COUNTOF(s_translateKtxFormat2); ++ii)
			{
				if (s_translateKtxFormat2[ii].m_internalFmt == glInternalFormat)
				{
					format = s_translateKtxFormat2[ii].m_format;
					break;
				}
			}
		}

		_imageContainer.m_allocator   = NULL;
		_imageContainer.m_data        = NULL;
		_imageContainer.m_size        = 0;
		_imageContainer.m_offset      = (uint32_t)offset;
		_imageContainer.m_width       = width;
		_imageContainer.m_height      = height;
		_imageContainer.m_depth       = depth;
		_imageContainer.m_format      = format;
		_imageContainer.m_orientation = Orientation::R0;
		_imageContainer.m_numLayers   = uint16_t(bx::uint32_max(numberOfArrayElements, 1) );
		_imageContainer.m_numMips     = uint8_t(bx::uint32_max(numMips, 1) );
		_imageContainer.m_hasAlpha    = hasAlpha;
		_imageContainer.m_cubeMap     = numFaces > 1;
		_imageContainer.m_ktx         = true;
		_imageContainer.m_ktxLE       = fromLittleEndian;
		_imageContainer.m_srgb        = false;

		if (TextureFormat::Unknown == format)
		{
			BX_ERROR_SET(_err, BIMG_ERROR, "Unrecognized image format.");
			return false;
		}

		return true;
	}

	ImageContainer* imageParseKtx(bx::AllocatorI* _allocator, const void* _src, uint32_t _size, bx::Error* _err)
	{
		return imageParseT<KTX_MAGIC, imageParseKtx>(_allocator, _src, _size, _err);
	}

// PVR3
#define PVR3_MAKE8CC(_a, _b, _c, _d, _e, _f, _g, _h) (uint64_t(BX_MAKEFOURCC(_a, _b, _c, _d) ) | (uint64_t(BX_MAKEFOURCC(_e, _f, _g, _h) )<<32) )

#define PVR3_MAGIC            BX_MAKEFOURCC('P', 'V', 'R', 3)
#define PVR3_HEADER_SIZE      52

#define PVR3_PVRTC1_2BPP_RGB  0
#define PVR3_PVRTC1_2BPP_RGBA 1
#define PVR3_PVRTC1_4BPP_RGB  2
#define PVR3_PVRTC1_4BPP_RGBA 3
#define PVR3_PVRTC2_2BPP_RGBA 4
#define PVR3_PVRTC2_4BPP_RGBA 5
#define PVR3_ETC1             6
#define PVR3_DXT1             7
#define PVR3_DXT2             8
#define PVR3_DXT3             9
#define PVR3_DXT4             10
#define PVR3_DXT5             11
#define PVR3_BC4              12
#define PVR3_BC5              13
#define PVR3_R8               PVR3_MAKE8CC('r',   0,   0,   0,  8,  0,  0,  0)
#define PVR3_R16              PVR3_MAKE8CC('r',   0,   0,   0, 16,  0,  0,  0)
#define PVR3_R32              PVR3_MAKE8CC('r',   0,   0,   0, 32,  0,  0,  0)
#define PVR3_RG8              PVR3_MAKE8CC('r', 'g',   0,   0,  8,  8,  0,  0)
#define PVR3_RG16             PVR3_MAKE8CC('r', 'g',   0,   0, 16, 16,  0,  0)
#define PVR3_RG32             PVR3_MAKE8CC('r', 'g',   0,   0, 32, 32,  0,  0)
#define PVR3_BGRA8            PVR3_MAKE8CC('b', 'g', 'r', 'a',  8,  8,  8,  8)
#define PVR3_RGBA16           PVR3_MAKE8CC('r', 'g', 'b', 'a', 16, 16, 16, 16)
#define PVR3_RGBA32           PVR3_MAKE8CC('r', 'g', 'b', 'a', 32, 32, 32, 32)
#define PVR3_RGB565           PVR3_MAKE8CC('r', 'g', 'b',   0,  5,  6,  5,  0)
#define PVR3_RGBA4            PVR3_MAKE8CC('r', 'g', 'b', 'a',  4,  4,  4,  4)
#define PVR3_RGBA51           PVR3_MAKE8CC('r', 'g', 'b', 'a',  5,  5,  5,  1)
#define PVR3_RGB10A2          PVR3_MAKE8CC('r', 'g', 'b', 'a', 10, 10, 10,  2)

#define PVR3_CHANNEL_TYPE_ANY   UINT32_MAX
#define PVR3_CHANNEL_TYPE_FLOAT UINT32_C(12)

	struct TranslatePvr3Format
	{
		uint64_t m_format;
		uint32_t m_channelTypeMask;
		TextureFormat::Enum m_textureFormat;
	};

	static const TranslatePvr3Format s_translatePvr3Format[] =
	{
		{ PVR3_PVRTC1_2BPP_RGB,  PVR3_CHANNEL_TYPE_ANY,   TextureFormat::PTC12   },
		{ PVR3_PVRTC1_2BPP_RGBA, PVR3_CHANNEL_TYPE_ANY,   TextureFormat::PTC12A  },
		{ PVR3_PVRTC1_4BPP_RGB,  PVR3_CHANNEL_TYPE_ANY,   TextureFormat::PTC14   },
		{ PVR3_PVRTC1_4BPP_RGBA, PVR3_CHANNEL_TYPE_ANY,   TextureFormat::PTC14A  },
		{ PVR3_PVRTC2_2BPP_RGBA, PVR3_CHANNEL_TYPE_ANY,   TextureFormat::PTC22   },
		{ PVR3_PVRTC2_4BPP_RGBA, PVR3_CHANNEL_TYPE_ANY,   TextureFormat::PTC24   },
		{ PVR3_ETC1,             PVR3_CHANNEL_TYPE_ANY,   TextureFormat::ETC1    },
		{ PVR3_DXT1,             PVR3_CHANNEL_TYPE_ANY,   TextureFormat::BC1     },
		{ PVR3_DXT2,             PVR3_CHANNEL_TYPE_ANY,   TextureFormat::BC2     },
		{ PVR3_DXT3,             PVR3_CHANNEL_TYPE_ANY,   TextureFormat::BC2     },
		{ PVR3_DXT4,             PVR3_CHANNEL_TYPE_ANY,   TextureFormat::BC3     },
		{ PVR3_DXT5,             PVR3_CHANNEL_TYPE_ANY,   TextureFormat::BC3     },
		{ PVR3_BC4,              PVR3_CHANNEL_TYPE_ANY,   TextureFormat::BC4     },
		{ PVR3_BC5,              PVR3_CHANNEL_TYPE_ANY,   TextureFormat::BC5     },
		{ PVR3_R8,               PVR3_CHANNEL_TYPE_ANY,   TextureFormat::R8      },
		{ PVR3_R16,              PVR3_CHANNEL_TYPE_ANY,   TextureFormat::R16U    },
		{ PVR3_R16,              PVR3_CHANNEL_TYPE_FLOAT, TextureFormat::R16F    },
		{ PVR3_R32,              PVR3_CHANNEL_TYPE_ANY,   TextureFormat::R32U    },
		{ PVR3_R32,              PVR3_CHANNEL_TYPE_FLOAT, TextureFormat::R32F    },
		{ PVR3_RG8,              PVR3_CHANNEL_TYPE_ANY,   TextureFormat::RG8     },
		{ PVR3_RG16,             PVR3_CHANNEL_TYPE_ANY,   TextureFormat::RG16    },
		{ PVR3_RG16,             PVR3_CHANNEL_TYPE_FLOAT, TextureFormat::RG16F   },
		{ PVR3_RG32,             PVR3_CHANNEL_TYPE_ANY,   TextureFormat::RG16    },
		{ PVR3_RG32,             PVR3_CHANNEL_TYPE_FLOAT, TextureFormat::RG32F   },
		{ PVR3_BGRA8,            PVR3_CHANNEL_TYPE_ANY,   TextureFormat::BGRA8   },
		{ PVR3_RGBA16,           PVR3_CHANNEL_TYPE_ANY,   TextureFormat::RGBA16  },
		{ PVR3_RGBA16,           PVR3_CHANNEL_TYPE_FLOAT, TextureFormat::RGBA16F },
		{ PVR3_RGBA32,           PVR3_CHANNEL_TYPE_ANY,   TextureFormat::RGBA32U },
		{ PVR3_RGBA32,           PVR3_CHANNEL_TYPE_FLOAT, TextureFormat::RGBA32F },
		{ PVR3_RGB565,           PVR3_CHANNEL_TYPE_ANY,   TextureFormat::R5G6B5  },
		{ PVR3_RGBA4,            PVR3_CHANNEL_TYPE_ANY,   TextureFormat::RGBA4   },
		{ PVR3_RGBA51,           PVR3_CHANNEL_TYPE_ANY,   TextureFormat::RGB5A1  },
		{ PVR3_RGB10A2,          PVR3_CHANNEL_TYPE_ANY,   TextureFormat::RGB10A2 },
	};

	bool imageParsePvr3(ImageContainer& _imageContainer, bx::ReaderSeekerI* _reader, bx::Error* _err)
	{
		BX_ERROR_SCOPE(_err);

		uint32_t flags;
		bx::read(_reader, flags);

		uint64_t pixelFormat;
		bx::read(_reader, pixelFormat);

		uint32_t colorSpace;
		bx::read(_reader, colorSpace); // 0 - linearRGB, 1 - sRGB

		uint32_t channelType;
		bx::read(_reader, channelType);

		uint32_t height;
		bx::read(_reader, height);

		uint32_t width;
		bx::read(_reader, width);

		uint32_t depth;
		bx::read(_reader, depth);

		uint32_t numSurfaces;
		bx::read(_reader, numSurfaces);

		uint32_t numFaces;
		bx::read(_reader, numFaces);

		uint32_t numMips;
		bx::read(_reader, numMips);

		uint32_t metaDataSize;
		bx::read(_reader, metaDataSize);

		// skip meta garbage...
		int64_t offset = bx::skip(_reader, metaDataSize);

		TextureFormat::Enum format = TextureFormat::Unknown;
		bool hasAlpha = false;

		for (uint32_t ii = 0; ii < BX_COUNTOF(s_translatePvr3Format); ++ii)
		{
			if (s_translatePvr3Format[ii].m_format == pixelFormat
			&&  channelType == (s_translatePvr3Format[ii].m_channelTypeMask & channelType) )
			{
				format = s_translatePvr3Format[ii].m_textureFormat;
				break;
			}
		}

		_imageContainer.m_allocator   = NULL;
		_imageContainer.m_data        = NULL;
		_imageContainer.m_size        = 0;
		_imageContainer.m_offset      = (uint32_t)offset;
		_imageContainer.m_width       = width;
		_imageContainer.m_height      = height;
		_imageContainer.m_depth       = depth;
		_imageContainer.m_format      = format;
		_imageContainer.m_orientation = Orientation::R0;
		_imageContainer.m_numLayers   = 1;
		_imageContainer.m_numMips     = uint8_t(bx::uint32_max(numMips, 1) );
		_imageContainer.m_hasAlpha    = hasAlpha;
		_imageContainer.m_cubeMap     = numFaces > 1;
		_imageContainer.m_ktx         = false;
		_imageContainer.m_ktxLE       = false;
		_imageContainer.m_srgb        = colorSpace > 0;

		return TextureFormat::Unknown != format;
	}

	ImageContainer* imageParsePvr3(bx::AllocatorI* _allocator, const void* _src, uint32_t _size, bx::Error* _err)
	{
		return imageParseT<PVR3_MAGIC, imageParsePvr3>(_allocator, _src, _size, _err);
	}

	bool imageParse(ImageContainer& _imageContainer, bx::ReaderSeekerI* _reader, bx::Error* _err)
	{
		BX_ERROR_SCOPE(_err);

		uint32_t magic;
		bx::read(_reader, magic, _err);

		if (DDS_MAGIC == magic)
		{
			return imageParseDds(_imageContainer, _reader, _err);
		}
		else if (KTX_MAGIC == magic)
		{
			return imageParseKtx(_imageContainer, _reader, _err);
		}
		else if (PVR3_MAGIC == magic)
		{
			return imageParsePvr3(_imageContainer, _reader, _err);
		}
		else if (BIMG_CHUNK_MAGIC_TEX == magic)
		{
			TextureCreate tc;
			bx::read(_reader, tc);

			_imageContainer.m_format      = tc.m_format;
			_imageContainer.m_orientation = Orientation::R0;
			_imageContainer.m_offset      = UINT32_MAX;
			_imageContainer.m_allocator   = NULL;
			if (NULL == tc.m_mem)
			{
				_imageContainer.m_data = NULL;
				_imageContainer.m_size = 0;
			}
			else
			{
				_imageContainer.m_data = tc.m_mem->data;
				_imageContainer.m_size = tc.m_mem->size;
			}
			_imageContainer.m_width     = tc.m_width;
			_imageContainer.m_height    = tc.m_height;
			_imageContainer.m_depth     = tc.m_depth;
			_imageContainer.m_numLayers = tc.m_numLayers;
			_imageContainer.m_numMips   = tc.m_numMips;
			_imageContainer.m_hasAlpha  = false;
			_imageContainer.m_cubeMap   = tc.m_cubeMap;
			_imageContainer.m_ktx       = false;
			_imageContainer.m_ktxLE     = false;
			_imageContainer.m_srgb      = false;

			return _err->isOk();
		}

		BX_TRACE("Unrecognized image format (magic: 0x%08x)!", magic);
		BX_ERROR_SET(_err, BIMG_ERROR, "Unrecognized image format.");

		return false;
	}

	bool imageParse(ImageContainer& _imageContainer, const void* _data, uint32_t _size, bx::Error* _err)
	{
		BX_ERROR_SCOPE(_err);

		bx::MemoryReader reader(_data, _size);
		return imageParse(_imageContainer, &reader, _err);
	}

	void imageDecodeToR8(bx::AllocatorI* _allocator, void* _dst, const void* _src, uint32_t _width, uint32_t _height, uint32_t _depth, uint32_t _dstPitch, TextureFormat::Enum _srcFormat)
	{
		const uint8_t* src = (const uint8_t*)_src;
		uint8_t* dst = (uint8_t*)_dst;

		const uint32_t srcBpp = s_imageBlockInfo[_srcFormat].bitsPerPixel;
		const uint32_t srcPitch = _width*srcBpp/8;

		for (uint32_t zz = 0; zz < _depth; ++zz, src += _height*srcPitch, dst += _height*_dstPitch)
		{
			if (isCompressed(_srcFormat))
			{
				uint32_t size = imageGetSize(NULL, uint16_t(_width), uint16_t(_height), 0, false, false, 1, TextureFormat::RGBA8);
				void* temp = BX_ALLOC(_allocator, size);
				imageDecodeToRgba8(temp, _src, _width, _height, _width*4, _srcFormat);
				imageConvert(dst, TextureFormat::R8, temp, TextureFormat::RGBA8, _width, _height, 1, _width*4);
				BX_FREE(_allocator, temp);
			}
			else
			{
				imageConvert(dst, TextureFormat::R8, src, _srcFormat, _width, _height, 1, srcPitch);
			}
		}
	}

	void imageDecodeToBgra8(void* _dst, const void* _src, uint32_t _width, uint32_t _height, uint32_t _dstPitch, TextureFormat::Enum _srcFormat)
	{
		const uint8_t* src = (const uint8_t*)_src;
		uint8_t* dst = (uint8_t*)_dst;

		uint32_t width  = _width/4;
		uint32_t height = _height/4;

		uint8_t temp[16*4];

		switch (_srcFormat)
		{
		case TextureFormat::BC1:
			for (uint32_t yy = 0; yy < height; ++yy)
			{
				for (uint32_t xx = 0; xx < width; ++xx)
				{
					decodeBlockDxt1(temp, src);
					src += 8;

					uint8_t* block = &dst[yy*_dstPitch*4 + xx*16];
					bx::memCopy(&block[0*_dstPitch], &temp[ 0], 16);
					bx::memCopy(&block[1*_dstPitch], &temp[16], 16);
					bx::memCopy(&block[2*_dstPitch], &temp[32], 16);
					bx::memCopy(&block[3*_dstPitch], &temp[48], 16);
				}
			}
			break;

		case TextureFormat::BC2:
			for (uint32_t yy = 0; yy < height; ++yy)
			{
				for (uint32_t xx = 0; xx < width; ++xx)
				{
					decodeBlockDxt23A(temp+3, src);
					src += 8;
					decodeBlockDxt(temp, src);
					src += 8;

					uint8_t* block = &dst[yy*_dstPitch*4 + xx*16];
					bx::memCopy(&block[0*_dstPitch], &temp[ 0], 16);
					bx::memCopy(&block[1*_dstPitch], &temp[16], 16);
					bx::memCopy(&block[2*_dstPitch], &temp[32], 16);
					bx::memCopy(&block[3*_dstPitch], &temp[48], 16);
				}
			}
			break;

		case TextureFormat::BC3:
			for (uint32_t yy = 0; yy < height; ++yy)
			{
				for (uint32_t xx = 0; xx < width; ++xx)
				{
					decodeBlockDxt45A(temp+3, src);
					src += 8;
					decodeBlockDxt(temp, src);
					src += 8;

					uint8_t* block = &dst[yy*_dstPitch*4 + xx*16];
					bx::memCopy(&block[0*_dstPitch], &temp[ 0], 16);
					bx::memCopy(&block[1*_dstPitch], &temp[16], 16);
					bx::memCopy(&block[2*_dstPitch], &temp[32], 16);
					bx::memCopy(&block[3*_dstPitch], &temp[48], 16);
				}
			}
			break;

		case TextureFormat::BC4:
			for (uint32_t yy = 0; yy < height; ++yy)
			{
				for (uint32_t xx = 0; xx < width; ++xx)
				{
					decodeBlockDxt45A(temp, src);
					src += 8;

					uint8_t* block = &dst[yy*_dstPitch*4 + xx*16];
					bx::memCopy(&block[0*_dstPitch], &temp[ 0], 16);
					bx::memCopy(&block[1*_dstPitch], &temp[16], 16);
					bx::memCopy(&block[2*_dstPitch], &temp[32], 16);
					bx::memCopy(&block[3*_dstPitch], &temp[48], 16);
				}
			}
			break;

		case TextureFormat::BC5:
			for (uint32_t yy = 0; yy < height; ++yy)
			{
				for (uint32_t xx = 0; xx < width; ++xx)
				{
					decodeBlockDxt45A(temp+2, src);
					src += 8;
					decodeBlockDxt45A(temp+1, src);
					src += 8;

					for (uint32_t ii = 0; ii < 16; ++ii)
					{
						float nx = temp[ii*4+2]*2.0f/255.0f - 1.0f;
						float ny = temp[ii*4+1]*2.0f/255.0f - 1.0f;
						float nz = bx::fsqrt(1.0f - nx*nx - ny*ny);
						temp[ii*4+0] = uint8_t( (nz + 1.0f)*255.0f/2.0f);
						temp[ii*4+3] = 0;
					}

					uint8_t* block = &dst[yy*_dstPitch*4 + xx*16];
					bx::memCopy(&block[0*_dstPitch], &temp[ 0], 16);
					bx::memCopy(&block[1*_dstPitch], &temp[16], 16);
					bx::memCopy(&block[2*_dstPitch], &temp[32], 16);
					bx::memCopy(&block[3*_dstPitch], &temp[48], 16);
				}
			}
			break;

		case TextureFormat::ETC1:
		case TextureFormat::ETC2:
			for (uint32_t yy = 0; yy < height; ++yy)
			{
				for (uint32_t xx = 0; xx < width; ++xx)
				{
					decodeBlockEtc12(temp, src);
					src += 8;

					uint8_t* block = &dst[yy*_dstPitch*4 + xx*16];
					bx::memCopy(&block[0*_dstPitch], &temp[ 0], 16);
					bx::memCopy(&block[1*_dstPitch], &temp[16], 16);
					bx::memCopy(&block[2*_dstPitch], &temp[32], 16);
					bx::memCopy(&block[3*_dstPitch], &temp[48], 16);
				}
			}
			break;

		case TextureFormat::ETC2A:
			BX_WARN(false, "ETC2A decoder is not implemented.");
			imageCheckerboard(_dst, _width, _height, 16, UINT32_C(0xff000000), UINT32_C(0xff00ff00) );
			break;

		case TextureFormat::ETC2A1:
			BX_WARN(false, "ETC2A1 decoder is not implemented.");
			imageCheckerboard(_dst, _width, _height, 16, UINT32_C(0xff000000), UINT32_C(0xffff0000) );
			break;

		case TextureFormat::PTC12:
			BX_WARN(false, "PTC12 decoder is not implemented.");
			imageCheckerboard(_dst, _width, _height, 16, UINT32_C(0xff000000), UINT32_C(0xffff00ff) );
			break;

		case TextureFormat::PTC12A:
			BX_WARN(false, "PTC12A decoder is not implemented.");
			imageCheckerboard(_dst, _width, _height, 16, UINT32_C(0xff000000), UINT32_C(0xffffff00) );
			break;

		case TextureFormat::PTC14:
			for (uint32_t yy = 0; yy < height; ++yy)
			{
				for (uint32_t xx = 0; xx < width; ++xx)
				{
					decodeBlockPtc14(temp, src, xx, yy, width, height);

					uint8_t* block = &dst[yy*_dstPitch*4 + xx*16];
					bx::memCopy(&block[0*_dstPitch], &temp[ 0], 16);
					bx::memCopy(&block[1*_dstPitch], &temp[16], 16);
					bx::memCopy(&block[2*_dstPitch], &temp[32], 16);
					bx::memCopy(&block[3*_dstPitch], &temp[48], 16);
				}
			}
			break;

		case TextureFormat::PTC14A:
			for (uint32_t yy = 0; yy < height; ++yy)
			{
				for (uint32_t xx = 0; xx < width; ++xx)
				{
					decodeBlockPtc14A(temp, src, xx, yy, width, height);

					uint8_t* block = &dst[yy*_dstPitch*4 + xx*16];
					bx::memCopy(&block[0*_dstPitch], &temp[ 0], 16);
					bx::memCopy(&block[1*_dstPitch], &temp[16], 16);
					bx::memCopy(&block[2*_dstPitch], &temp[32], 16);
					bx::memCopy(&block[3*_dstPitch], &temp[48], 16);
				}
			}
			break;

		case TextureFormat::PTC22:
			BX_WARN(false, "PTC22 decoder is not implemented.");
			imageCheckerboard(_dst, _width, _height, 16, UINT32_C(0xff00ff00), UINT32_C(0xff0000ff) );
			break;

		case TextureFormat::PTC24:
			BX_WARN(false, "PTC24 decoder is not implemented.");
			imageCheckerboard(_dst, _width, _height, 16, UINT32_C(0xff000000), UINT32_C(0xffffffff) );
			break;

		case TextureFormat::RGBA8:
			{
				const uint32_t srcPitch = _width * 4;
				imageSwizzleBgra8(_dst, _dstPitch, _width, _height, _src, srcPitch);
			}
			break;

		case TextureFormat::BGRA8:
			{
				const uint32_t srcPitch = _width * 4;
				const uint32_t size = bx::uint32_min(srcPitch, _dstPitch);
				bx::memCopy(_dst, _src, size, _height, srcPitch, _dstPitch);
			}
			break;

		default:
			{
				const uint32_t srcBpp   = s_imageBlockInfo[_srcFormat].bitsPerPixel;
				const uint32_t srcPitch = _width * srcBpp / 8;
				if (!imageConvert(_dst, TextureFormat::BGRA8, _src, _srcFormat, _width, _height, 1, srcPitch) )
				{
					// Failed to convert, just make ugly red-yellow checkerboard texture.
					imageCheckerboard(_dst, _width, _height, 16, UINT32_C(0xffff0000), UINT32_C(0xffffff00) );
				}
			}
			break;
		}
	}

	void imageDecodeToRgba8(void* _dst, const void* _src, uint32_t _width, uint32_t _height, uint32_t _dstPitch, TextureFormat::Enum _srcFormat)
	{
		switch (_srcFormat)
		{
		case TextureFormat::RGBA8:
			{
				const uint32_t srcPitch = _width * 4;
				const uint32_t size = bx::uint32_min(srcPitch, _dstPitch);
				bx::memCopy(_dst, _src, size, _height, srcPitch, _dstPitch);
			}
			break;

		case TextureFormat::BGRA8:
			{
				const uint32_t srcPitch = _width * 4;
				imageSwizzleBgra8(_dst, _dstPitch, _width, _height, _src, srcPitch);
			}
			break;

		default:
			{
				const uint32_t srcPitch = _width * 4;
				imageDecodeToBgra8(_dst, _src, _width, _height, _dstPitch, _srcFormat);
				imageSwizzleBgra8(_dst, _dstPitch, _width, _height, _dst, srcPitch);
			}
			break;
		}
	}

	void imageRgba8ToRgba32fRef(void* _dst, uint32_t _width, uint32_t _height, uint32_t _srcPitch, const void* _src)
	{
		const uint32_t dstWidth  = _width;
		const uint32_t dstHeight = _height;

		if (0 == dstWidth
		||  0 == dstHeight)
		{
			return;
		}

		float* dst = (float*)_dst;
		const uint8_t* src = (const uint8_t*)_src;

		for (uint32_t yy = 0, ystep = _srcPitch; yy < dstHeight; ++yy, src += ystep)
		{
			const uint8_t* rgba = src;
			for (uint32_t xx = 0; xx < dstWidth; ++xx, rgba += 4, dst += 4)
			{
				dst[0] = bx::fpow(rgba[0], 2.2f);
				dst[1] = bx::fpow(rgba[1], 2.2f);
				dst[2] = bx::fpow(rgba[2], 2.2f);
				dst[3] =          rgba[3];
			}
		}
	}

	void imageRgba8ToRgba32f(void* _dst, uint32_t _width, uint32_t _height, uint32_t _srcPitch, const void* _src)
	{
		const uint32_t dstWidth  = _width;
		const uint32_t dstHeight = _height;

		if (0 == dstWidth
		||  0 == dstHeight)
		{
			return;
		}

		float* dst = (float*)_dst;
		const uint8_t* src = (const uint8_t*)_src;

		using namespace bx;
		const simd128_t unpack = simd_ld(1.0f, 1.0f/256.0f, 1.0f/65536.0f, 1.0f/16777216.0f);
		const simd128_t umask  = simd_ild(0xff, 0xff00, 0xff0000, 0xff000000);
		const simd128_t wflip  = simd_ild(0, 0, 0, 0x80000000);
		const simd128_t wadd   = simd_ld(0.0f, 0.0f, 0.0f, 32768.0f*65536.0f);

		for (uint32_t yy = 0, ystep = _srcPitch; yy < dstHeight; ++yy, src += ystep)
		{
			const uint8_t* rgba = src;
			for (uint32_t xx = 0; xx < dstWidth; ++xx, rgba += 4, dst += 4)
			{
				const simd128_t abgr0  = simd_splat(rgba);
				const simd128_t abgr0m = simd_and(abgr0, umask);
				const simd128_t abgr0x = simd_xor(abgr0m, wflip);
				const simd128_t abgr0f = simd_itof(abgr0x);
				const simd128_t abgr0c = simd_add(abgr0f, wadd);
				const simd128_t abgr0n = simd_mul(abgr0c, unpack);

				simd_st(dst, abgr0n);
			}
		}
	}

	void imageDecodeToRgba32f(bx::AllocatorI* _allocator, void* _dst, const void* _src, uint32_t _width, uint32_t _height, uint32_t _depth, uint32_t _dstPitch, TextureFormat::Enum _srcFormat)
	{
		const uint8_t* src = (const uint8_t*)_src;
		uint8_t* dst = (uint8_t*)_dst;

		const uint32_t srcBpp   = s_imageBlockInfo[_srcFormat].bitsPerPixel;
		const uint32_t srcPitch = _width*srcBpp/8;

		for (uint32_t zz = 0; zz < _depth; ++zz, src += _height*srcPitch, dst += _height*_dstPitch)
		{
			switch (_srcFormat)
			{
			case TextureFormat::BC5:
				{
					uint32_t width  = _width/4;
					uint32_t height = _height/4;

					const uint8_t* srcData = src;

					for (uint32_t yy = 0; yy < height; ++yy)
					{
						for (uint32_t xx = 0; xx < width; ++xx)
						{
							uint8_t temp[16*4];

							decodeBlockDxt45A(temp+2, srcData);
							srcData += 8;
							decodeBlockDxt45A(temp+1, srcData);
							srcData += 8;

							for (uint32_t ii = 0; ii < 16; ++ii)
							{
								float nx = temp[ii*4+2]*2.0f/255.0f - 1.0f;
								float ny = temp[ii*4+1]*2.0f/255.0f - 1.0f;
								float nz = bx::fsqrt(1.0f - nx*nx - ny*ny);

								const uint32_t offset = (yy*4 + ii/4)*_width*16 + (xx*4 + ii%4)*16;
								float* block = (float*)&dst[offset];
								block[0] = nx;
								block[1] = ny;
								block[2] = nz;
								block[3] = 0.0f;
							}
						}
					}
				}
				break;

			case TextureFormat::RGBA32F:
				bx::memCopy(dst, src, _dstPitch*_height);
				break;

			default:
				if (isCompressed(_srcFormat) )
				{
					uint32_t size = imageGetSize(NULL, uint16_t(_width), uint16_t(_height), 0, false, false, 1, TextureFormat::RGBA8);
					void* temp = BX_ALLOC(_allocator, size);
					imageDecodeToRgba8(temp, src, _width, _height, _width*4, _srcFormat);
					imageRgba8ToRgba32f(dst, _width, _height, _width*4, temp);
					BX_FREE(_allocator, temp);
				}
				else
				{
					imageConvert(dst, TextureFormat::RGBA32F, src, _srcFormat, _width, _height, 1, srcPitch);
				}
				break;
			}
		}
	}

	bool imageGetRawData(const ImageContainer& _imageContainer, uint16_t _side, uint8_t _lod, const void* _data, uint32_t _size, ImageMip& _mip)
	{
		uint32_t offset = _imageContainer.m_offset;
		TextureFormat::Enum format = TextureFormat::Enum(_imageContainer.m_format);
		bool hasAlpha = _imageContainer.m_hasAlpha;

		const ImageBlockInfo& blockInfo = s_imageBlockInfo[format];
		const uint8_t  bpp         = blockInfo.bitsPerPixel;
		const uint32_t blockSize   = blockInfo.blockSize;
		const uint32_t blockWidth  = blockInfo.blockWidth;
		const uint32_t blockHeight = blockInfo.blockHeight;
		const uint32_t minBlockX   = blockInfo.minBlockX;
		const uint32_t minBlockY   = blockInfo.minBlockY;

		if (UINT32_MAX == _imageContainer.m_offset)
		{
			if (NULL == _imageContainer.m_data)
			{
				return false;
			}

			offset = 0;
			_data = _imageContainer.m_data;
			_size = _imageContainer.m_size;
		}

		const uint8_t* data = (const uint8_t*)_data;
		const uint16_t numSides = _imageContainer.m_numLayers * (_imageContainer.m_cubeMap ? 6 : 1);

		if (_imageContainer.m_ktx)
		{
			uint32_t width  = _imageContainer.m_width;
			uint32_t height = _imageContainer.m_height;
			uint32_t depth  = _imageContainer.m_depth;

			for (uint8_t lod = 0, num = _imageContainer.m_numMips; lod < num; ++lod)
			{
				width  = bx::uint32_max(blockWidth  * minBlockX, ( (width  + blockWidth  - 1) / blockWidth )*blockWidth);
				height = bx::uint32_max(blockHeight * minBlockY, ( (height + blockHeight - 1) / blockHeight)*blockHeight);
				depth  = bx::uint32_max(1, depth);

				const uint32_t mipSize = width*height*depth*bpp/8;

				const uint32_t size = mipSize*numSides;
				uint32_t imageSize = bx::toHostEndian(*(const uint32_t*)&data[offset], _imageContainer.m_ktxLE);
				BX_CHECK(size == imageSize, "KTX: Image size mismatch %d (expected %d).", size, imageSize);
				BX_UNUSED(size, imageSize);

				offset += sizeof(uint32_t);

				for (uint16_t side = 0; side < numSides; ++side)
				{
					if (side == _side
					&&  lod  == _lod)
					{
						_mip.m_width     = width;
						_mip.m_height    = height;
						_mip.m_depth     = depth;
						_mip.m_blockSize = blockSize;
						_mip.m_size      = mipSize;
						_mip.m_data      = &data[offset];
						_mip.m_bpp       = bpp;
						_mip.m_format    = format;
						_mip.m_hasAlpha  = hasAlpha;
						return true;
					}

					offset += mipSize;

					BX_CHECK(offset <= _size, "Reading past size of data buffer! (offset %d, size %d)", offset, _size);
					BX_UNUSED(_size);
				}

				width  >>= 1;
				height >>= 1;
				depth  >>= 1;
			}
		}
		else
		{
			for (uint16_t side = 0; side < numSides; ++side)
			{
				uint32_t width  = _imageContainer.m_width;
				uint32_t height = _imageContainer.m_height;
				uint32_t depth  = _imageContainer.m_depth;

				for (uint8_t lod = 0, num = _imageContainer.m_numMips; lod < num; ++lod)
				{
					width  = bx::uint32_max(blockWidth  * minBlockX, ( (width  + blockWidth  - 1) / blockWidth )*blockWidth);
					height = bx::uint32_max(blockHeight * minBlockY, ( (height + blockHeight - 1) / blockHeight)*blockHeight);
					depth  = bx::uint32_max(1, depth);

					uint32_t size = width*height*depth*bpp/8;

					if (side == _side
					&&  lod  == _lod)
					{
						_mip.m_width     = width;
						_mip.m_height    = height;
						_mip.m_depth     = depth;
						_mip.m_blockSize = blockSize;
						_mip.m_size      = size;
						_mip.m_data      = &data[offset];
						_mip.m_bpp       = bpp;
						_mip.m_format    = format;
						_mip.m_hasAlpha  = hasAlpha;
						return true;
					}

					offset += size;

					BX_CHECK(offset <= _size, "Reading past size of data buffer! (offset %d, size %d)", offset, _size);
					BX_UNUSED(_size);

					width  >>= 1;
					height >>= 1;
					depth  >>= 1;
				}
			}
		}

		return false;
	}

	int32_t imageWriteTga(bx::WriterI* _writer, uint32_t _width, uint32_t _height, uint32_t _srcPitch, const void* _src, bool _grayscale, bool _yflip, bx::Error* _err)
	{
		BX_ERROR_SCOPE(_err);

		uint8_t type = _grayscale ? 3 :  2;
		uint8_t bpp  = _grayscale ? 8 : 32;

		uint8_t header[18] = {};
		header[ 2] = type;
		header[12] =  _width     &0xff;
		header[13] = (_width >>8)&0xff;
		header[14] =  _height    &0xff;
		header[15] = (_height>>8)&0xff;
		header[16] = bpp;
		header[17] = 32;

		int32_t total = 0;
		total += bx::write(_writer, header, sizeof(header), _err);

		uint32_t dstPitch = _width*bpp/8;
		if (_yflip)
		{
			const uint8_t* data = (const uint8_t*)_src + _srcPitch*_height - _srcPitch;
			for (uint32_t yy = 0; yy < _height && _err->isOk(); ++yy)
			{
				total += bx::write(_writer, data, dstPitch, _err);
				data -= _srcPitch;
			}
		}
		else if (_srcPitch == dstPitch)
		{
			total += bx::write(_writer, _src, _height*_srcPitch, _err);
		}
		else
		{
			const uint8_t* data = (const uint8_t*)_src;
			for (uint32_t yy = 0; yy < _height && _err->isOk(); ++yy)
			{
				total += bx::write(_writer, data, dstPitch, _err);
				data += _srcPitch;
			}
		}

		return total;
	}

	template<typename Ty>
	class HashWriter : public bx::WriterI
	{
	public:
		HashWriter(bx::WriterI* _writer)
			: m_writer(_writer)
		{
			begin();
		}

		void begin()
		{
			m_hash.begin();
		}

		uint32_t end()
		{
			return m_hash.end();
		}

		virtual int32_t write(const void* _data, int32_t _size, bx::Error* _err) override
		{
			m_hash.add(_data, _size);
			return m_writer->write(_data, _size, _err);
		}

	private:
		Ty m_hash;
		bx::WriterI* m_writer;
	};

	int32_t imageWritePng(bx::WriterI* _writer, uint32_t _width, uint32_t _height, uint32_t _srcPitch, const void* _src, bool _grayscale, bool _yflip, bx::Error* _err)
	{
		BX_ERROR_SCOPE(_err);

		int32_t total = 0;
		total += bx::write(_writer, "\x89PNG\r\n\x1a\n", _err);
		total += bx::write(_writer, bx::toBigEndian<uint32_t>(13), _err);

		HashWriter<bx::HashCrc32> writerC(_writer);
		total += bx::write(&writerC, "IHDR", _err);
		total += bx::write(&writerC, bx::toBigEndian(_width),  _err);
		total += bx::write(&writerC, bx::toBigEndian(_height), _err);
		total += bx::write(&writerC, "\x08\x06", _err);
		total += bx::writeRep(&writerC, 0, 3, _err);
		total += bx::write(_writer, bx::toBigEndian(writerC.end() ), _err);

		const uint32_t bpp    = _grayscale ? 8 : 32;
		const uint32_t stride = _width*bpp/8;
		const uint16_t zlen   = bx::toLittleEndian<uint16_t>(uint16_t(stride + 1) );
		const uint16_t zlenC  = bx::toLittleEndian<uint16_t>(~zlen);

		total += bx::write(_writer, bx::toBigEndian<uint32_t>(_height*(stride+6)+6), _err);

		writerC.begin();
		total += bx::write(&writerC, "IDAT", _err);
		total += bx::write(&writerC, "\x78\x9c", _err);

		const uint8_t* data = (const uint8_t*)_src;
		int32_t step = int32_t(_srcPitch);
		if (_yflip)
		{
			data += _srcPitch*_height - _srcPitch;
			step = -step;
		}

		HashWriter<bx::HashAdler32> writerA(&writerC);

		for (uint32_t ii = 0; ii < _height && _err->isOk(); ++ii)
		{
			total += bx::write(&writerC, uint8_t(ii == _height-1 ? 1 : 0), _err);
			total += bx::write(&writerC, zlen,  _err);
			total += bx::write(&writerC, zlenC, _err);

			total += bx::write(&writerA, uint8_t(0), _err);

			if (_grayscale)
			{
				total += bx::write(&writerA, data, stride, _err);
			}
			else
			{
				for (uint32_t xx = 0; xx < _width; ++xx)
				{
					const uint8_t* bgra = &data[xx*4];
					const uint8_t bb = bgra[0];
					const uint8_t gg = bgra[1];
					const uint8_t rr = bgra[2];
					const uint8_t aa = bgra[3];
					total += bx::write(&writerA, rr, _err);
					total += bx::write(&writerA, gg, _err);
					total += bx::write(&writerA, bb, _err);
					total += bx::write(&writerA, aa, _err);
				}
			}

			data += step;
		}
		total += bx::write(&writerC, bx::toBigEndian(writerA.end() ), _err);
		total += bx::write(_writer,  bx::toBigEndian(writerC.end() ), _err);

		total += bx::write(&writerC, uint32_t(0), _err);
		writerC.begin();
		total += bx::write(&writerC, "IEND", _err);
		total += bx::write(_writer,  bx::toBigEndian(writerC.end() ), _err);

		return total;
	}

	static int32_t imageWriteDdsHeader(bx::WriterI* _writer, TextureFormat::Enum _format, bool _cubeMap, uint32_t _width, uint32_t _height, uint32_t _depth, uint8_t _numMips, bx::Error* _err)
	{
		BX_ERROR_SCOPE(_err);

		uint32_t ddspf      = UINT32_MAX;
		uint32_t dxgiFormat = UINT32_MAX;

		for (uint32_t ii = 0; ii < BX_COUNTOF(s_translateDdsPixelFormat); ++ii)
		{
			if (s_translateDdsPixelFormat[ii].m_textureFormat == _format)
			{
				ddspf = ii;
				break;
			}
		}

		if (UINT32_MAX == ddspf)
		{
			for (uint32_t ii = 0; ii < BX_COUNTOF(s_translateDxgiFormat); ++ii)
			{
				if (s_translateDxgiFormat[ii].m_textureFormat == _format)
				{
					dxgiFormat = s_translateDxgiFormat[ii].m_format;
					break;
				}
			}

			if (UINT32_MAX == dxgiFormat)
			{
				BX_ERROR_SET(_err, BIMG_ERROR, "DDS: DXGI format not supported.");
				return 0;
			}
		}

		const uint32_t bpp = getBitsPerPixel(_format);

		uint32_t total = 0;
		total += bx::write(_writer, uint32_t(DDS_MAGIC), _err);

		uint32_t headerStart = total;
		total += bx::write(_writer, uint32_t(DDS_HEADER_SIZE), _err);
		total += bx::write(_writer, uint32_t(0
			| DDSD_HEIGHT
			| DDSD_WIDTH
			| DDSD_PIXELFORMAT
			| DDSD_CAPS
			| (1 < _depth            ? DDSD_DEPTH       : 0)
			| (1 < _numMips          ? DDSD_MIPMAPCOUNT : 0)
			| (isCompressed(_format) ? DDSD_LINEARSIZE  : DDSD_PITCH)
			)
			, _err
			);
		const uint32_t pitchOrLinearSize = isCompressed(_format)
			? _width*_height*bpp/8
			: _width*bpp/8
			;

		total += bx::write(_writer, _height, _err);
		total += bx::write(_writer, _width, _err);
		total += bx::write(_writer, pitchOrLinearSize, _err);
		total += bx::write(_writer, _depth, _err);
		total += bx::write(_writer, uint32_t(_numMips), _err);

		total += bx::writeRep(_writer, 0, 44, _err); // reserved1

		if (UINT32_MAX != ddspf)
		{
			const TranslateDdsPixelFormat& pf = s_translateDdsPixelFormat[ddspf];

			total += bx::write(_writer, uint32_t(8*sizeof(uint32_t) ), _err); // pixelFormatSize
			total += bx::write(_writer, pf.m_flags, _err);
			total += bx::write(_writer, uint32_t(0), _err);
			total += bx::write(_writer, pf.m_bitCount, _err);
			total += bx::write(_writer, pf.m_bitmask, _err);
		}
		else
		{
			total += bx::write(_writer, uint32_t(8*sizeof(uint32_t) ), _err); // pixelFormatSize
			total += bx::write(_writer, uint32_t(DDPF_FOURCC), _err);
			total += bx::write(_writer, uint32_t(DDS_DX10), _err);
			total += bx::write(_writer, uint32_t(0), _err); // bitCount
			total += bx::writeRep(_writer, 0, 4*sizeof(uint32_t), _err); // bitmask
		}

		uint32_t caps[4] =
		{
			uint32_t(DDSCAPS_TEXTURE | (1 < _numMips ? DDSCAPS_COMPLEX|DDSCAPS_MIPMAP : 0) ),
			uint32_t(_cubeMap ? DDSCAPS2_CUBEMAP|DSCAPS2_CUBEMAP_ALLSIDES : 0),
			0,
			0,
		};
		total += bx::write(_writer, caps, sizeof(caps) );

		total += bx::writeRep(_writer, 0, 4, _err); // reserved2

		BX_WARN(total-headerStart == DDS_HEADER_SIZE
			, "DDS: Failed to write header size %d (expected: %d)."
			, total-headerStart
			, DDS_HEADER_SIZE
			);

		if (UINT32_MAX != dxgiFormat)
		{
			total += bx::write(_writer, dxgiFormat);
			total += bx::write(_writer, uint32_t(1 < _depth ? DDS_DX10_DIMENSION_TEXTURE3D : DDS_DX10_DIMENSION_TEXTURE2D), _err); // dims
			total += bx::write(_writer, uint32_t(_cubeMap   ? DDS_DX10_MISC_TEXTURECUBE    : 0), _err); // miscFlags
			total += bx::write(_writer, uint32_t(1), _err); // arraySize
			total += bx::write(_writer, uint32_t(0), _err); // miscFlags2

			BX_WARN(total-headerStart == DDS_HEADER_SIZE+20
				, "DDS: Failed to write header size %d (expected: %d)."
				, total-headerStart
				, DDS_HEADER_SIZE+20
				);
			BX_UNUSED(headerStart);
		}

		return total;
	}

	int32_t imageWriteDds(bx::WriterI* _writer, ImageContainer& _imageContainer, const void* _data, uint32_t _size, bx::Error* _err)
	{
		BX_ERROR_SCOPE(_err);

		int32_t total = 0;
		total += imageWriteDdsHeader(_writer
			, TextureFormat::Enum(_imageContainer.m_format)
			, _imageContainer.m_cubeMap
			, _imageContainer.m_width
			, _imageContainer.m_height
			, _imageContainer.m_depth
			, _imageContainer.m_numMips
			, _err
			);

		if (!_err->isOk() )
		{
			return total;
		}

		for (uint8_t side = 0, numSides = _imageContainer.m_cubeMap ? 6 : 1; side < numSides && _err->isOk(); ++side)
		{
			for (uint8_t lod = 0, num = _imageContainer.m_numMips; lod < num && _err->isOk(); ++lod)
			{
				ImageMip mip;
				if (imageGetRawData(_imageContainer, side, lod, _data, _size, mip) )
				{
					total += bx::write(_writer, mip.m_data, mip.m_size, _err);
				}
			}
		}

		return total;
	}

	static int32_t imageWriteKtxHeader(bx::WriterI* _writer, TextureFormat::Enum _format, bool _cubeMap, uint32_t _width, uint32_t _height, uint32_t _depth, uint8_t _numMips, uint32_t _numLayers, bx::Error* _err)
	{
		BX_ERROR_SCOPE(_err);

		const KtxFormatInfo& tfi = s_translateKtxFormat[_format];

		int32_t total = 0;
		total += bx::write(_writer, "\xabKTX 11\xbb\r\n\x1a\n", 12, _err);
		total += bx::write(_writer, uint32_t(0x04030201), _err);
		total += bx::write(_writer, uint32_t(0), _err); // glType
		total += bx::write(_writer, uint32_t(1), _err); // glTypeSize
		total += bx::write(_writer, uint32_t(0), _err); // glFormat
		total += bx::write(_writer, tfi.m_internalFmt, _err); // glInternalFormat
		total += bx::write(_writer, tfi.m_fmt, _err); // glBaseInternalFormat
		total += bx::write(_writer, _width, _err);
		total += bx::write(_writer, _height, _err);
		total += bx::write(_writer, _depth, _err);
		total += bx::write(_writer, _numLayers, _err); // numberOfArrayElements
		total += bx::write(_writer, _cubeMap ? uint32_t(6) : uint32_t(0), _err);
		total += bx::write(_writer, uint32_t(_numMips), _err);
		total += bx::write(_writer, uint32_t(0), _err); // Meta-data size.

		BX_WARN(total == 64, "KTX: Failed to write header size %d (expected: %d).", total, 64);
		return total;
	}

	int32_t imageWriteKtx(bx::WriterI* _writer, TextureFormat::Enum _format, bool _cubeMap, uint32_t _width, uint32_t _height, uint32_t _depth, uint8_t _numMips, uint32_t _numLayers, const void* _src, bx::Error* _err)
	{
		BX_ERROR_SCOPE(_err);

		int32_t total = 0;
		total += imageWriteKtxHeader(_writer, _format, _cubeMap, _width, _height, _depth, _numMips, _numLayers, _err);

		if (!_err->isOk() )
		{
			return total;
		}

		const ImageBlockInfo& blockInfo = s_imageBlockInfo[_format];
		const uint8_t  bpp         = blockInfo.bitsPerPixel;
		const uint32_t blockWidth  = blockInfo.blockWidth;
		const uint32_t blockHeight = blockInfo.blockHeight;
		const uint32_t minBlockX   = blockInfo.minBlockX;
		const uint32_t minBlockY   = blockInfo.minBlockY;

		const uint8_t* src = (const uint8_t*)_src;

		const uint32_t numLayers = bx::uint32_max(_numLayers, 1);
		const uint32_t numSides = _cubeMap ? 6 : 1;

		uint32_t width  = _width;
		uint32_t height = _height;
		uint32_t depth  = _depth;

		for (uint8_t lod = 0; lod < _numMips && _err->isOk(); ++lod)
		{
			width  = bx::uint32_max(blockWidth  * minBlockX, ( (width  + blockWidth  - 1) / blockWidth )*blockWidth);
			height = bx::uint32_max(blockHeight * minBlockY, ( (height + blockHeight - 1) / blockHeight)*blockHeight);
			depth  = bx::uint32_max(1, depth);

			const uint32_t mipSize = width*height*depth*bpp/8;
			const uint32_t size = mipSize*numLayers*numSides;
			total += bx::write(_writer, size, _err);

			for (uint32_t layer = 0; layer < numLayers && _err->isOk(); ++layer)
			{
				for (uint8_t side = 0; side < numSides && _err->isOk(); ++side)
				{
					total += bx::write(_writer, src, size, _err);
					src += size;
				}
			}

			width  >>= 1;
			height >>= 1;
			depth  >>= 1;
		}

		return total;
	}

	int32_t imageWriteKtx(bx::WriterI* _writer, ImageContainer& _imageContainer, const void* _data, uint32_t _size, bx::Error* _err)
	{
		BX_ERROR_SCOPE(_err);

		int32_t total = 0;
		total += imageWriteKtxHeader(_writer
			, TextureFormat::Enum(_imageContainer.m_format)
			, _imageContainer.m_cubeMap
			, _imageContainer.m_width
			, _imageContainer.m_height
			, _imageContainer.m_depth
			, _imageContainer.m_numMips
			, _imageContainer.m_numLayers
			, _err
			);

		if (!_err->isOk() )
		{
			return total;
		}

		const uint32_t numMips   = _imageContainer.m_numMips;
		const uint32_t numLayers = bx::uint32_max(_imageContainer.m_numLayers, 1);
		const uint32_t numSides  = _imageContainer.m_cubeMap ? 6 : 1;

		for (uint8_t lod = 0; lod < numMips && _err->isOk(); ++lod)
		{
			ImageMip mip;
			imageGetRawData(_imageContainer, 0, lod, _data, _size, mip);

			const uint32_t size = mip.m_size*numSides*numLayers;
			total += bx::write(_writer, size, _err);

			for (uint32_t layer = 0; layer < numLayers && _err->isOk(); ++layer)
			{
				for (uint8_t side = 0; side < numSides && _err->isOk(); ++side)
				{
					if (imageGetRawData(_imageContainer, uint16_t(layer*numSides + side), lod, _data, _size, mip) )
					{
						total += bx::write(_writer, mip.m_data, mip.m_size, _err);
					}
				}
			}
		}

		return total;
	}

} // namespace bimg
