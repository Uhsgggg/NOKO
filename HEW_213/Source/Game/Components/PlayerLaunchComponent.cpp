#include "PlayerLaunchComponent.h"
#include "Engine/Scene/GameObject.h"
#include <cmath>
#ifdef _DEBUG
#include "imgui.h"
#endif

PlayerLaunchComponent::PlayerLaunchComponent(GameObject* pOwner) : Component(pOwner)
{
}

void PlayerLaunchComponent::Configure(float minDrag, float maxDrag, float maxSpeed)
{
	if (!std::isfinite(minDrag) ||
		!std::isfinite(maxDrag) ||
		!std::isfinite(maxSpeed)) return;
	m_minDrag  = minDrag;
	m_maxDrag  = maxDrag;
	m_maxSpeed = maxSpeed;
}

// 引っ張った方向の逆へ飛ばす。距離に応じて速さを決める
bool PlayerLaunchComponent::TryBuildVelocity(const AimState& aim, BoardVec2& out) const
{
	out = {};

	if (!aim.released) return false;

	float x = aim.anchor.x - aim.point.x;
	float z = aim.anchor.z - aim.point.z;
	float length = std::sqrt(x * x + z * z);
	if (!std::isfinite(length) ||
		length <= 0 ||
		length < m_minDrag) return false;

	float speed = length / m_maxDrag * m_maxSpeed;
	if (speed > m_maxSpeed)
		speed = m_maxSpeed;
	out.x = x / length * speed;
	out.z = z / length * speed;

	return true;
}

float PlayerLaunchComponent::GetMinDrag() const
{
	return m_minDrag;
}

float PlayerLaunchComponent::GetMaxDrag() const
{
	return m_maxDrag;
}

float PlayerLaunchComponent::GetMaxSpeed() const
{
	return m_maxSpeed;
}

std::unique_ptr<Component> PlayerLaunchComponent::Clone(GameObject* pNewOwner) const
{
	auto pCopy = std::make_unique<PlayerLaunchComponent>(pNewOwner);
	pCopy->Configure(m_minDrag, m_maxDrag, m_maxSpeed);
	pCopy->SetEnabled(IsEnabled());
	return pCopy;
}

#ifdef _DEBUG
void PlayerLaunchComponent::DrawDebugInspector()
{
	float minD = m_minDrag;
	float maxD = m_maxDrag;
	float maxS = m_maxSpeed;
	ImGui::DragFloat("最小ドラッグ距離",&minD, 0.05f, 0,     10);
	ImGui::DragFloat("最大ドラッグ距離",&maxD, 0.1f,  0.01f, 20);
	ImGui::DragFloat("最大発射速度", &maxS, 0.1f, 0.01f, 100);
	Configure(minD, maxD, maxS);
}
#endif