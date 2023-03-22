#pragma once

#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

#define DX_SAFE_RELEASE(dxObject)	\
{									\
	if (( dxObject) != nullptr)		\
	{								\
		(dxObject)->Release();		\
		(dxObject) = nullptr;		\
	}								\
}


