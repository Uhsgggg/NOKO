#ifndef __SCENE_GAME_H__
#define __SCENE_GAME_H__

#include "Engine/Camera/Camera.h"
#include "Engine/Scene/GameObject.h"
#include "Engine/Graphics/StaticFbxModel.h"
#include "Engine/Scene/Scene.h"

#include "Game/Input/BoardPointer.h"
#include "Game/Systems/CollisionSystem.h"
#include "Game/Systems/TurnSystem.h"
#include "Game/UI/HudRenderer.h"

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
	void ResetGamePlay();
	void UpdateGameplay(float deltaTime);
	void UpdatePlayerInput();
	void UpdatePhysics(float deltaTime);
	void ApplyCollisionNotices(const std::vector<CollisionNotice>& notices);
	bool AnyMoving() const;
	void FinishTurn();
	bool SpawnEnemy();
	void UpdateHUD();
	void DrawGameplayHUD();
	bool LoadGameplayStage();

private: // main
	// GameObjectをまとめて管理する
	std::vector<std::unique_ptr<GameObject>> m_objects;
	
	// Hierarchyで選択中のオブジェクト番号
	//int m_selectedObjectIndex;

	// 編集専用カメラ
	std::unique_ptr<Camera> m_pCamera;

	// プレイ専用カメラ
	std::unique_ptr<CameraPlay> m_pPlayCamera;

	// グリッドを表示するかどうか
	bool m_showDebugGrid;

private: // Systems
	CollisionSystem m_collision;
	TurnSystem m_turn;
	HudRenderer m_hudRenderer;
	HudView m_hud;
	int m_result = 0; // 0: playing, 1: clear, 2: game over
	int m_completedShots = 0;
	int m_spawnSerial = 0;
	int m_hasBoss = false;
	int m_hadEnemies = false;
	BoardPointer m_boardPointer;
	bool m_gameReady = false;
	float m_arrowY = 0.65f;

#ifdef _DEBUG
private: // Debug
	bool m_bShowColliders = true;
	void UpdateColliderDebug();
	void DrawColliderDebug();
	void DrawColliderDebugPanel();

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