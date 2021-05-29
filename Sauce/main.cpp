//-----------------------------------------------------------------------------
// File: main.cpp
//
// DirectX12 Test
// とりあえずmain.cppのみでテスト
// 後に設計し直し
//
// EditHistry:
//  2021/05/29 リファクタリング
//-----------------------------------------------------------------------------
#include <Windows.h>
#include <tchar.h>
#ifdef _DEBUG
#include <iostream>
#include <assert.h>
#endif // _DEBUG

#include <d3d12.h>
#include <dxgi1_6.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

#include <vector>

constexpr unsigned int window_width = 1280;
constexpr unsigned int window_height = 720;

// Direct3D.DXGI
ID3D12Device*				_dev			= nullptr;
IDXGIFactory6*				_dxgiFactory	= nullptr;
IDXGISwapChain4*			_swapchain		= nullptr;

ID3D12CommandAllocator*		_cmdAllocator	= nullptr;
ID3D12GraphicsCommandList*	_cmdList		= nullptr;
ID3D12CommandQueue*			_cmdQueue		= nullptr;

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
	if (msg == WM_DESTROY)
	{
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
	w.cbSize			= sizeof(WNDCLASSEX);
	w.lpfnWndProc		= (WNDPROC)WindowProcedure;
	w.lpszClassName		= _T("DirectX12Sample");
	w.hInstance			= GetModuleHandle(nullptr);
	RegisterClassEx(&w);

	RECT wrc = { 0, 0, window_width, window_height };
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	HWND hwnd = CreateWindow(
		w.lpszClassName, _T("DirectX12 book of magic"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
		wrc.right - wrc.left, wrc.bottom - wrc.top, nullptr, nullptr, w.hInstance, nullptr
	);
	if (hwnd == nullptr) {
		assert(0 && "error create window.");
		return -1;
	}

	//--------------------------------------------------
	// Direct3D.DXGI関連
	//--------------------------------------------------

	HRESULT hr = S_OK;

	// DXGI作成
	hr = CreateDXGIFactory1(IID_PPV_ARGS(&_dxgiFactory));
	if (FAILED(hr)) {
		assert(0 && "error create DXGI.");
		return -1;
	}

	// D3DのLv列挙
	D3D_FEATURE_LEVEL levels[] = {
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};

	// デバイス作成
	for (auto lv : levels)
	{
		if (D3D12CreateDevice(nullptr, lv, IID_PPV_ARGS(&_dev)) == S_OK)
			break;
	}

	if (_dev == nullptr) {
		assert(0 && "error create device.");
		return -1;
	}

	// コマンドアロケータ作成
	hr = _dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&_cmdAllocator));
	if (FAILED(hr)) {
		assert(0 && "error create command allocator.");
		return -1;
	}

	// コマンドリスト作成
	hr = _dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _cmdAllocator, nullptr, IID_PPV_ARGS(&_cmdList));
	if (FAILED(hr)) {
		assert(0 && "error create command list.");
		return -1;
	}

	// コマンドキュー設定
	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};
	cmdQueueDesc.Flags		= D3D12_COMMAND_QUEUE_FLAG_NONE;
	cmdQueueDesc.NodeMask	= 0;
	cmdQueueDesc.Priority	= D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	cmdQueueDesc.Type		= D3D12_COMMAND_LIST_TYPE_DIRECT;
	// コマンドキュー作成
	hr = _dev->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&_cmdQueue));
	if (FAILED(hr)) {
		assert(0 && "error create command queue.");
		return -1;
	}

	// スワップチェイン設定
	DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};
	swapchainDesc.Width					= window_width;
	swapchainDesc.Height				= window_height;
	swapchainDesc.Format				= DXGI_FORMAT_R8G8B8A8_UNORM;
	swapchainDesc.Stereo				= false;
	swapchainDesc.SampleDesc.Count		= 1;
	swapchainDesc.SampleDesc.Quality	= 0;
	swapchainDesc.BufferUsage			= DXGI_USAGE_BACK_BUFFER;
	swapchainDesc.BufferCount			= 2;
	// バックバッファは伸び縮み可能
	swapchainDesc.Scaling				= DXGI_SCALING_STRETCH;
	// フリップ後は即破棄
	swapchainDesc.SwapEffect			= DXGI_SWAP_EFFECT_FLIP_DISCARD;
	// 特に指定なし
	swapchainDesc.AlphaMode				= DXGI_ALPHA_MODE_UNSPECIFIED;
	// ウィンドウ <-> フルスクリーン切り替え可能
	swapchainDesc.Flags					= DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	// スワップチェイン作成
	hr = _dxgiFactory->CreateSwapChainForHwnd(_cmdQueue, hwnd, &swapchainDesc, nullptr, nullptr, (IDXGISwapChain1**)&_swapchain);
	if (FAILED(hr)) {
		assert(0 && "error create swap chain.");
		return -1;
	}

	// ディスクリプタヒープ設定
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Type			= D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heapDesc.NodeMask		= 0;
	heapDesc.NumDescriptors = 2;
	heapDesc.Flags			= D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	// ディスクリプタヒープ作成
	ID3D12DescriptorHeap* rtvHeaps = nullptr;
	hr = _dev->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&rtvHeaps));
	if (FAILED(hr)) {
		assert(0 && "error create descriptor heap.");
		return -1;
	}

	// スワップチェインのメモリと紐づけ
	std::vector<ID3D12Resource*> _backBuffer(swapchainDesc.BufferCount);
	for (UINT idx = 0; idx < swapchainDesc.BufferCount; ++idx)
	{
		hr = _swapchain->GetBuffer(idx, IID_PPV_ARGS(&_backBuffer[idx]));
		// サイズを考慮してptrをずらす
		D3D12_CPU_DESCRIPTOR_HANDLE handle = rtvHeaps->GetCPUDescriptorHandleForHeapStart();
		handle.ptr += idx * _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		// RTV作成
		_dev->CreateRenderTargetView(_backBuffer[idx], nullptr, handle);
	}

	// ここまでで初期化完了

	ShowWindow(hwnd, SW_SHOW);

	//--------------------------------------------------
	// メッセージループ
	//--------------------------------------------------

	MSG msg = {};
	while (true)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (msg.message == WM_QUIT)
			break;

		// DirectX処理

		// バックバッファのインデックスを取得
		auto bbIdx = _swapchain->GetCurrentBackBufferIndex();

		// レンダーターゲットを指定
		auto rtvH = rtvHeaps->GetCPUDescriptorHandleForHeapStart();
		rtvH.ptr += static_cast<ULONG_PTR>(bbIdx * _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
		_cmdList->OMSetRenderTargets(1, &rtvH, false, nullptr);

		// 画面クリア
		float clearColor[] = { 0.0f, 0.0f, 1.0f, 1.0f };
		_cmdList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);

		// 命令のクローズ
		hr = _cmdList->Close();
		if (FAILED(hr)) {
			assert(0 && "error create CommandList->Close().");
			return -1;
		}

		//コマンドリストの実行
		ID3D12CommandList* cmdlists[] = { _cmdList };
		_cmdQueue->ExecuteCommandLists(1, cmdlists);

		//キューをクリア
		hr = _cmdAllocator->Reset();
		if (FAILED(hr)) {
			assert(0 && "error create CommandAllocator->Reset().");
			return -1;
		}
		//再びコマンドリストをためる準備
		hr = _cmdList->Reset(_cmdAllocator, nullptr);
		if (FAILED(hr)) {
			assert(0 && "error create CommandList->Reset().");
			return -1;
		}

		//フリップ
		_swapchain->Present(1, 0);
	}

	// 登録解除
	UnregisterClass(w.lpszClassName, w.hInstance);

	return 0;
}
