#pragma once

class TurnSystem
{
public:
	void Reset(int shots);
	bool canLaunch() const;
	bool TryConsumeShot(); // 成功時だけ残数-1、Resolvingへ
	bool EndIfStopped(bool anyMoving); // 1ターン終了時だけtrue
	int GetRemainingShots() const;
	bool IsResolving() const;

private:
	int m_remaining = 0;
	bool m_resolving = false;
};