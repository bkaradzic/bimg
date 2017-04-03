/*
 * Copyright 2011-2017 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause
 */

#include <bimg/bimg.h>
#include <bx/endian.h>
#include <bx/error.h>
#include <bx/simd_t.h>

#define BIMG_CHUNK_MAGIC_TEX BX_MAKEFOURCC('T', 'E', 'X', 0x0)

namespace bimg
{
	struct Memory
	{
		uint8_t* data;
		uint32_t size;
	};

	struct TextureCreate
	{
		TextureFormat::Enum m_format;
		uint16_t m_width;
		uint16_t m_height;
		uint16_t m_depth;
		uint16_t m_numLayers;
		uint8_t m_numMips;
		bool m_cubeMap;
		const Memory* m_mem;
	};

	inline uint8_t calcNumMips(bool _hasMips, uint16_t _width, uint16_t _height, uint16_t _depth = 1)
	{
		if (_hasMips)
		{
			const uint32_t max = bx::uint32_max(bx::uint32_max(_width, _height), _depth);
			const uint32_t num = 1 + uint32_t(bx::flog2(float(max) ) );

			return uint8_t(num);
		}

		return 1;
	}

} // namespace bimg
