#pragma once

#include "Camera.h"

class CameraPlay : public Camera
{
public:
	CameraPlay();
	~CameraPlay() override = default;

	// 固定カメラの予定だから何もしない
	void Update() override;

	// カメラの位置と注視点を変更
	void SetPose(
		const DirectX::XMFLOAT3& position,
		const DirectX::XMFLOAT3& look
	);

	// カメラの位置を取得
	const DirectX::XMFLOAT3& GetPosition() const
	{
		return m_pos;
	}

	// 注視点を取得
	const DirectX::XMFLOAT3& GetLook() const
	{
		return m_look;
	}
};

