#include "BallMotionComponent.h"
#include "Engine/Scene/GameObject.h"
#include <cmath>
#ifdef _DEBUG
#include "imgui.h"
#endif

using namespace std;

//速度はXZ平面のみ。Clone時には設定のみコピーする
BallMotionComponent::BallMotionComponent(GameObject* pOwner) : Component(pOwner)
{
}

void BallMotionComponent::Configure(float deceleration, float stopSpeed, float maxSpeed)
{
	//設定が間違っていたら今の設定を残す
	if (!isfinite(deceleration) ||
		!isfinite(stopSpeed) ||
		!isfinite(maxSpeed))
		return;

	if (deceleration <= 0 ||
		stopSpeed < 0 ||
		maxSpeed <= stopSpeed)
		return;

	m_deceleration = deceleration;
	m_stopSpeed    = stopSpeed;
	m_maxSpeed     = maxSpeed;
	SetVelocity(m_velocity);
}

void BallMotionComponent::SetVelocity(BoardVec2 v)
{
	float speed = sqrt(v.x * v.x + v.z * v.z);
	if (!isfinite(speed)) return;
	if (speed <= m_stopSpeed)
	{
		Stop();
		return;
	}
	if (speed > m_maxSpeed)
	{
		v.x = v.x / speed * m_maxSpeed;
		v.z = v.z / speed * m_maxSpeed;
	}
	m_velocity = v;
}

BoardVec2 BallMotionComponent::GetVelocity() const
{
	return m_velocity;
}

bool BallMotionComponent::IsMoving() const
{
	return m_velocity.x != 0 || m_velocity.z != 0;
}

void BallMotionComponent::Stop()
{
	m_velocity = {};
}

float BallMotionComponent::GetDeceleration() const
{
	return m_deceleration;
}

float BallMotionComponent::GetStopSpeed() const
{
	return m_stopSpeed;
}

float BallMotionComponent::GetMaxSpeed() const
{
	return m_maxSpeed;
}

// 1.位置を決める → 2.速さだけを小さくする → 3.停止判定
void BallMotionComponent::Update(float dt)
{
	if (!m_pOwner || !IsMoving() ||
		!isfinite(dt) ||
		dt <= 0) return;

	Transform& t = m_pOwner->GetTransform();
	t.position[0] += m_velocity.x * dt;
	t.position[2] += m_velocity.z * dt;

	float Speed = sqrt(
		m_velocity.x * m_velocity.x + m_velocity.z * m_velocity.z);
	float nextSpeed = Speed - m_deceleration * dt;
	if (nextSpeed <= m_stopSpeed)
	{
		Stop();
		return;
	}
	//向きを変えずに速さだけ減らす
	m_velocity.x = m_velocity.x / Speed * nextSpeed;
	m_velocity.z = m_velocity.z / Speed * nextSpeed;
}

unique_ptr<Component> BallMotionComponent::Clone(GameObject* pNewOwner) const
{
	auto pCopy = make_unique<BallMotionComponent>(pNewOwner);
	pCopy->Configure(m_deceleration, m_stopSpeed, m_maxSpeed);
	pCopy->SetEnabled(IsEnabled());
	return pCopy;
}

#if _DEBUG
void BallMotionComponent::DrawDebugInspector()
{
	float d = m_deceleration;
	float s = m_stopSpeed;
	float m = m_maxSpeed;
	ImGui::DragFloat("Deceleration", &d,  0.1f, 0.01f, 100.0f);
	ImGui::DragFloat("Stop speed",   &s, 0.01f,  0.0f,  10.0f);
	ImGui::DragFloat("Max speed",    &m,  0.1f,  0.0f, 100.0f);
	Configure(d, s, m);
	ImGui::Text("Velocity: %.2f, %.2f", m_velocity.x, m_velocity.z);
}
#endif