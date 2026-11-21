#include "ColliderComponent.h"
#include"Engine/Scene/GameObject.h"
#include<cmath>
#ifdef _DEBUG
#include"imgui.h"
#endif

ColliderComponent::ColliderComponent(GameObject *pOwner)
	: Component(pOwner)
{
}

void ColliderComponent::Configure(const ColliderConfig& c)
{
	//不正な設定なら変更しない
	if (static_cast<unsigned>(c.role) > 3 || static_cast<unsigned>(c.shape) > 1)
	   return;
	if (!std::isfinite(c.radius) || !std::isfinite(c.halfExtent.x) ||
		!std::isfinite(c.halfExtent.z) || !std::isfinite(c.restitution))
		return;
	if (c.radius <= 0 || c.halfExtent.x <= 0 || c.halfExtent.z <= 0)
		return;
	if (c.restitution < 0 || c.restitution > 1)
		return;
	m_config = c;
	
}

const ColliderConfig& ColliderComponent::GetConfig() const
{
	// TODO: return ステートメントをここに挿入します
	return m_config;
}

std::unique_ptr<Component> ColliderComponent::Clone(GameObject *pNewOwner) const
{
	auto pCopy = std::make_unique<ColliderComponent>(pNewOwner);
	pCopy->Configure(m_config);
	pCopy->SetEnabled(IsEnabled());
	return pCopy;
}

#ifdef DEBUG
void ColliderComponent::DrawDebugInspector()
{
	ColliderConfig c = m_config;
	int role = int(c.role);
	int shape = int(c.shape);
	ImGui::Combo("Role", &role, "Player\0Enemy\0Boss\0Wall\0");
	ImGui::Combo("Shape", &shape, "Sphere\0Box\0");
	c.role = GameRole(role);
	c.shape = ColliderShape(shape);
	ImGui::DragFloat("Radius", &c.radius, 0.05f, 0.01f, 100);
	ImGui::DragFloat("Half Extent X", &c.halfExtent.x, 0.05f, 0.01f, 100);
	ImGui::DragFloat("Half Extent Z", &c.halfExtent.z, 0.05f, 0.01f, 100);
	ImGui::DragFloat("Restitution", &c.restitution, 0.01f, 0, 1.0f);
	Configure(c);
}
#endif // DEBUG
