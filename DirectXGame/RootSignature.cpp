#include "RootSignature.h"
#include "KamataEngine.h"
#include <cassert>

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

	// デスクリプタレンジ
	D3D12_DESCRIPTOR_RANGE srvDescRange[1] = {};
	// t0レジスタに利用可能にする
	srvDescRange[0].BaseShaderRegister = 0;                                                   // t0レジスタ
	srvDescRange[0].NumDescriptors = 1;                                                       // 1個
	srvDescRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;                              // SRV
	srvDescRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND; // デスクリプタテーブルの先頭からの相対オフセット

	// RootParameterの用意 ※PixelShaderに読ませるために必要 ★00_09追加
	// 複数設定できるので配列の構造をしている。今回は1つだけなので、長さ1の配列として用意する
	D3D12_ROOT_PARAMETER rootParameters[1]{};

	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;   // DescriptorTable
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;             // PixelShaderで使う
	rootParameters[0].DescriptorTable.pDescriptorRanges = srvDescRange;             // 拡張しやすくする
	rootParameters[0].DescriptorTable.NumDescriptorRanges = _countof(srvDescRange); // RangeTableを数

	descriptionRootSignature.pParameters = rootParameters; // ルートパラメータ配列へのポインタ
	descriptionRootSignature.NumParameters = _countof(rootParameters);

	// Samplerの設定　★00_09 追加
	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;         // バイリニアフィルタ
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;       // 0.0～1.0の範囲外をリピート
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;       // 0.0～1.0の範囲外をリピート
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;       // 0.0～1.0の範囲外をリピート
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;     // 比較しない
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;                       // ありったけのMipMapを使う
	staticSamplers[0].ShaderRegister = 0;                               // レジスタ番号 0 を使う(s0)
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderで使う

	descriptionRootSignature.pStaticSamplers = staticSamplers;
	descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);

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
