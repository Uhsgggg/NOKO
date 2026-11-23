#include "TurnSystem.h"

void TurnSystem::Reset(int initialShots)
{
	if (initialShots < 0)
	{
		initialShots = 0;
	}
	m_remainingShots = initialShots;
	m_resolvingShots = false;
}

bool TurnSystem::CanLaunch() const
{
	return m_remainingShots > 0 && !m_resolvingShots;
}

bool TurnSystem::TryConsumeShot()
{
	if (!CanLaunch()) return false;
	--m_remainingShots;
	m_resolvingShots = true;
	return true;
}

bool TurnSystem::EndIfStopped(bool areAnyBallsMoving)
{
	if (!m_resolvingShots || areAnyBallsMoving) return false;
	m_resolvingShots = false;
	return true;
}

int TurnSystem::GetRemainingShots() const
{
	return m_remainingShots;
}

bool TurnSystem::IsResolving() const
{
	return m_resolvingShots;
}
