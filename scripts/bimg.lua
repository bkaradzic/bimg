--
-- Copyright 2010-2017 Branimir Karadzic. All rights reserved.
-- License: https://github.com/bkaradzic/bx#license-bsd-2-clause
--

function filesexist(_srcPath, _dstPath, _files)
	for _, file in ipairs(_files) do
		file = path.getrelative(_srcPath, file)
		local filePath = path.join(_dstPath, file)
		if not os.isfile(filePath) then return false end
	end

	return true
end

project "bimg"
	kind "StaticLib"

	includedirs {
		path.join(BX_DIR, "include"),
		path.join(BIMG_DIR, "include"),
	}

	files {
		path.join(BIMG_DIR, "include/**"),
		path.join(BIMG_DIR, "src/image.*"),
		path.join(BIMG_DIR, "src/image_gnf.cpp"),
	}

--[[
	if filesexist(BIMG_DIR, path.join(BIMG_DIR, "../bimg-ext"), {
		path.join(BIMG_DIR, "scripts/bimg.lua"), }) then

		if filesexist(BIMG_DIR, path.join(BIMG_DIR, "../bimg-ext"), {
			path.join(BIMG_DIR, "src/image_gnf.cpp"), }) then

			removefiles {
				path.join(BIMG_DIR, "src/image_gnf.cpp"),
			}
		end

		dofile(path.join(BIMG_DIR, "../bimg-ext/scripts/bimg.lua") )
	end]]

	configuration { "linux-*" }
		buildoptions {
			"-fPIC",
		}

	configuration {}
