//-----------------------------------------------------------------------------
// File: main.cpp
//
// DirectX12の魔導書
//
// とりあえずmain.cppのみでテスト
// 後に設計し直し
//
// EditHistry:
//  2021/05/29 画面クリア
//  2021/05/30 ポリゴン描画
//-----------------------------------------------------------------------------
#include <Windows.h>
#include <DirectXMath.h>

#include <iostream>
#include <tchar.h>
#include <vector>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>

// リンカ リンク
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"d3dcompiler.lib")

// 算術系 エイリアス
using namespace DirectX;

// Direct3D.DXGI
ID3D12Device*				_dev			= nullptr;
IDXGIFactory6*				_dxgiFactory	= nullptr;
IDXGISwapChain4*			_swapchain		= nullptr;

ID3D12CommandAllocator*		_cmdAllocator	= nullptr;
ID3D12GraphicsCommandList*	_cmdList		= nullptr;
ID3D12CommandQueue*			_cmdQueue		= nullptr;

// ウィンドウ 矩形
constexpr unsigned int		window_width	= 1280;
constexpr unsigned int		window_height	= 720;

// アダプタ名
std::wstring				_adapterName	= L"";

//-----------------------------------------------------------------------------
// コンソール画面にフォーマット対応の文字列を表示 可変長引数
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
// デバッグレイヤーを有効に
//-----------------------------------------------------------------------------
void EnableDebugLayer()
{
	ID3D12Debug* debugLayer = nullptr;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugLayer))))
	{
		debugLayer->EnableDebugLayer();// デバッグレイヤーを有効
		debugLayer->Release();// 有効化したら即解放
	}
}

//-----------------------------------------------------------------------------
// ウィンドウプロシージャ
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
// メインエントリ
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

#ifdef _DEBUG
	// デバッグレイヤー有効
	EnableDebugLayer();
#endif // _DEBUG

	// DXGI作成
	hr = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&_dxgiFactory));
	if (FAILED(hr)) {
		assert(0 && "error create DXGI.");
		return -1;
	}

	// アダプター設定
	std::vector <IDXGIAdapter*> adapters;
	IDXGIAdapter* tmpAdapter = nullptr;
	for (int i = 0; _dxgiFactory->EnumAdapters(i, &tmpAdapter) != DXGI_ERROR_NOT_FOUND; ++i)
		adapters.push_back(tmpAdapter);

	for (auto adpt : adapters)
	{
		DXGI_ADAPTER_DESC adesc = {};
		adpt->GetDesc(&adesc);
		std::wstring strDesc = adesc.Description;
		if (strDesc.find(L"NVIDIA") != std::string::npos)
		{
			tmpAdapter = adpt;
			_adapterName = strDesc;
			break;
		}
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
		if (D3D12CreateDevice(tmpAdapter, lv, IID_PPV_ARGS(&_dev)) == S_OK)
			break;

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
	swapchainDesc.Flags					= 0/*DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH*/;

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
	std::vector<ID3D12Resource*> _backBuffers(swapchainDesc.BufferCount);
	for (UINT idx = 0; idx < swapchainDesc.BufferCount; ++idx)
	{
		hr = _swapchain->GetBuffer(idx, IID_PPV_ARGS(&_backBuffers[idx]));
		// サイズを考慮してptrをずらす
		D3D12_CPU_DESCRIPTOR_HANDLE handle = rtvHeaps->GetCPUDescriptorHandleForHeapStart();
		handle.ptr += idx * _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		// RTV作成
		_dev->CreateRenderTargetView(_backBuffers[idx], nullptr, handle);
	}

	ID3D12Fence* _fence = nullptr;
	UINT64 _fenceVal = 0;
	hr = _dev->CreateFence(_fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence));

	// ここまでで初期化完了

	ShowWindow(hwnd, SW_SHOW);

	// 以下 ポリゴン描画テスト

	// 頂点情報
	XMFLOAT3 vertices[] =
	{
		{ -0.4f, -0.7f, 0.0f },// 左下
		{ -0.4f,  0.7f, 0.0f },// 左上
		{  0.4f, -0.7f, 0.0f },// 右下
		{  0.4f,  0.7f, 0.0f },// 右上
	};

	// 頂点バッファーの作成
	D3D12_HEAP_PROPERTIES heapprop = {};
	heapprop.Type					= D3D12_HEAP_TYPE_UPLOAD;
	heapprop.CPUPageProperty		= D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapprop.MemoryPoolPreference	= D3D12_MEMORY_POOL_UNKNOWN;

	D3D12_RESOURCE_DESC resdesc = {};
	resdesc.Dimension			= D3D12_RESOURCE_DIMENSION_BUFFER;
	resdesc.Width				= sizeof(vertices);
	resdesc.Height				= 1;
	resdesc.DepthOrArraySize	= 1;
	resdesc.MipLevels			= 1;
	resdesc.Format				= DXGI_FORMAT_UNKNOWN;
	resdesc.SampleDesc.Count	= 1;
	resdesc.Flags				= D3D12_RESOURCE_FLAG_NONE;
	resdesc.Layout				= D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	// UPLOAD(確保は可能)
	ID3D12Resource* vertBuff = nullptr;
	hr = _dev->CreateCommittedResource(&heapprop, D3D12_HEAP_FLAG_NONE, &resdesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertBuff));

	XMFLOAT3* vertMap = nullptr;
	hr = vertBuff->Map(0, nullptr, (void**)&vertMap);

	std::copy(std::begin(vertices), std::end(vertices), vertMap);

	vertBuff->Unmap(0, nullptr);

	D3D12_VERTEX_BUFFER_VIEW vbView = {};
	vbView.BufferLocation	= vertBuff->GetGPUVirtualAddress();	// バッファの仮想アドレス
	vbView.SizeInBytes		= sizeof(vertices);					// 全バイト数
	vbView.StrideInBytes	= sizeof(vertices[0]);				// 1頂点あたりのバイト数

	unsigned short indices[] = { 0, 1, 2, 2, 1, 3 };

	ID3D12Resource* idxBuff = nullptr;
	// 設定はバッファのサイズ以外頂点バッファの設定を使いまわしてOK
	resdesc.Width = sizeof(indices);
	hr = _dev->CreateCommittedResource(&heapprop, D3D12_HEAP_FLAG_NONE, &resdesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&idxBuff));

	// 作ったバッファにインデックスデータをコピー
	unsigned short* mappedIdx = nullptr;
	idxBuff->Map(0, nullptr, (void**)&mappedIdx);
	std::copy(std::begin(indices), std::end(indices), mappedIdx);
	idxBuff->Unmap(0, nullptr);

	// インデックスバッファビューを作成
	D3D12_INDEX_BUFFER_VIEW ibView = {};
	ibView.BufferLocation	= idxBuff->GetGPUVirtualAddress();
	ibView.Format			= DXGI_FORMAT_R16_UINT;
	ibView.SizeInBytes		= sizeof(indices);

	ID3DBlob* _vsBlob = nullptr;
	ID3DBlob* _psBlob = nullptr;

	ID3DBlob* errorBlob = nullptr;

	// 頂点シェーダコンパイル 作成
	hr = D3DCompileFromFile(L"Sauce/BasicVertexShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"BasicVS", "vs_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &_vsBlob, &errorBlob);

	if (FAILED(hr))
	{
		if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
		{
			assert(0 && "error not found shader file");
		}
		else
		{
			std::string errstr;
			errstr.resize(errorBlob->GetBufferSize());
			std::copy_n((char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize(), errstr.begin());
			errstr += "\n";
			assert(0 && errstr.c_str());
		}
		return -1;
	}

	// ピクセルシェーダコンパイル 作成
	hr = D3DCompileFromFile(L"Sauce/BasicPixelShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"BasicPS", "ps_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &_psBlob, &errorBlob);

	if (FAILED(hr))
	{
		if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
		{
			assert(0 && "error not found shader file");
		}
		else
		{
			std::string errstr;
			errstr.resize(errorBlob->GetBufferSize());
			std::copy_n((char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize(), errstr.begin());
			errstr += "\n";
			assert(0 && errstr.c_str());
		}
		return -1;
	}

	// 頂点入力レイアウト
	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	// レンダーターゲットブレンド設定
	D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc = {};
	renderTargetBlendDesc.BlendEnable				= false;// ひとまず加算や乗算やαブレンディングは使用しない
	renderTargetBlendDesc.RenderTargetWriteMask		= D3D12_COLOR_WRITE_ENABLE_ALL;
	renderTargetBlendDesc.LogicOpEnable				= false;// ひとまず論理演算は使用しない

	// ルートシグネチャ作成
	ID3D12RootSignature* rootsignature = nullptr;
	ID3DBlob* rootSigBlob = nullptr;

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSigBlob, &errorBlob);
	hr = _dev->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(&rootsignature));
	rootSigBlob->Release();

	// グラフィックスパイプライン設定
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline = {};
	gpipeline.pRootSignature						= nullptr;
	gpipeline.VS.pShaderBytecode					= _vsBlob->GetBufferPointer();
	gpipeline.VS.BytecodeLength						= _vsBlob->GetBufferSize();
	gpipeline.PS.pShaderBytecode					= _psBlob->GetBufferPointer();
	gpipeline.PS.BytecodeLength						= _psBlob->GetBufferSize();
	gpipeline.SampleMask							= D3D12_DEFAULT_SAMPLE_MASK;// 中身は0xffffffff

	gpipeline.BlendState.AlphaToCoverageEnable		= false;
	gpipeline.BlendState.IndependentBlendEnable		= false;
	gpipeline.BlendState.RenderTarget[0]			= renderTargetBlendDesc;

	gpipeline.RasterizerState.MultisampleEnable		= false;				// まだアンチェリは使わない
	gpipeline.RasterizerState.CullMode				= D3D12_CULL_MODE_NONE;	// カリングしない
	gpipeline.RasterizerState.FillMode				= D3D12_FILL_MODE_SOLID;// 中身を塗りつぶす
	gpipeline.RasterizerState.DepthClipEnable		= true;					// 深度方向のクリッピングは有効に

	gpipeline.RasterizerState.FrontCounterClockwise	= false;
	gpipeline.RasterizerState.DepthBias				= D3D12_DEFAULT_DEPTH_BIAS;
	gpipeline.RasterizerState.DepthBiasClamp		= D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	gpipeline.RasterizerState.SlopeScaledDepthBias	= D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	gpipeline.RasterizerState.AntialiasedLineEnable = false;
	gpipeline.RasterizerState.ForcedSampleCount		= 0;
	gpipeline.RasterizerState.ConservativeRaster	= D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	gpipeline.DepthStencilState.DepthEnable			= false;
	gpipeline.DepthStencilState.StencilEnable		= false;

	gpipeline.InputLayout.pInputElementDescs		= inputLayout;			// レイアウト先頭アドレス
	gpipeline.InputLayout.NumElements				= _countof(inputLayout);// レイアウト配列数

	gpipeline.IBStripCutValue						= D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;// ストリップ時のカットなし
	gpipeline.PrimitiveTopologyType					= D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;// 三角形で構成

	gpipeline.NumRenderTargets						= 1;// 今は１つのみ
	gpipeline.RTVFormats[0]							= DXGI_FORMAT_R8G8B8A8_UNORM;// 0～1に正規化されたRGBA

	gpipeline.SampleDesc.Count						= 1;// サンプリングは1ピクセルにつき１
	gpipeline.SampleDesc.Quality					= 0;// クオリティは最低

	gpipeline.pRootSignature						= rootsignature;

	// グラフィックスパイプライン作成
	ID3D12PipelineState* _pipelinestate = nullptr;
	hr = _dev->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(&_pipelinestate));
	if (FAILED(hr) && _pipelinestate == nullptr) {
		assert(0 && "error create pipeline state.");
		return -1;
	}

	// ビューポート設定
	D3D12_VIEWPORT viewport = {};
	viewport.Width		= window_width;	// 出力先の幅(ピクセル数)
	viewport.Height		= window_height;// 出力先の高さ(ピクセル数)
	viewport.TopLeftX	= 0;			// 出力先の左上座標X
	viewport.TopLeftY	= 0;			// 出力先の左上座標Y
	viewport.MaxDepth	= 1.0f;			// 深度最大値
	viewport.MinDepth	= 0.0f;			// 深度最小値

	// 切り抜き座標設定
	D3D12_RECT scissorrect = {};
	scissorrect.top		= 0;
	scissorrect.left	= 0;
	scissorrect.right	= scissorrect.left + window_width;
	scissorrect.bottom	= scissorrect.top + window_height;

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

		// リソースバリア設定
		D3D12_RESOURCE_BARRIER BarrierDesc = {};
		BarrierDesc.Type					= D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		BarrierDesc.Flags					= D3D12_RESOURCE_BARRIER_FLAG_NONE;
		BarrierDesc.Transition.pResource	= _backBuffers[bbIdx];
		BarrierDesc.Transition.Subresource	= D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		BarrierDesc.Transition.StateBefore	= D3D12_RESOURCE_STATE_PRESENT;
		BarrierDesc.Transition.StateAfter	= D3D12_RESOURCE_STATE_RENDER_TARGET;

		// リソースバリア
		_cmdList->ResourceBarrier(1, &BarrierDesc);

		// パイプラインステート設定
		if (_pipelinestate != 0)
			_cmdList->SetPipelineState(_pipelinestate);

		// レンダーターゲットを指定
		auto rtvH = rtvHeaps->GetCPUDescriptorHandleForHeapStart();
		rtvH.ptr += static_cast<ULONG_PTR>(bbIdx * _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
		_cmdList->OMSetRenderTargets(1, &rtvH, false, nullptr);

		// 画面クリア
		float clearColor[] = { 0.0f, 0.0f, 1.0f, 1.0f };
		_cmdList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);

		// ポリゴン描画テスト
		_cmdList->RSSetViewports(1, &viewport);
		_cmdList->RSSetScissorRects(1, &scissorrect);
		_cmdList->SetGraphicsRootSignature(rootsignature);

		_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		_cmdList->IASetVertexBuffers(0, 1, &vbView);
		_cmdList->IASetIndexBuffer(&ibView);

		_cmdList->DrawIndexedInstanced(6, 1, 0, 0, 0);

		BarrierDesc.Transition.StateBefore	= D3D12_RESOURCE_STATE_RENDER_TARGET;
		BarrierDesc.Transition.StateAfter	= D3D12_RESOURCE_STATE_PRESENT;
		_cmdList->ResourceBarrier(1, &BarrierDesc);

		// 命令のクローズ
		hr = _cmdList->Close();
		if (FAILED(hr)) {
			assert(0 && "error create CommandList->Close().");
			return -1;
		}

		//コマンドリストの実行
		ID3D12CommandList* cmdlists[] = { _cmdList };
		_cmdQueue->ExecuteCommandLists(1, cmdlists);

		// 待ち
		_cmdQueue->Signal(_fence, ++_fenceVal);

		if (_fence->GetCompletedValue() != _fenceVal)
		{
			auto event = CreateEvent(nullptr, false, false, nullptr);
			_fence->SetEventOnCompletion(_fenceVal, event);
			if (event != 0)
			{
				WaitForSingleObject(event, INFINITE);
				CloseHandle(event);
			}
		}

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
