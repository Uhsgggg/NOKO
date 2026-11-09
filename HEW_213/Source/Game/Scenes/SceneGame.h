#ifndef __SCENE_GAME_H__
#define __SCENE_GAME_H__

#include "Source/Engine/Scene/Scene.h"
#include "Source/Engine/Scene/GameObject.h"
#include "Source/Engine/Camera/Camera.h"
#include "Source/Engine/Graphics/StaticFbxModel.h"
#include "Source/Game/Enemies/Enemy.h"

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

private:
	// GameObjectをまとめて管理する
	std::vector<std::unique_ptr<GameObject>> m_objects;
	
	// Hierarchyで選択中のオブジェクト番号
	// -1 は、今回だけKingSlimeを表す
	int m_selectedObjectIndex;

	// 編集専用カメラ
	Camera* m_pCamera;

	// プレイ専用カメラ
	CameraPlay* m_pPlayCamera;

	// グリッドを表示するかどうか
	bool m_showDebugGrid;

	// Enemyをまとめて管理する
	std::vector<std::unique_ptr<Enemy>> m_enemies;

	// FBX : KingSlime
	StaticFbxModel m_kingSlimeModel;

#ifdef _DEBUG
private:
	// true  : Edit Mode
	// false : Play Mode
	bool m_isEditMode = true;

	// Play中に編集用のGameObjectを保管する
	std::vector<std::unique_ptr<GameObject>> m_editObjects;

	// Play Modeを開始する
	void EnterPlayMode();

	// Play Modeを終了する
	void ExitPlayMode();
#endif
};

#endif // __SCENE_GAME_H__