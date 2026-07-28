#include "IndexBuffer.h"
#include "KamataEngine.h"
#include <cassert>
#include <d3d12.h>

using namespace KamataEngine;

void IndexBuffer::Create(const UINT size, const UINT stride) {
	// strideの値によって、一つのインデックスのフォーマットを決める
	assert(stride == 2 || stride == 4);
	DXGI_FORMAT format = (stride == 2) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;

	// クラス内でdxCommonを利用するために追加
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	// インデックスリソースの生成-----
	// インデックスリソース用のヒープの設定
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD; // GPUへの転送用
	// インデックスリソースの設定
	D3D12_RESOURCE_DESC indexResourceDesc{};
	indexResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER; // バッファリソース
	indexResourceDesc.Width = size;                                // インデックスバッファのサイズ
	//バッファの場合はこれらは1にする決まり
	indexResourceDesc.Height = 1; // バッファの高さは1
	indexResourceDesc.DepthOrArraySize = 1; // バッファの奥行きは1
	indexResourceDesc.MipLevels = 1;        // ミップマップレベルは1
	indexResourceDesc.SampleDesc.Count = 1; // マルチサンプリングはしないので1
	//バッファの場合はこれにする決まり
	indexResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR; // バッファのレイアウトは行優先

	// インデックスリソースの生成
	ID3D12Resource* indexResource = nullptr;
	[[maybe_unused]] HRESULT hr = dxCommon->GetDevice()->CreateCommittedResource(
	    &uploadHeapProperties,             // ヒープ設定
	    D3D12_HEAP_FLAG_NONE,              // ヒープフラグ
	    &indexResourceDesc,                // リソース設定
	    D3D12_RESOURCE_STATE_GENERIC_READ, // リソースの使用状態
	    nullptr,                           // クリア用設定。バッファなので不要
	    IID_PPV_ARGS(&indexResource));     // 生成するリソースポインタのアドレス
	assert(SUCCEEDED(hr));

	// 生成したインデックスリソースをメンバ変数に格納
	indexBuffer_ = indexResource;

	// インデックスバッファビューの作成-----
	D3D12_INDEX_BUFFER_VIEW indexBufferView{};
	// インデックスリソースの先頭アドレスから使う
	indexBufferView.BufferLocation = indexResource->GetGPUVirtualAddress();
	//使用するインデックスリソースの全サイズ
	indexBufferView.SizeInBytes = size;
	// インデックスのフォーマット
	indexBufferView.Format = format;

	// 作成したインデックスバッファビューをメンバ変数に格納
	indexBufferView_ = indexBufferView;
}

ID3D12Resource* IndexBuffer::Get() { return indexBuffer_; }

D3D12_INDEX_BUFFER_VIEW* IndexBuffer::GetView() { return &indexBufferView_; }

IndexBuffer::IndexBuffer() {}

IndexBuffer::~IndexBuffer() {
	if (indexBuffer_) {
		indexBuffer_ -> Release();
		indexBuffer_ = nullptr;
	}
}

