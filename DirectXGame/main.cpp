#include "KamataEngine.h"
#include "Shader.h"
#include <Windows.h>
//#include <d3dcompiler.h>
#include <cassert>

using namespace KamataEngine;

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
	// 構造体にデータを用意する
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature = {};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	ID3DBlob* signatureBlob = nullptr; // シリアライズされたルートシグネチャが入るバッファ
	ID3DBlob* errorBlob = nullptr;     // エラー情報が入るバッファ
	HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr)) {
		DebugText::GetInstance()->ConsolePrintf(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}

	// バイナリを元に生成
	ID3D12RootSignature* rootSignature = nullptr;
	hr = dxCommon->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(hr));

	// inputLayoutの作成------
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[1] = {};
	inputElementDescs[0].SemanticName = "POSITION";                        // 頂点の座標
	inputElementDescs[0].SemanticIndex = 0;                                // 同じセマンティクスが複数ある場合の識別番号
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;          // データの形式(RGBA)
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT; // 頂点データ内のオフセット値。前の要素の次から自動で算出する
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

	// コンパイル済みのshader、エラー時情報の格納場所の用意
	// ID3DBlob* vsBlob = nullptr; // 頂点シェーダーオブジェクト
	// ID3DBlob* psBlob = nullptr; // ピクセルシェーダーオブジェクト
	// ID3DBlob* errorBlob = nullptr; // エラー情報オブジェクト

	// 頂点シェーダーの読み込みとコンパイル
	Shader vs;
	vs.LoadDxc(L"Resources/Shaders/TestVS.hlsl", L"vs_6_0");
	assert(vs.GetDxcBlob() != nullptr);

#pragma region vs関数化前のコード

	// std::wstring vsFile = L"Resources/Shaders/TestVS.hlsl";
	// hr = D3DCompileFromFile(
	//     vsFile.c_str(), // シェーダーファイルのパス
	//     nullptr,
	//     D3D_COMPILE_STANDARD_FILE_INCLUDE,               // インクルード可能にする
	//     "main", "vs_5_0",                                // エントリーポイント関数名、シェーダーモデル
	//     D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, // デバッグ用のフラグ
	//     0, &vsBlob, &errorBlob);
	// if (FAILED(hr)) {
	//	DebugText::GetInstance()->ConsolePrintf(std::system_category().message(hr).c_str());
	//	if (errorBlob) {
	//		DebugText::GetInstance()->ConsolePrintf(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
	//	}

	//	OutputDebugStringA((char*)errorBlob->GetBufferPointer());

	//	assert(false);
	//}

#pragma endregion

	// ピクセルシェーダーの読み込みとコンパイル
	Shader ps;
	ps.LoadDxc(L"Resources/Shaders/TestPS.hlsl", L"ps_6_0");
	assert(ps.GetDxcBlob() != nullptr);

#pragma region ps関数化前のコード

	// std::wstring psFile = L"Resources/Shaders/TestPS.hlsl";
	//  hr = D3DCompileFromFile(
	//      psFile.c_str(), // シェーダーファイルのパス
	//      nullptr,
	//      D3D_COMPILE_STANDARD_FILE_INCLUDE,               // インクルード可能にする
	//      "main", "ps_5_0",                                // エントリーポイント関数名、シェーダーモデル
	//      D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, // デバッグ用のフラグ
	//      0, &psBlob, &errorBlob);
	//  if (FAILED(hr)) {
	//	DebugText::GetInstance()->ConsolePrintf(std::system_category().message(hr).c_str());
	//	if (errorBlob) {
	//		DebugText::GetInstance()->ConsolePrintf(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
	//	}
	//	assert(false);
	//  }

#pragma endregion

	// PSO(Pipeline State Object)の作成------
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc = {};
	graphicsPipelineStateDesc.pRootSignature = rootSignature;                             // ルートシグネチャ
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;                              // 頂点レイアウト
	graphicsPipelineStateDesc.VS = {vs.GetDxcBlob()->GetBufferPointer(), vs.GetDxcBlob()->GetBufferSize()}; // 頂点シェーダー
	graphicsPipelineStateDesc.PS = {ps.GetDxcBlob()->GetBufferPointer(), ps.GetDxcBlob()->GetBufferSize()}; // ピクセルシェーダー
	graphicsPipelineStateDesc.BlendState = blendDesc;                                     // ブレンドステート
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;                           // ラスタライザーステート
	// 書き込むRTVの情報
	graphicsPipelineStateDesc.NumRenderTargets = 1;                            // 書き込むRTVの数
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; // RTVのフォーマット
	// 利用するトポロジ(形状)のタイプ。今回は三角形リスト(3点で1つの三角形を構成)を指定
	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	// どのように画面に色を打ち込むかの設定
	graphicsPipelineStateDesc.SampleDesc.Count = 1;                   // マルチサンプリングのサンプル数。今回はマルチサンプリングをしないので1
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK; // 標準のサンプルマスク。全てのサンプルを有効にする
	// PSOの生成
	ID3D12PipelineState* graphicsPipelineState = nullptr;
	hr = dxCommon->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&graphicsPipelineState));
	assert(SUCCEEDED(hr));

	// vertexResourceの作成------
	// 頂点リソース用のヒープの設定
	D3D12_HEAP_PROPERTIES uploadHeapProperties = {};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD; // CPUから書き込むヒープ
	// 頂点リソースの設定
	D3D12_RESOURCE_DESC vertexResourceDesc = {};
	vertexResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER; // バッファリソース
	vertexResourceDesc.Width = sizeof(Vector4) * 3;                 // 頂点データのサイズ（例として3頂点分）
	// バッファの場合はこれらを1にする必要がある
	vertexResourceDesc.Height = 1;
	vertexResourceDesc.DepthOrArraySize = 1;
	vertexResourceDesc.MipLevels = 1;
	vertexResourceDesc.SampleDesc.Count = 1;
	// バッファの場合はこれにする決まり
	vertexResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	// 頂点リソースの生成
	ID3D12Resource* vertexResource = nullptr;
	hr = dxCommon->GetDevice()->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &vertexResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertexResource));
	assert(SUCCEEDED(hr)); // 上手くいかなかったときは起動できない

	// VertexBufferViewの作成------
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};
	// resourceの先頭アドレスから使う
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点３つ分のサイズ
	vertexBufferView.SizeInBytes = sizeof(Vector4) * 3;
	// 頂点の構造体のサイズ
	vertexBufferView.StrideInBytes = sizeof(Vector4);

	// 頂点リソースにデータを書き込む------
	Vector4* vertexData = nullptr;
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData)); // 頂点リソースをCPUから書き込めるようにマップする
	vertexData[0] = {-0.5f, -0.5f, 0.0f, 1.0f};                             // 左下
	vertexData[1] = {0.0f, 0.5f, 0.0f, 1.0f};                               // 上
	vertexData[2] = {0.5f, -0.5f, 0.0f, 1.0f};                              // 右下
	// 書き込んだらアンマップする
	vertexResource->Unmap(0, nullptr);

	// メインループ
	while (true) {
		// エンジンの更新
		if (KamataEngine::Update()) {
			break;
		}

		// 描画開始
		dxCommon->PreDraw();

		// コマンドを積む
		commandList->SetGraphicsRootSignature(rootSignature);     // ルートシグネチャの設定
		commandList->SetPipelineState(graphicsPipelineState);     // PSOの設定
		commandList->IASetVertexBuffers(0, 1, &vertexBufferView); // 頂点バッファビューの設定
		// トポロジの設定。今回は三角形リスト
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		// 描画コマンド。頂点数3、インスタンス数1、スタート頂点0、スタートインスタンス0
		commandList->DrawInstanced(3, 1, 0, 0);

		// 描画終了
		dxCommon->PostDraw();
	}

	// リソースの解放
	vertexResource->Release();
	graphicsPipelineState->Release();
	signatureBlob->Release();
	// if (errorBlob) {
	//	errorBlob->Release();
	// }
	rootSignature->Release();
	//vsBlob->Release();
	//psBlob->Release();

	// エンジンの終了処理
	KamataEngine::Finalize();

	return 0;
}
