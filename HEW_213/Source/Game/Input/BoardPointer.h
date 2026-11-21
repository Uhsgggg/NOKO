#pragma once

#include "Game/Systems/GameTypes.h"

#include <Windows.h>

class Camera;

class BoardPointer
{
public:
	// Startup.cppから呼ぶRaw Inputの入口
	static bool RegisterMouse(HWND hWnd);
	static void ReceiveRawInput(LPARAM lParam);
	static void CancelRawInput();

	// 毎フレーム1回呼ぶ（発射可能なPlay中のみ）
	PointerFrame BuildFrame(Camera& camera);

	// 入力状態を初期化する
	void Reset();

private:
	bool m_bDragging = false;
	bool m_bWaitForRelease = true;
	BoardVec2 m_point;
	BoardVec2 m_right;
};

