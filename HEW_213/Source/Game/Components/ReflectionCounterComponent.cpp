#include "ReflectionCounterComponent.h"
#include "Engine/Scene/GameObject.h"
#ifdef _DEBUG
#include"imgui.h"
using namespace ImGui;
#endif

#include <climits>

ReflectionCounterComponent::ReflectionCounterComponent(GameObject *pOwner)
	: Component(pOwner)
{
}


void ReflectionCounterComponent::Reset()
{
	m_reflectionCount = 0;
	m_attackLevel = 0;
}

void ReflectionCounterComponent::Increment()
{
	RecordReflection(1);
}

int ReflectionCounterComponent::GetCount() const
{
	return m_reflectionCount;
}

void ReflectionCounterComponent::RecordReflection(int attackLevelGain)
{
	if (m_reflectionCount < INT_MAX)
	{
		++m_reflectionCount;
	}

	if (attackLevelGain <= 0) return;

	if (m_attackLevel > INT_MAX - attackLevelGain)
	{
		m_attackLevel = INT_MAX;
	}
	else
	{
		m_attackLevel += attackLevelGain;
	}
}

int ReflectionCounterComponent::GetAttackLevel() const
{
	return m_attackLevel;
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
	Text("îΩî≠âÒêî: %d", m_reflectionCount);
	Text("çUåÇã≠âªíl: %d", m_attackLevel);
}
#endif


