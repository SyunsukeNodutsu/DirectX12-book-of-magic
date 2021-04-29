//-----------------------------------------------------------------------------
// File: main.cpp
//
// Edit: 2021/03/29 野筒隼輔
//-----------------------------------------------------------------------------
#include <Windows.h>
#include <tchar.h>
#ifdef _DEBUG
#include <iostream>
#endif // _DEBUG

constexpr unsigned int window_width = 1280;
constexpr unsigned int window_height = 720;

//-----------------------------------------------------------------------------
// Name: DebugOutputFormatString()
// Desc: コンソール画面にフォーマット対応の文字列を表示 可変長引数
//-----------------------------------------------------------------------------
void DebugOutputFormatString(const char* format, ...)
{
#ifdef _DEBUG
	va_list valist;
	va_start(valist, format);
	vprintf(format, valist);
	va_end(valist);
#endif // _DEBUG
}

//-----------------------------------------------------------------------------
// Name: WindowProcedure()
// Desc: ウィンドウプロシージャ
//-----------------------------------------------------------------------------
LRESULT WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	if (msg == WM_DESTROY) {
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

//-----------------------------------------------------------------------------
// Name: main()
// Desc: メインエントリ
//-----------------------------------------------------------------------------
#ifdef _DEBUG
int main()
{
#else
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
#endif // _DEBUG
	DebugOutputFormatString("Show Window Test.");

	//--------------------------------------------------
	// ウィンドウクラスの作成.登録
	//--------------------------------------------------
	WNDCLASSEX w = {};
	w.cbSize = sizeof(WNDCLASSEX);
	w.lpfnWndProc = (WNDPROC)WindowProcedure;
	w.lpszClassName = _T("DirectX12Sample");
	w.hInstance = GetModuleHandle(nullptr);
	RegisterClassEx(&w);

	RECT wrc = { 0, 0, window_width, window_height };
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	HWND hwnd = CreateWindow(
		w.lpszClassName,
		_T("DirectX12 book of magic"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		wrc.right - wrc.left,
		wrc.bottom - wrc.top,
		nullptr,
		nullptr,
		w.hInstance,
		nullptr,
		);

	//--------------------------------------------------
	// ウィンドウ表示
	//--------------------------------------------------
	ShowWindow(hwnd, SW_SHOW);

	//--------------------------------------------------
	// メッセージループ
	//--------------------------------------------------
	MSG msg = {};
	while (true) {
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (msg.message == WM_QUIT) {
			break;
		}
	}

	// 登録解除
	UnregisterClass(w.lpszClassName, w.hInstance);

	return 0;
}