#include "CameraPlay.h"

using namespace DirectX;
using namespace std;

CameraPlay::CameraPlay()
{
	// カメラの初期位置
	m_pos = XMFLOAT3(
		0.0f,
		8.0f,
		-10.0f
	);

	// カメラの注視点
	m_look = XMFLOAT3(
		0.0f,
		0.0f,
		0.0f
	);

	// 上方向
	m_up = XMFLOAT3(
		0.0f,
		1.0f,
		0.0f
	);
}

void CameraPlay::Update()
{
	// 固定カメラの予定なので更新処理はなし
}

void CameraPlay::SetPose(
	const XMFLOAT3& position,
	const XMFLOAT3& look)
{
	m_pos = position;
	m_look = look;
}


