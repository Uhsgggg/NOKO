#include <DirectXMath.h>
#include "Camera.h"

using namespace DirectX;

// コンストラクタ
Camera::Camera()
	:m_pos{ 0.0f, 3.5f, -5.0f }
	,m_look{ 0.0f, 0.0f, 0.0f }
	,m_up{ 0.0f, 1.0f, 0.0f }
	,m_fovY(XMConvertToRadians(60.0f))
	,m_aspect(16.0f / 9.0f)
#ifdef _DEBUG
	,m_near(CMETER(0.1f))
	,m_farZ(METER(100.0f))
#else
	, m_near(CMETER(0.1f))
	, m_farZ(METER(25.0f))
#endif
{

}

// デストラクタ
Camera::~Camera()
{

}

// ビュー変換行列の取得(デフォルトでは転置済みの行列を計算)
XMFLOAT4X4 Camera::GetViewMatrix(bool transpose)
{
	XMVECTOR pos = XMLoadFloat3(&m_pos);
	XMVECTOR look = XMLoadFloat3(&m_look);
	XMVECTOR up = XMLoadFloat3(&m_up);

	// ビュー変換行列の作成
	XMMATRIX view;
	view = XMMatrixLookAtLH(pos, look, up);
	if (transpose)
	{
		view = XMMatrixTranspose(view);
	}

	// 読み取りデータに変換
	XMFLOAT4X4 mat;
	XMStoreFloat4x4(&mat, view);

	// 計算結果取得
	return mat;
}

// プロジェクション変換行列の取得(デフォルトでは転置済みの行列を計算)
XMFLOAT4X4 Camera::GetProjectionMatrix(bool transpose)
{
	// プロジェクション変換行列の作成
	XMMATRIX proj;
	proj = XMMatrixPerspectiveFovLH(m_fovY, m_aspect, m_near, m_farZ);
	if (transpose)
	{
		proj = XMMatrixTranspose(proj);
	}

	// 読み取りデータに変換
	XMFLOAT4X4 mat;
	XMStoreFloat4x4(&mat, proj);

	// 計算結果取得
	return mat;
}

// 座標の取得
XMFLOAT3 Camera::GetPos()
{
	return m_pos;
}

// 注視点の取得
XMFLOAT3 Camera::GetLook()
{
	return m_look;
}

void Camera::SetAspectRatio(float aspect)
{
	if (aspect > 0.0f)
	{
		m_aspect = aspect;
	}
}
