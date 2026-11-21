#include "TurnSystem.h"

void TurnSystem::Reset(int shots)
{
	if (shots < 0)
	{
		shots = 0;
		m_remaining = shots;
		m_resolving = false;
	}
}

bool TurnSystem::CanLaunch() const
{
	return m_remaining > 0 && !m_resolving;
}

bool TurnSystem::TryConsumeShot()
{
	if (!CanLaunch()) return false;
	--m_remaining;
	m_resolving = true;
	return true;
}

bool TurnSystem::EndIfStopped(bool anyMoving)
{
	if (!m_resolving || anyMoving) return false;
	m_resolving = false;
	return true;
}

int TurnSystem::GetRemainingShots() const
{
	return m_remaining;
}

bool TurnSystem::IsResolving() const
{
	return m_resolving;
}
