#include "ReflectionCounterComponent.h"
#include "Engine/Scene/GameObject.h"
#ifdef _DEBUG
#include"imgui.h"
#endif


ReflectionCounterComponent::ReflectionCounterComponent(GameObject *pOwner)
	: Component(pOwner)
{
}


void ReflectionCounterComponent::Reset()
{
	m_reflectionCount = 0;
}

void ReflectionCounterComponent::Increment()
{
	if (m_reflectionCount < INT_MAX)
	{
		++m_reflectionCount;
	}
}

int ReflectionCounterComponent::GetCount() const
{
	return m_reflectionCount;
}

std::unique_ptr<Component> ReflectionCounterComponent::Clone(GameObject *pNewOwner) const
{
	auto pCopy = std::make_unique<ReflectionCounterComponent>(pNewOwner);
	pCopy->SetEnabled(IsEnabled());
	return pCopy;
}

#ifdef _DEBUG
void ReflectionCounterComponent::DrawDebugInspector()
{
	ImGui::Text("îΩî≠åWêî: %d", m_reflectionCount);
}
#endif


