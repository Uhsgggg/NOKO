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
		GameObject& a, GameObject& b, Contact& out);

	static BoardVec2 Reflect(
		BoardVec2 velocity, BoardVec2 normal, float restitution);

private:
	std::vector<std::pair<GameObject*, GameObject*>>
		m_previousContacts;
};