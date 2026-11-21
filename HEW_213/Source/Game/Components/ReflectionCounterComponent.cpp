#include "ReflectionCounterComponent.h"
#include "Engine/Scene/GameObject.h"
#ifdef DEBUG
#include"imgui.h"
#endif // DEBUG


ReflectionCounterComponent::ReflectionCounterComponent(GameObject *pOwner)
	: Component(pOwner)
{
}


void ReflectionCounterComponent::Reset()
{
	m_count = 0;
}

void ReflectionCounterComponent::Increment()
{
	if(m_count < INT_MAX)
	++m_count;
}

int ReflectionCounterComponent::GetCount() const
{
	return m_count;
}

std::unique_ptr<Component> ReflectionCounterComponent::Clone(GameObject *pNewOwner) const
{
	auto pCopy = std::make_unique<ReflectionCounterComponent>(pNewOwner);
	pCopy->SetEnabled(IsEnabled());
	return pCopy;
}

#ifdef DEBUG
void ReflectionCounterComponent::DrawDebugInspector()
{
	ImGui::Text("Reflection: %d", m_count);
}
#endif // DEBUG


