#include "MoveComponent.h"
#include "Engine/Scene/GameObject.h"

#ifdef _DEBUG
#include "imgui.h"
#endif

using namespace std;

//========================================
// コンストラクタ
//========================================
MoveComponent::MoveComponent(
	GameObject* pOwner,
	float x, float y, float z)
	: Component(pOwner)
	, m_velocity{ x, y, z }
{
}

//========================================
// 移動処理
//========================================
void MoveComponent::Update(float deltaTime)
{
	if (!m_pOwner) return;

	// 所有者のTransformを取得
	Transform& transform = m_pOwner->GetTransform();

	// 速度 × 経過時間だけ移動
	transform.position[0] += m_velocity[0] * deltaTime;
	transform.position[1] += m_velocity[1] * deltaTime;
	transform.position[2] += m_velocity[2] * deltaTime;
}

//========================================
// コンポーネントを複製
//========================================
unique_ptr<Component> MoveComponent::Clone(
	GameObject* pNewOwner) const
{
	unique_ptr<Component> pCopy(
		new MoveComponent(
			pNewOwner,
			m_velocity[0],
			m_velocity[1],
			m_velocity[2]
		)
	);

	// 有効・無効も引き継ぐ
	pCopy->SetEnabled(IsEnabled());

	return pCopy;
}

//========================================
// 移動速度を変更
//========================================
void MoveComponent::SetVelocity(float x, float y, float z)
{
	m_velocity[0] = x;
	m_velocity[1] = y;
	m_velocity[2] = z;
}

#ifdef _DEBUG
//========================================
// Inspectorの表示
//========================================
void MoveComponent::DrawDebugInspector()
{
	using namespace ImGui;
	DragFloat3(
		"Velocity",
		m_velocity,
		0.05f
	);

	TextDisabled("Units / second");
}
#endif


