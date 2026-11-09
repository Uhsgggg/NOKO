#pragma once

#include <DirectXMath.h>

// 3Dオブジェクトの変換情報
struct Transform
{
	// 座標
	float position[3] =
	{
		0.0f, 0.0f, 0.0f
	};

	// 回転角度（度数法）
	float rotation[3] =
	{
		0.0f, 0.0f, 0.0f
	};

	// 拡大率
	float scale[3] =
	{
		1.0f, 1.0f, 1.0f
	};

	// ワールド行列を計算
	// animationRadian はアニメーション用の追加回転角
	DirectX::XMMATRIX GetWorldMatrix(float animationRadian = 0.0f) const
	{
		using namespace DirectX;

		XMMATRIX S = XMMatrixScaling(
			scale[0], scale[1], scale[2]);
		XMMATRIX Rx = XMMatrixRotationX(
			XMConvertToRadians(rotation[0]) + animationRadian);
		XMMATRIX Ry = XMMatrixRotationY(
			XMConvertToRadians(rotation[1]) + animationRadian);
		XMMATRIX Rz = XMMatrixRotationZ(
			XMConvertToRadians(rotation[2]) + animationRadian);
		XMMATRIX T = XMMatrixTranslation(
			position[0], position[1], position[2]);

		return S * Rx * Ry * Rz * T;
	}
};