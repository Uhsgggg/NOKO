#pragma once

class TurnSystem
{
public:
	void Reset(int initialShots);
	bool CanLaunch() const;
	bool TryConsumeShot(); // 成功時だけ残数-1、Resolvingへ
	bool EndIfStopped(bool areAnyBallsMoving); // 1ターン終了時だけtrue
	int GetRemainingShots() const;
	bool IsResolving() const;

private:
	int m_remainingShots = 0;
	bool m_resolvingShots = false;
};