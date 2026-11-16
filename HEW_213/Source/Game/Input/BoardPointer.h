#pragma once

#include "Game/Systems/GameTypes.h"

class Camera;
class GameObject;

class BoardPointer
{
public:
	// 1フレーム分のマウス入力を作成する
	PointerFrame BuildFrame(
		Camera& camera,
		const GameObject& player,
		float playerRadius);

	// 入力状態を初期化する
	void Reset();

private:
	// 画面座標をXZ平面へ変換する
	bool ScreenToBoard(
		Camera& camera,
		float mouseX, float mouseY,
		float width, float height,
		float boardY,
		BoardVec2& outPoint) const;

	bool m_bPreviousHeld = false;
	bool m_bWaitForRelease = true;
};

