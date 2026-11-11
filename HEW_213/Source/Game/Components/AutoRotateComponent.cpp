#include "AutoRotateComponent.h"
#include "Engine/Scene/GameObject.h"

#ifdef _DEBUG
#include "imgui.h"

using namespace ImGui;
#endif

AutoRotateComponent::AutoRotateComponent(
	GameObject* pOwner, float speedX, float speedY, float speedZ)
	: Component(pOwner)
	, m_rotationSpeed{ speedX, speedY, speedZ }
{

}

void AutoRotateComponent::Update(float deltaTime)
{
	// 所属するGameObjectのTransformを取得
	Transform& transform = m_pOwner->GetTransform();

	// ｘ軸の回転
	transform.rotation[0] += m_rotationSpeed[0] * deltaTime;

	// ｙ軸の回転
	transform.rotation[1] += m_rotationSpeed[1] * deltaTime;

	// ｚ軸の回転
	transform.rotation[2] += m_rotationSpeed[2] * deltaTime;

}

std::unique_ptr<Component> AutoRotateComponent::Clone(
	GameObject* pNewOwner) const
{
	// 同じ回転速度を持つコンポーネントを作成
	std::unique_ptr<Component> pCopy(
		new AutoRotateComponent(
			pNewOwner,
			m_rotationSpeed[0],
			m_rotationSpeed[1],
			m_rotationSpeed[2]
		)
	);

	// 有効・無効の状態もコピー
	pCopy->SetEnabled(IsEnabled());

	return pCopy;
}

#ifdef _DEBUG
void AutoRotateComponent::DrawDebugInspector()
{
	// 回転速度を直接編集
	DragFloat3(u8"回転速度", m_rotationSpeed, 0.1f);

	// 現在の値を表示
	Text(
		"x: %.1f  y: %.1f  z: %.1f",
		m_rotationSpeed[0],
		m_rotationSpeed[1],
		m_rotationSpeed[2]
	);

	TextDisabled(u8"度 / 秒");
}
#endif
