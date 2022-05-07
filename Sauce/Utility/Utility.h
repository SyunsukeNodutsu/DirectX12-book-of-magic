#pragma once

//TODO: pch.h
#include <Windows.h>
#include <DirectXMath.h>

#include <iostream>
#include <tchar.h>
#include <vector>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>

#include <DirectXTex.h>
#include <d3dx12.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib,"DirectXTex.lib")

//デバッグ用 便利機能
namespace Debug
{
	//@brief コンソール画面にフォーマット対応の文字列を表示 可変長引数
	void Log(const char* format, ...)
	{
#if _DEBUG
		va_list valist;
		va_start(valist, format);
		vprintf(format, valist);
		va_end(valist);
#endif
	}

}
