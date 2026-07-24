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

void SetupPipelineState(PipelineState& pipelineState, RootSignature& rs, Shader& vs, Shader& ps);

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
		dxCommon->PreDraw();

		// コマンドを積む
		commandList->SetGraphicsRootSignature(rs.Get());     // ルートシグネチャの設定
		commandList->SetPipelineState(pipelineState.Get());  // PSOの設定
		commandList->IASetVertexBuffers(0, 1, vb.GetView()); // 頂点バッファビューの設定
		commandList->IASetIndexBuffer(ib.GetView());         // インデックスバッファビューの設定
		// トポロジの設定。今回は三角形リスト
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		// 描画コマンド。頂点数3、インスタンス数1、スタート頂点0、スタートインスタンス0
		commandList->DrawIndexedInstanced(_countof(indices), 1, 0, 0, 0);

		// 描画終了
		dxCommon->PostDraw();
	}

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
