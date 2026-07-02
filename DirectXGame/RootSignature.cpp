#include "RootSignature.h"
#include "KamataEngine.h"

using namespace KamataEngine;

// RootSignatureの生成
void RootSignature::Create() {
	// 既にインスタンスがあるなら開放する
	if (rootSignature_) {
		rootSignature_->Release();
		rootSignature_ = nullptr;
	}
	// クラス内で取得するために追加
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	// RootSignatureの作成------
	// 構造体にデータを用意する
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature = {};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	ID3DBlob* signatureBlob = nullptr; // シリアライズされたルートシグネチャが入るバッファ
	ID3DBlob* errorBlog = nullptr;     // エラー情報が入るバッファ
	HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlog);
	if (FAILED(hr)) {
		DebugText::GetInstance()->ConsolePrintf(reinterpret_cast<char*>(errorBlog->GetBufferPointer()));
		assert(false);
	}

	// バイナリを元に生成
	ID3D12RootSignature* rootSignature = nullptr;
	hr = dxCommon->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(hr));

	// signatureBlobは解放してもいい
	signatureBlob->Release();
	// 生成したRootSignatureをメンバ変数に保存
	rootSignature_ = rootSignature;
}

ID3D12RootSignature* RootSignature::Get() { return rootSignature_; }

RootSignature::RootSignature() {}

RootSignature::~RootSignature() {
	if (rootSignature_) {
		rootSignature_->Release();
		rootSignature_ = nullptr;
	}
}
