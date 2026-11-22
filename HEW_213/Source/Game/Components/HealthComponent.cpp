#include "HealthComponent.h"
#include "Engine/Scene/GameObject.h"
#ifdef _DEBUG
#include "imgui.h"
using namespace ImGui;
#endif // _DEBUG


HealthComponent::HealthComponent(GameObject* pOwner) : Component(pOwner)
{

}
void HealthComponent::Reset(int maxHP)
{
	if (maxHP < 1)
	{
		maxHP = 1;
	}
	m_maxHP = maxHP;
	m_hp = maxHP;
}
int HealthComponent::ApplyDamage(int amount)
{
	// 与えるダメージが0以下、または現在のHPが0だったら
	// 与えるダメージを0として返す
	if (amount <= 0 || m_hp == 0)
	{
		return 0;
	}
	// 与えるダメージが現在のHPより大きかったら
	// 現在のHPと同じ値を返す。
	if (amount > m_hp)
	{
		amount = m_hp;
	}
	m_hp -= amount;
	return amount;
}
int HealthComponent::GetHP() const
{
	return m_hp;
}
int HealthComponent::GetMaxHP() const
{
	return m_maxHP;
}
bool HealthComponent::IsDead() const
{
	return m_hp == 0;
}
std::unique_ptr<Component>HealthComponent::Clone(GameObject* pNewOwner) const
{
	auto pCopy = std::make_unique<HealthComponent>(pNewOwner);
	pCopy->Reset(m_maxHP);
	pCopy->SetEnabled(IsEnabled());
	return pCopy;
}
#ifdef _DEBUG
void HealthComponent::DrawDebugInspector()
{
	int maxHP = m_maxHP;
	if (DragInt("最大HP", &maxHP, 1, 1, 100000) && maxHP > 0)
	{
		Reset(maxHP);
	}
	Text("HP: %d / %d", m_hp, m_maxHP);
}
#endif // _DEBUG
