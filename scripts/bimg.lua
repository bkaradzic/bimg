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

function overridefiles(_srcPath, _dstPath, _files)

	local remove = {}
	local add = {}
	for _, file in ipairs(_files) do
		file = path.getrelative(_srcPath, file)
		local filePath = path.join(_dstPath, file)
		if not os.isfile(filePath) then return end

		table.insert(remove, path.join(_srcPath, file))
		table.insert(add, filePath)
	end

	removefiles {
		remove,
	}

	files {
		add,
	}
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
	}

	overridefiles(BIMG_DIR, path.join(BIMG_DIR, "../bimg-ext"), {
		path.join(BIMG_DIR, "src/image_gnf.cpp"),
	})

	configuration { "linux-*" }
		buildoptions {
			"-fPIC",
		}

	configuration {}

	if filesexist(BIMG_DIR, path.join(BIMG_DIR, "../bimg-ext"), { path.join(BIMG_DIR, "scripts/bimg.lua"), }) then
		dofile(path.join(BIMG_DIR, "../bimg-ext/scripts/bimg.lua") )
	end
