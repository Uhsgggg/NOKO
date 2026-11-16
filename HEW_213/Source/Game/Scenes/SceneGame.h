#ifndef __SCENE_GAME_H__
#define __SCENE_GAME_H__

#include "Engine/Scene/Scene.h"
#include "Engine/Scene/GameObject.h"
#include "Engine/Camera/Camera.h"
#include "Engine/Graphics/StaticFbxModel.h"

#include "Game/Systems/StopAllSystem.h"

#include <vector>
#include <memory>

class CameraPlay;

class SceneGame : public Scene
{	
public:
	SceneGame();
	~SceneGame();

	void Update() final;
	void Draw() final;

	void OnResize(unsigned int width, unsigned int height) override;

private:
	void DrawDebugGrid();

private: // main
	// GameObjectをまとめて管理する
	std::vector<std::unique_ptr<GameObject>> m_objects;
	
	// Hierarchyで選択中のオブジェクト番号
	int m_selectedObjectIndex;

	// 編集専用カメラ
	std::unique_ptr<Camera> m_pCamera;

	// プレイ専用カメラ
	std::unique_ptr<CameraPlay> m_pPlayCamera;

	// グリッドを表示するかどうか
	bool m_showDebugGrid;

private: // Systems
	// 全オブジェクト停止システム
	StopAllSystem m_stopAll;

#ifdef _DEBUG
private: // Debug
	// true  : Edit Mode
	// false : Play Mode
	bool m_isEditMode = true;

	// Play中に編集用のGameObjectを保管する
	std::vector<std::unique_ptr<GameObject>> m_editObjects;

	// Play Modeを開始する
	void EnterPlayMode();
	void DrawEditor();

	// Play Modeを終了する
	void ExitPlayMode();
#endif
};

#endif // __SCENE_GAME_H__