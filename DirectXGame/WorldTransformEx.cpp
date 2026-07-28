#include "WorldTransformEx.h"

using namespace KamataEngine;
using namespace KamataEngine::MathUtility;

void WorldTransformEx::UpdateMatrix() {
	// world変換行列を計算し、matWorld_に代入する
	matWorld_ = MakeAffineMatrix();
	// 定数バッファに転送する
	TransferMatrix();
}

KamataEngine::Matrix4x4 WorldTransformEx::MakeAffineMatrix() { 
	//scaleMatrix
	Matrix4x4 matScale = MakeScaleMatrix(scale_);

	// rotationMatrix
	Matrix4x4 matRotX = MakeRotateXMatrix(rotation_.x);
	Matrix4x4 matRotY = MakeRotateYMatrix(rotation_.y);
	Matrix4x4 matRotZ = MakeRotateZMatrix(rotation_.z);
	Matrix4x4 matRot = matRotZ * matRotX * matRotY;

	// translationMatrix
	Matrix4x4 matTrans = MakeTranslateMatrix(translation_);

	// worldMatrix
	Matrix4x4 matWorld = matScale * matRot * matTrans;

	return matWorld;
}
