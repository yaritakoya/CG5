#pragma once

#include <d3d12.h>
#include <string>

class Shader {
public:
	// シェーダーファイルを読み込み、コンパイル済みデータを生成する
	void Load(const std::wstring& filePath, const std::string& shaderModel);

	// 生成したコンパイル済みデータを取得する
	ID3DBlob* GetBlob();

	// コンストラクト
	Shader();
	// デストラクト
	~Shader();

private:
	// コンストラクタで初期化しなくていい
	ID3DBlob* blob_ = nullptr;
};
