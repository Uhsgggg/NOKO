#include "HealthComponent.h"
#include "Engine/Scene/GameObject.h"
#ifdef _DEBUG
#include "imgui.h"
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
	if (amount <= 0 || m_hp == 0) return 0;

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
	int hp = m_maxHP;
	if (ImGui::DragInt("Max HP", &hp, 1, 1, 100000) && hp > 0)
		Reset(hp);
	ImGui::Text("HP: &d / %d", m_hp, m_maxHP);
}
#endif // _DEBUG
