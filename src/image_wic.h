/*
 * Copyright 2011-2026 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bimg/blob/master/LICENSE
 */

#ifndef BIMG_IMAGE_WIC_H_HEADER_GUARD
#define BIMG_IMAGE_WIC_H_HEADER_GUARD

#include "bimg_p.h"

#if BX_PLATFORM_WINDOWS

#include <bx/os.h>

#ifndef WIN32_LEAN_AND_MEAN
#	define WIN32_LEAN_AND_MEAN
#endif // WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#	define NOMINMAX
#endif // NOMINMAX
#include <windows.h>
#include <objbase.h>
#include <wincodec.h>

#define WIC_RELEASE(_ptr)              \
			BX_MACRO_BLOCK_BEGIN       \
				if (NULL != (_ptr) )   \
				{                      \
					(_ptr)->Release(); \
					(_ptr) = NULL;     \
				}                      \
			BX_MACRO_BLOCK_END

namespace bimg
{
	static const GUID kGUID_WICPixelFormat32bppRGBA = { 0xf5c7ad2d, 0x6a8d, 0x43dd, { 0xa7, 0xa8, 0xa2, 0x99, 0x35, 0x26, 0x1a, 0xe9 } };

	struct WicContext
	{
		WicContext()
			: m_wicDll(NULL)
			, m_ole32Dll(NULL)
			, m_coUninitialize(NULL)
			, m_factory(NULL)
		{
		}

		~WicContext()
		{
			WIC_RELEASE(m_factory);

			if (NULL != m_coUninitialize)
			{
				m_coUninitialize();
			}

			if (NULL != m_ole32Dll)
			{
				bx::dlclose(m_ole32Dll);
			}

			if (NULL != m_wicDll)
			{
				bx::dlclose(m_wicDll);
			}
		}

		bool init(bx::Error* _err)
		{
			static const GUID kCLSID_WICImagingFactory = { 0xcacaf262, 0x9370, 0x4615, { 0xa1, 0x3b, 0x9f, 0x55, 0x39, 0xda, 0x4c, 0x0a } };

			m_wicDll = bx::dlopen("windowscodecs.dll");
			if (NULL == m_wicDll)
			{
				BX_ERROR_SET(_err, BIMG_ERROR, "WIC: windowscodecs.dll is not available.");
				return false;
			}

			typedef HRESULT (WINAPI* PFN_DllGetClassObject)(REFCLSID, REFIID, LPVOID*);
			PFN_DllGetClassObject dllGetClassObject = bx::dlsym<PFN_DllGetClassObject>(m_wicDll, "DllGetClassObject");
			if (NULL == dllGetClassObject)
			{
				BX_ERROR_SET(_err, BIMG_ERROR, "WIC: DllGetClassObject is not available.");
				return false;
			}

			m_ole32Dll = bx::dlopen("ole32.dll");
			if (NULL != m_ole32Dll)
			{
				typedef HRESULT (WINAPI* PFN_CoInitializeEx)(LPVOID, DWORD);
				typedef void    (WINAPI* PFN_CoUninitialize)(void);
				PFN_CoInitializeEx coInitializeEx = bx::dlsym<PFN_CoInitializeEx>(m_ole32Dll, "CoInitializeEx");
				PFN_CoUninitialize coUninitialize = bx::dlsym<PFN_CoUninitialize>(m_ole32Dll, "CoUninitialize");

				if (NULL != coInitializeEx)
				{
					const HRESULT hr = coInitializeEx(NULL, COINIT_MULTITHREADED);
					if (S_OK == hr
					||  S_FALSE == hr)
					{
						m_coUninitialize = coUninitialize;
					}
				}
			}

			IClassFactory* classFactory = NULL;
			if (SUCCEEDED(dllGetClassObject(kCLSID_WICImagingFactory, __uuidof(IClassFactory), (void**)&classFactory) )
			&&  NULL != classFactory)
			{
				classFactory->CreateInstance(NULL, __uuidof(IWICImagingFactory), (void**)&m_factory);
				classFactory->Release();
			}

			if (NULL == m_factory)
			{
				BX_ERROR_SET(_err, BIMG_ERROR, "WIC: Failed to create imaging factory.");
				return false;
			}

			return true;
		}

		void*               m_wicDll;
		void*               m_ole32Dll;
		void                (WINAPI* m_coUninitialize)(void);
		IWICImagingFactory* m_factory;
	};

} // namespace bimg

#endif // BX_PLATFORM_WINDOWS

#endif // BIMG_IMAGE_WIC_H_HEADER_GUARD
