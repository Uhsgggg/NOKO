#pragma once

#ifdef _DEBUG
#include <Windows.h>
#include <vector>
#include <memory>

#include "Game/Editor/EditorCommand.h"

class GameObject;
class Camera;
class CameraPlay;
#endif

class DebugEditor
{
public:
#ifdef _DEBUG
	// ImGuiの起動・終了
	static bool Initialize(HWND hWnd);
	static void Shutdown();

	// 1フレームの開始・終了
	static void BeginFrame();
	static void Render();

	// Windowsからの入力をImGuiへ渡す
	static bool HandleWndProc(
		HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	// ゲームオブジェクトの編集画面
	static EditorCommand DrawScene(
		std::vector<std::unique_ptr<GameObject>>& objects,
		bool& showGrid);

	// ImGuiが入力を使用しているか
	static bool WantsMouse();
	// 3Dシーン上で左クリックされたか判定
	static bool TryGetSceneClick(
		float& mouseX,
		float& mouseY,
		float& viewportWidth,
		float& viewportHeight
	);

	static bool WantsKeyboard();

	static void SetSelectedObjectIndex(int index);

	// Playカメラの設定画面
	static void DrawPlayCamera(
		CameraPlay& playCamera,
		Camera& editCamera
	);

#else
	// ReleaseではImGuiが存在しなくても動く
	static bool WantsMouse()
	{
		return false;
	}

	static bool WantsKeyboard()
	{
		return false;
	}
#endif

private:
#ifdef _DEBUG
	static bool s_initialized;
	static int  s_selectedObjectIndex;
#endif
};

