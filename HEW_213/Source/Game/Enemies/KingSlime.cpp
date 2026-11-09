#include "KingSlime.h"

using namespace DirectX;

KingSlime::KingSlime(StaticFbxModel& model)
	: m_pModel(&model)
{
	m_hp = 200;
}

void KingSlime::Update(float deltaTime)
{

}

void KingSlime::Draw(
	const DirectX::XMFLOAT4X4& view,
	const DirectX::XMFLOAT4X4& projection
)
{
	XMMATRIX world =
		XMMatrixScaling(m_scale, m_scale, m_scale) *
		XMMatrixRotationY(m_rotationY) *
		XMMatrixTranslation(
			m_position.x,
			m_position.y,
			m_position.z
		);

	XMFLOAT4X4 worldMatrix;
	XMStoreFloat4x4(
		&worldMatrix,
		XMMatrixTranspose(world)
	);

	m_pModel->Draw(worldMatrix, view, projection);
}