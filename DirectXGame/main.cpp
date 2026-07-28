#include "KamataEngine.h"
#include "Shader.h"
#include <Windows.h>
// #include <d3dcompiler.h>
#include "IndexBuffer.h"
#include "PipelineState.h"
#include "RootSignature.h"
#include "VertexBuffer.h"
#include <cassert>

using namespace KamataEngine;

// 前方宣言
//  PipelineStateObjectを作成する関数
void SetupPipelineState(PipelineState& pipelineState, RootSignature& rs, Shader& vs, Shader& ps);
// RenderTargetResourceを作成する関数
ID3D12Resource* CreateRenderTextureResource(ID3D12Device* device, int32_t width, int32_t height, DXGI_FORMAT format, const FLOAT* clearColor);
// DepthStencilTextureResourceを作成する関数
ID3D12Resource* CreateDepthStencilTextureResource(ID3D12Device* device, int32_t width, int32_t height);

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	// エンジンの初期化
	KamataEngine::Initialize(L"LE3D_25_ヤリタ_コウヤ_CG5");

	// DirectXCommonインスタンスの取得
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	// DirectXCommonクラスが管理している、ウィンドウの幅と高さの値の取得
	int32_t w = dxCommon->GetBackBufferWidth();
	int32_t h = dxCommon->GetBackBufferHeight();
	DebugText::GetInstance()->ConsolePrintf(std::format("width:{},height:{}\n", w, h).c_str());

	// DirectXCommonクラスが管理している、コマンドリストの取得
	ID3D12GraphicsCommandList* commandList = dxCommon->GetCommandList();

	// RootSignatureの作成------
	RootSignature rs;
	rs.Create();

	// 頂点シェーダーの読み込みとコンパイル
	Shader vs;
	vs.LoadDxc(L"Resources/Shaders/TestVS.hlsl", L"vs_6_0");
	assert(vs.GetDxcBlob() != nullptr);

	// ピクセルシェーダーの読み込みとコンパイル
	Shader ps;
	ps.LoadDxc(L"Resources/Shaders/TestPS.hlsl", L"ps_6_0");
	assert(ps.GetDxcBlob() != nullptr);

	// PSO(Pipeline State Object)の作成------
	PipelineState pipelineState;
	SetupPipelineState(pipelineState, rs, vs, ps);

	struct VertexData {
		Vector4 position; // xyz座標
		Vector2 texcoord; // uv座標
	};

	VertexData vertices[] = {
	    // x, y, z, w,				//u, v
	    {{-0.5f, 0.5f, 0.0f, 1.0f},  {0.0f, 0.0f}}, // 0 左上
	    {{0.5f, 0.5f, 0.0f, 1.0f},   {1.0f, 0.0f}}, // 1 右上
	    {{-0.5f, -0.5f, 0.0f, 1.0f}, {0.0f, 1.0f}}, // 2 左下
	    {{0.5f, -0.5f, 0.0f, 1.0f},  {1.0f, 1.0f}}  // 3 右下
	};

	// vertexResourceの作成------
	VertexBuffer vb;
	vb.Create(sizeof(vertices), sizeof(vertices[0]));

	uint16_t indices[] = {0, 1, 2, 2, 1, 3};

	// indexBufferの作成------
	IndexBuffer ib;
	ib.Create(sizeof(indices), sizeof(indices[0]));

	// 頂点インデックスリソースにデータを書き込む
	uint16_t* pGpuIndices = nullptr;
	ib.Get()->Map(0, nullptr, reinterpret_cast<void**>(&pGpuIndices)); // インデックスリソースをCPUから書き込めるようにマップする

	for (int i = 0; i < _countof(indices); ++i) {
		pGpuIndices[i] = indices[i];
	}

	// Resource生成、Heap生成、View生成で利用される変数の準備
	ID3D12Device* device = dxCommon->GetDevice();
	HRESULT hr;

	// RenderTargetResourceの作成------
	// 0. RenderTextureResourceの作成
	// クリアカラーの設定
	const FLOAT kRenderTextureClearColor[] = {1.0f, 0.0f, 0.0f, 1.0f}; // 赤

	ID3D12Resource* renderTextureResource = CreateRenderTextureResource(device, WinApp::kWindowWidth, WinApp::kWindowHeight, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, kRenderTextureClearColor);

	// 1.RTV用のDescriptorHeapを作成
	ID3D12DescriptorHeap* rtvDescriptorHeap = nullptr;

	D3D12_DESCRIPTOR_HEAP_DESC rtvDescriptorHeapDesc = {};
	rtvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; // RTV用
	rtvDescriptorHeapDesc.NumDescriptors = 1;                    // ディスクリプタの数

	hr = device->CreateDescriptorHeap(&rtvDescriptorHeapDesc, IID_PPV_ARGS(&rtvDescriptorHeap));
	assert(SUCCEEDED(hr));

	// CPU側から見たHANDLEを取得
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandleCPU = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	// 2.RTV用のViewの生成

	device->CreateRenderTargetView(renderTextureResource, nullptr, rtvHandleCPU);

	// depthStencilTextureResourceの作成------
	ID3D12Resource* depthStencilResource = CreateDepthStencilTextureResource(device, WinApp::kWindowWidth, WinApp::kWindowHeight);

	// 1.DSV用のDescriptorHeapを作成
	ID3D12DescriptorHeap* dsvDescriptorHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC dsvDescriptorHeapDesc = {};
	dsvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvDescriptorHeapDesc.NumDescriptors = 1;
	dsvDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	hr = device->CreateDescriptorHeap(&dsvDescriptorHeapDesc, IID_PPV_ARGS(&dsvDescriptorHeap));
	assert(SUCCEEDED(hr));

	// CPU側から見たHANDLEを取得
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandleCPU = dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	// 2.DSV用のViewの生成
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

	// DSVHeapの先頭にDSVの生成
	device->CreateDepthStencilView(depthStencilResource, &dsvDesc, dsvHandleCPU);

	//SRVを準備する
	// 1.SRV用のDescriptorHeapを作成
	ID3D12DescriptorHeap* srvDescriptorHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC srvDescriptorHeapDesc = {};
	srvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV; // SRV用
	srvDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE; // シェーダから見えるようにする
	srvDescriptorHeapDesc.NumDescriptors = 1;                            // ディスクリプタの数
	
	hr = device->CreateDescriptorHeap(&srvDescriptorHeapDesc, IID_PPV_ARGS(&srvDescriptorHeap));
	assert(SUCCEEDED(hr));

	// CPU側から見たHANDLEを取得
	D3D12_CPU_DESCRIPTOR_HANDLE srvHandleCPU = srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU = srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();

	// 2.SRV用のViewの生成
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; // 画像のフォーマット
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING; // 4成分のマッピングを標準設定
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; // 2Dテクスチャ
	srvDesc.Texture2D.MipLevels = 1;                                            // ミップマップの最大レベル数

	device->CreateShaderResourceView(renderTextureResource, &srvDesc, srvHandleCPU);

	// 頂点リソースにデータを書き込む------
	VertexData* pGpuVertices = nullptr;
	vb.Get()->Map(0, nullptr, reinterpret_cast<void**>(&pGpuVertices)); // 頂点リソースをCPUから書き込めるようにマップする
	for (int i = 0; i < _countof(vertices); ++i) {
		pGpuVertices[i] = vertices[i];
	}

	// メインループ
	while (true) {
		// エンジンの更新
		if (KamataEngine::Update()) {
			break;
		}

		// 描画開始
		//dxCommon->PreDraw();

		// TransitioonBarrierをSRV->RTVに設定する
		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION; // リソースの状態遷移
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;                            // 特殊なフラグはなし
		barrier.Transition.pResource = renderTextureResource;  // 遷移するリソース
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE; // 遷移前の状態
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;          // 遷移後の状態
		commandList->ResourceBarrier(1, &barrier);

		// 描画先のRTVとDSVを設定する
		commandList->OMSetRenderTargets(1, &rtvHandleCPU, false, &dsvHandleCPU);

		// Viewportの設定
		D3D12_VIEWPORT viewport = {};
		viewport.Width = static_cast<float>(WinApp::kWindowWidth); // 幅
		viewport.Height = static_cast<float>(WinApp::kWindowHeight); // 高さ
		viewport.TopLeftX = 0;                                       // 左上X座標
		viewport.TopLeftY = 0;                                       // 左上Y座標
		viewport.MinDepth = 0.0f;                                    // 深度の最小値
		viewport.MaxDepth = 1.0f;                                    // 深度の最大値

		commandList->RSSetViewports(1, &viewport);

		// Scissorの設定
		D3D12_RECT scissorRect = {};
		scissorRect.left = 0;
		scissorRect.top = 0;
		scissorRect.right = WinApp::kWindowWidth;
		scissorRect.bottom = WinApp::kWindowHeight;
		
		commandList->RSSetScissorRects(1, &scissorRect);

		//全画面クリア
		commandList->ClearRenderTargetView(rtvHandleCPU, kRenderTextureClearColor, 0, nullptr);
		//指定した深度で画面全体をクリアする
		commandList->ClearDepthStencilView(dsvHandleCPU, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

		//描画(後で)

		// TransitionBarrierを元に戻し、pixelShaderResourceに戻す
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION; // リソースの状態遷移
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;      // 特殊なフラグはなし
		barrier.Transition.pResource = renderTextureResource;  // 遷移するリソース
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET; // 遷移前の状態
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE; // 遷移後の状態
		commandList->ResourceBarrier(1, &barrier);

		dxCommon->PreDraw();

		// コマンドを積む
		commandList->SetGraphicsRootSignature(rs.Get());     // ルートシグネチャの設定
		commandList->SetPipelineState(pipelineState.Get());  // PSOの設定
		commandList->IASetVertexBuffers(0, 1, vb.GetView()); // 頂点バッファビューの設定
		commandList->IASetIndexBuffer(ib.GetView());         // インデックスバッファビューの設定

		//使用するディスクリプタの設定
		commandList->SetDescriptorHeaps(1, &srvDescriptorHeap);

		// SRVのdescriptorTableの先頭を設定
		commandList->SetGraphicsRootDescriptorTable(0, srvHandleGPU);

		// トポロジの設定。今回は三角形リスト
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		// 描画コマンド。頂点数3、インスタンス数1、スタート頂点0、スタートインスタンス0
		commandList->DrawIndexedInstanced(_countof(indices), 1, 0, 0, 0);

		// 描画終了
		dxCommon->PostDraw();
	}

	//解放
	renderTextureResource->Release();
	srvDescriptorHeap->Release();
	rtvDescriptorHeap->Release();
	depthStencilResource->Release();
	dsvDescriptorHeap->Release();

	// エンジンの終了処理
	KamataEngine::Finalize();

	return 0;
}

void SetupPipelineState(PipelineState& pipelineState, RootSignature& rs, Shader& vs, Shader& ps) {
	// inputLayoutの作成------
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[2] = {};
	inputElementDescs[0].SemanticName = "POSITION";                        // 頂点の座標
	inputElementDescs[0].SemanticIndex = 0;                                // 同じセマンティクスが複数ある場合の識別番号
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;          // データの形式(RGBA)
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT; // 頂点データ内のオフセット値。前の要素の次から自動で算出する

	inputElementDescs[1].SemanticName = "TEXCOORD";                        // 頂点のUV座標
	inputElementDescs[1].SemanticIndex = 0;                                // 同じセマンティクスが複数ある場合の識別番号
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;                // データの形式(UV)
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT; // 頂点データ内のオフセット値。前の要素の次から自動で算出する

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc = {};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;    // 入力要素の配列
	inputLayoutDesc.NumElements = _countof(inputElementDescs); // 入力要素の数

	// blendStateの作成------
	D3D12_BLEND_DESC blendDesc = {};
	// 全ての色色素を書き込む
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	// rasterizerStateの作成------
	D3D12_RASTERIZER_DESC rasterizerDesc = {};
	// 裏面(反時計回り)をカリングする
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	// 塗りつぶしモードをソリッドにする(ワイヤーフレームならD3D12_FILL_MODE_WIREFRAME)
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	// PSO(Pipeline State Object)の作成------
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc = {};
	graphicsPipelineStateDesc.pRootSignature = rs.Get();                                                    // ルートシグネチャ
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;                                                // 頂点レイアウト
	graphicsPipelineStateDesc.VS = {vs.GetDxcBlob()->GetBufferPointer(), vs.GetDxcBlob()->GetBufferSize()}; // 頂点シェーダー
	graphicsPipelineStateDesc.PS = {ps.GetDxcBlob()->GetBufferPointer(), ps.GetDxcBlob()->GetBufferSize()}; // ピクセルシェーダー
	graphicsPipelineStateDesc.BlendState = blendDesc;                                                       // ブレンドステート
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;

	// 書き込むRTVの情報
	graphicsPipelineStateDesc.NumRenderTargets = 1;                            // 書き込むRTVの数
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; // RTVのフォーマット
	// 利用するトポロジ(形状)のタイプ。今回は三角形リスト(3点で1つの三角形を構成)を指定
	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	// どのように画面に色を打ち込むかの設定
	graphicsPipelineStateDesc.SampleDesc.Count = 1;                   // マルチサンプリングのサンプル数。今回はマルチサンプリングをしないので1
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK; // 標準のサンプルマスク。全てのサンプルを有効にする

	// PSOの生成
	pipelineState.Create(graphicsPipelineStateDesc);
}

// RenderTargetResourceを作成する関数
ID3D12Resource* CreateRenderTextureResource(ID3D12Device* device, int32_t width, int32_t height, DXGI_FORMAT clearFormat, const FLOAT* clearColor) {
	// 1.リソース設定
	D3D12_RESOURCE_DESC resourceDesc = {};
	resourceDesc.Width = UINT(width);
	resourceDesc.Height = UINT(height);
	resourceDesc.MipLevels = 1;        // ミップマップレベル数
	resourceDesc.DepthOrArraySize = 1; // 2Dテクスチャなので1
	resourceDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D; //
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	// 2.ヒープ設定
	D3D12_HEAP_PROPERTIES heapProperties = {};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT; // デフォルトヒープ

	// 3.クリアカラー設定
	D3D12_CLEAR_VALUE clearValue = {};
	clearValue.Format = clearFormat;
	clearValue.Color[0] = clearColor[0];
	clearValue.Color[1] = clearColor[1];
	clearValue.Color[2] = clearColor[2];
	clearValue.Color[3] = clearColor[3];

	// 4.リソース生成
	ID3D12Resource* resource = nullptr;
	[[maybe_unused]] HRESULT hr =
	    device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, &clearValue, IID_PPV_ARGS(&resource));
	assert(SUCCEEDED(hr));

	return resource;
}

// DepthStencilTextureの生成
ID3D12Resource* CreateDepthStencilTextureResource(ID3D12Device* device, int32_t width, int32_t height) {

	// 1. 生成するDepthStencilTextureのDescの設定
	D3D12_RESOURCE_DESC resourceDesc{};

	resourceDesc.Width = width;                                   // Textureの幅
	resourceDesc.Height = height;                                 // Textureの高さ
	resourceDesc.MipLevels = 1;                                   // mipmapの数 DepthStencilなので1つでいい
	resourceDesc.DepthOrArraySize = 1;                            // Textureの配列数 DepthStencilは1つでいい
	resourceDesc.Format = DXGI_FORMAT_D32_FLOAT;                  // DepthStencilとして利用可能なフォーマット
	                                                              // ※ KamataEngineと合わせる
	resourceDesc.SampleDesc.Count = 1;                            // サンプリングカウント 1固定
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;  // 2次元
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL; // DepthStencilとして使う通知

	// 2. ヒープ設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT; // デフォルトヒープ

	// クリア値の設定
	D3D12_CLEAR_VALUE depthClearValue{};
	depthClearValue.DepthStencil.Depth = 1.0f;      // 深度の初期値
	depthClearValue.Format = DXGI_FORMAT_D32_FLOAT; // ステンシルの初期値

	// 3. DepthStencilTextureResourceの生成
	ID3D12Resource* resource = nullptr;

	[[maybe_unused]] HRESULT hr = device->CreateCommittedResource(
	    &heapProperties,                  // Heapの設定
	    D3D12_HEAP_FLAG_NONE,             // Heapの特殊な設定 ★後で変更？
	    &resourceDesc,                    // Resourceの設定
	    D3D12_RESOURCE_STATE_DEPTH_WRITE, // 深度値を書き込み状態にしておく
	    &depthClearValue,                 // Clear設定値
	    IID_PPV_ARGS(&resource)           // 作成するResourceポインタへのポインタ
	);

	assert(SUCCEEDED(hr));

	return resource;
}