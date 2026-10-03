/*
 * Copyright 2011-2026 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bimg/blob/master/LICENSE
 */

#ifndef BIMG_DECODE_H_HEADER_GUARD
#define BIMG_DECODE_H_HEADER_GUARD

#include "bimg.h"

namespace bimg
{
	/// PNG color-key transparency preserves source RGB in fully transparent pixels.
	ImageContainer* imageParse(
		  bx::AllocatorI* _allocator
		, const void* _data
		, uint32_t _size
		, TextureFormat::Enum _dstFormat = TextureFormat::Count
		, bx::Error* _err = NULL
		);

	///
	bool imageParseInfo(
		  bx::AllocatorI* _allocator
		, ImageContainer& _imageContainer
		, const void* _data
		, uint32_t _size
		, bx::Error* _err = NULL
		);

	/// Rasterizes SVG image at the requested size.
	///
	/// @param[in] _width Width in pixels. When 0 it follows from `_height`.
	/// @param[in] _height Height in pixels. When 0 it follows from `_width`.
	///
	/// @remarks
	///   Aspect ratio is always preserved. When both `_width` and `_height` are given the image is
	///   scaled to fit inside of them. When both are 0 documents that fit inside of 4096x4096 are
	///   rasterized at native scale, with pixel dimensions rounded to the nearest integer (half
	///   up). Dimensions rounding to 0 are rejected. Larger documents are scaled down to fit
	///   inside of 4096x4096. `bimg::imageParse` and `bimg::imageParseInfo` use the same size policy.
	///
	ImageContainer* imageParseSvg(
		  bx::AllocatorI* _allocator
		, const void* _data
		, uint32_t _size
		, uint32_t _width
		, uint32_t _height
		, bx::Error* _err = NULL
		);

	/// Returns a NULL-terminated list of the lower-case file name extensions for
	/// the image formats that are compiled into this build.
	const char* const* getSupportedExt();

} // namespace bimg

#endif // BIMG_DECODE_H_HEADER_GUARD
