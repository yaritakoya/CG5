#include "Shader.h"
#include <d3dcompiler.h>
#include <cassert>

void Shader::Load(const std::wstring& filePath, const std::string& shaderModel) {

	ID3DBlob* shaderBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;

	HRESULT hr = D3DCompileFromFile(
	    filePath.c_str(), // シェーダーファイルのパス
	    nullptr,
	    D3D_COMPILE_STANDARD_FILE_INCLUDE,               // インクルード可能にする
	    "main", shaderModel.c_str(),                     // エントリーポイント関数名、シェーダーモデル
	    D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, // デバッグ用のフラグ
	    0, &shaderBlob, &errorBlob);

	//エラーが発生した場合、止める
	if (FAILED(hr)) {
		if (errorBlob) {
			OutputDebugStringA(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
			errorBlob->Release();
		}
		assert(false);
	}
	// 生成したコンパイル済みシェーダーオブジェクトをメンバ変数に保存する
	blob_ = shaderBlob;
}

ID3DBlob* Shader::GetBlob() { return blob_; }

Shader::Shader() {}

Shader::~Shader() {
	if (blob_ != nullptr) {
		blob_->Release();
		blob_ = nullptr;
	}
}
