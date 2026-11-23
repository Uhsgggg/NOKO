#pragma once

#include "Game/Systems/GameTypes.h"

#include <memory>
#include <utility>

class CollisionSystem
{
public:
	void Reset();

	// 描画・HP変更・vector削除はしない。
	// 通知は呼び出し直後に消費。
	void Step(
		const std::vector<std::unique_ptr<GameObject>>& objects,
		std::vector<CollisionNotice>& outNotices);

	static bool Detect(
		GameObject& objectA, GameObject& objectB, Contact& outContacts);

	static BoardVec2 Reflect(
		BoardVec2 velocity, BoardVec2 normal, float restitution);

private:
	struct HitPair
	{
		GameObject* objectA;
		GameObject* objectB;
	};

	std::vector<HitPair> m_previousContacts;

	void SolvePair(
		GameObject& objectA, GameObject& objectB,
		std::vector<CollisionNotice>& outNotices,
		std::vector<HitPair>& currentContacts);
};