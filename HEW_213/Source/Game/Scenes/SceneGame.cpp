//========================================//
// 
// SceneGame.cpp
// 
// DebugEditorから命令を受け取って、
// SceneEditActionsへ渡す。
// 実行結果だけ受け取る。
// 
//========================================//
#include "SceneGame.h"

#include "App/Defines.h"
#include "Engine/Input/Input.h"
#include "ThirdParty/nlohmann/json.hpp"

#include "Engine/Graphics/DirectX/ShaderList.h"
#include "Engine/Graphics/Geometory.h"

#include "Engine/Camera/CameraDebug.h"
#include "Engine/Camera/CameraPlay.h"

#include "Game/Serialization/SceneSerializer.h"

#include "Game/Objects/Ground.h"
#include "Game/Objects/Goal.h"
#include "Game/Objects/PrimitiveObject.h"
#include "Game/Objects/FbxObject.h"

#include "Engine/Graphics/DirectX/DirectX.h"

#include "Game/Components/BallMotionComponent.h"
#include "Game/Components/ColliderComponent.h"
#include "Game/Components/HealthComponent.h"
#include "Game/Components/MoveComponent.h"
#include "Game/Components/PlayerInputComponent.h"
#include "Game/Components/PlayerLaunchComponent.h"
#include "Game/Components/ReflectionCounterComponent.h"

#include "Game/Systems/DamageFormula.h"

#include "Game/UI/HudPresenter.h"

#include <Windows.h>
#include <algorithm>
#include <cmath>
#include <string>

#ifdef _DEBUG
#include "Engine/Debug/DebugEditor.h"
#include "imgui.h"
#include "Engine/Scene/SceneRayPicker.h"
#include "Game/Editor/SceneEditActions.h"
using namespace ImGui;
#endif

using namespace DirectX;
using namespace std;

// namespace はこのcppファイルの中だけで使用する関数をまとめる仕組み
namespace
{
	// ステージの保存先
	const char* const kStageFilePath =
		"Assets/Stages/Stage01.json";

	// プロトタイプの仮仕様。企画確定後に変更するところ
	constexpr int kShots = 10;
	constexpr int kWaveInterval = 3;
	constexpr float kBaseDamage = 3.0f;
	constexpr float kMultiplier = 0.5f;
}




//========================================
// コンストラクタ
//========================================
SceneGame::SceneGame()
	: m_pCamera(nullptr)
	, m_pPlayCamera(nullptr)
	, m_showDebugGrid(true)
{
	//========================================
	// Groundを生成
	//========================================
	unique_ptr<Ground> pGround(new Ground());

	pGround->SetPos(0.0f, 0.0f, 0.0f);
	pGround->SetScale(3.0f, 0.4f, 3.0f);

	// 現在のTransformを初期状態として保存
	pGround->SaveInitialTransform();

	// GameObjectの配列に追加
	m_objects.push_back(move(pGround));


	//========================================
	// Goalを生成する
	//========================================
	unique_ptr<Goal> pGoal(new Goal());

	pGoal->SetPos(0.0f, 1.0f, 0.0f);
	pGoal->SetScale(0.5f, 0.5f, 0.5f);

	pGoal->SaveInitialTransform();

	m_objects.push_back(move(pGoal));


	//========================================
	// カメラ
	//========================================
	// 編集用カメラはDebugのみ
#ifdef _DEBUG
	m_pCamera.reset(new CameraDebug());
#endif

	// Play用カメラはDebug・Release共通
	m_pPlayCamera.reset(new CameraPlay());

	LoadGameplayStage();
	if (!m_hudRenderer.Initialize())
	{
		MessageBoxA(nullptr, "HUD texture creation failed.", "HUD Error", MB_OK | MB_ICONERROR);
	}

#ifndef _DEBUG
	ResetGamePlay();
#endif

	//========================================
	// 保存済みステージを自動読み込み
	//========================================
	// Stage01.jsonが存在するか確認
	DWORD attributes = GetFileAttributesA(
		kStageFilePath
	);

	// ファイルが存在する場合だけ読み込む
	if (attributes != INVALID_FILE_ATTRIBUTES)
	{
		std::string error;

		if (!SceneSerializer::Load(
			kStageFilePath,
			m_objects,
			*m_pPlayCamera,
			error))
		{
			// 読み込みに失敗した場合だけ通知
			MessageBoxA(
				nullptr,
				error.c_str(),
				"Stage Load Error",
				MB_OK | MB_ICONERROR
			);
		}
	}
}




//========================================
// デストラクタ
//========================================
SceneGame::~SceneGame() = default;




//========================================
// 更新処理
//========================================
void SceneGame::Update()
{
	// 更新で進める時間
	const float deltaTime = 1.0f / FPS;

	// ゲームを更新するかどうか
	bool shouldUpdateGame = true;

#ifdef _DEBUG
	//========================================
	// F1でモード切り替え
	//========================================
	if (IsKeyTrigger(VK_F1))
	{
		if (m_isEditMode)
		{
			// Edit -> Play
			EnterPlayMode();
		}
		else
		{
			// Play -> Edit
			ExitPlayMode();
		}
	}

	// Edit Mode中はゲームを更新しない
	shouldUpdateGame = !m_isEditMode;
	UpdateColliderDebug();
#endif

	//========================================
	// ゲームの更新処理
	//========================================
	if (shouldUpdateGame)
	{
		if (IsKeyTrigger('R'))
		{
#ifdef _DEBUG
			ExitPlayMode();
			EnterPlayMode();
#else
			if (LoadGameplayStage())
			{
				ResetGamePlay();
			}
#endif
		}
		UpdateGameplay(deltaTime);
	}

	//========================================
	// カメラ更新
	//========================================
#ifdef _DEBUG
	if (m_isEditMode)
	{
		// Edit Modeでは自由操作カメラ
		m_pCamera->Update();
	}
	else
	{
		// Play Modeでは固定カメラ
		m_pPlayCamera->Update();
	}
#else
	// Releaseでは常にPlay専用カメラ
	m_pPlayCamera->Update();
#endif
}




//========================================
// 描画処理
//========================================
void SceneGame::Draw()
{
	//========================================
	// ライティング設定
	//========================================
	ShaderList::SetLight(
		XMFLOAT4(1.2f, 1.2f, 1.2f, 1.0f),
		XMFLOAT3(0.0f, -1.0f, 0.0f)
	);

	//========================================
	// 使用するカメラを決定
	//========================================
	// 基本的にはPlay用カメラを使用
	Camera* pRenderCamera = m_pPlayCamera.get();

#ifdef _DEBUG
	// Edit Modeの場合だけDebugカメラを使用
	if (m_isEditMode)
	{
		pRenderCamera = m_pCamera.get();
	}
#endif

	//========================================
	// カメラ行列の設定
	//========================================
	const XMFLOAT4X4 view =
		pRenderCamera->GetViewMatrix();
	const XMFLOAT4X4 projection =
		pRenderCamera->GetProjectionMatrix();

	Geometory::SetView(view);
	Geometory::SetProjection(projection);

	// FBXオブジェクトにもカメラ行列を設定
	FbxObject::SetCameraMatrices(view, projection);

	//========================================
	// すべてのGameOjbectを描画
	//========================================
	for (auto& pObject : m_objects)
	{
		if (pObject && pObject->IsActive())
		{
			pObject->Draw();
		}
	}

	//========================================
	// Edit Modeでのみグリッドを描画
	//========================================
#ifdef _DEBUG
	if (m_isEditMode && m_showDebugGrid)
	{
		DrawDebugGrid();
	}
#endif

#ifdef _DEBUG
	// モードに関係なく表示する
	DrawColliderDebugPanel();
	DrawColliderDebug();
	if (m_isEditMode)
	{
		DrawEditor();
	}
	else
	{
		DrawGameplayHUD();
	}
#else
	DrawGameplayHUD();
#endif
}




#ifdef _DEBUG
//========================================
// Edit Modeのエディタ処理
//========================================
void SceneGame::DrawEditor()
{
	//========================================
	// 3D画面内でのオブジェクト選択
	//========================================
	float mouseX = 0.0f;
	float mouseY = 0.0f;
	float viewportWidth = 0.0f;
	float viewportHeight = 0.0f;

	// 3D画面が左クリックされたか確認
	if (DebugEditor::TryGetSceneClick(
		mouseX,
		mouseY,
			viewportWidth,
			viewportHeight))
			{
				// Raycastでオブジェクトを探す
				int selectedIndex = SceneRayPicker::Pick(
					m_objects,
					*m_pCamera,
					mouseX,
					mouseY,
					viewportWidth,
					viewportHeight
				);

				// オブジェクトに当たった場合だけ選択
				if (selectedIndex >= 0)
				{
					DebugEditor::SetSelectedObjectIndex(
						selectedIndex
					);
				}
	}

	//========================================
	// UIから命令を受け取る
	//========================================
	EditorCommand command =
		DebugEditor::DrawScene(
			m_objects,
			m_showDebugGrid
		);

	//========================================
	// 命令を実行する
	//========================================
	SceneEditResult result =
		SceneEditActions::Execute(
			command,
			m_objects,
			*m_pPlayCamera,
			kStageFilePath
		);

	//========================================
	// 実行結果を反映する
	//========================================
	if (result.selectionChanged)
	{
		DebugEditor::SetSelectedObjectIndex(
			result.selectedIndex
		);
	}

	//========================================
	// エラーがあれば表示
	//========================================
	if (!result.error.empty())
	{
		MessageBoxA(
			nullptr,
			result.error.c_str(),
			"Scene Editor Error",
			MB_OK | MB_ICONERROR
		);
	}

	//========================================
	// Playカメラ設定
	//========================================
	DebugEditor::DrawPlayCamera(*m_pPlayCamera, *m_pCamera);
}
#endif




#ifdef _DEBUG
void SceneGame::UpdateColliderDebug()
{
	for (auto& pObject : m_objects)
	{
		if (!pObject) continue;
		auto* pCollider = pObject->GetComponent<ColliderComponent>();
		if (pCollider)
		{
			pCollider->UpdateDebugHit();
		}
	}
}
#endif




#ifdef _DEBUG
void SceneGame::DrawColliderDebugPanel()
{// ImGui
	if (GetCurrentContext()) return;

	float centerX = GetIO().DisplaySize.x * 0.5f;
	SetNextWindowPos(ImVec2(centerX, 12.0f),
		ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.0f));

	if (Begin("Collider Debug", nullptr,
		ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse))
	{
		Checkbox("Show Colliders", &m_bShowColliders);
		TextColored(ImVec4(0.1f, 0.65f, 1.0f, 1.0f), "Player");
		SameLine();
		TextColored(ImVec4(0.1f, 0.2f, 0.2f, 1.0f), "Enemy");
		SameLine();
		TextColored(ImVec4(0.8f, 0.3f, 1.0f, 1.0f), "Boss");
		SameLine();
		TextColored(ImVec4(1.0f, 0.85f, 0.15f, 1.0f), "Wall");
		TextUnformatted("White = Hit (9 frames)");
	}
	End();
}
#endif




#ifdef _DEBUG
void SceneGame::DrawColliderDebug()
{
	if (!m_bShowColliders) return;

	// 深度とブレンドの元の設定を保存しておく
	ID3D11DepthStencilState* pDepth = nullptr;
	UINT stencil = 0;
	ID3D11BlendState* pBlend = nullptr;
	FLOAT blendFactor[4] = {};
	UINT sampleMask = 0;
	GetContext()->OMGetDepthStencilState(&pDepth, &stencil);
	GetContext()->OMGetBlendState(&pBlend, blendFactor, &sampleMask);

	// モデルに隠れず、半透明で線を描く。
	SetDepthTest(false);
	SetBlendMode(BLEND_ALPHA);

	for (auto& pObject : m_objects)
	{
		if (!pObject || !pObject->IsActive()) continue;
		auto* pCollider = pObject->GetComponent<ColliderComponent>();
		if (pCollider)
		{
			pCollider->DrawDebug();
		}
	}

	// 後から描くHUDなどに影響させない。
	GetContext()->OMSetDepthStencilState(pDepth, stencil);
	GetContext()->OMSetBlendState(pBlend, blendFactor, sampleMask);
	if (pDepth)
		pDepth->Release();
	if (pBlend)
		pBlend->Release();
}
#endif




void SceneGame::OnResize(unsigned int width, unsigned int height)
{
	if (width == 0 || height == 0) return;

	// 新しい画面の縦横比
	const float aspect =
		static_cast<float>(width) /
		static_cast<float>(height);

	//========================================
	// Edit用カメラ
	//========================================
	if (m_pCamera)
	{
		m_pCamera->SetAspectRatio(aspect);
	}

	//========================================
	// Play用カメラ
	//========================================
	if (m_pPlayCamera)
	{
		m_pPlayCamera->SetAspectRatio(aspect);
	}
}




//========================================
// グリッド描画処理
//========================================
void SceneGame::DrawDebugGrid()
{
#ifdef _DEBUG
	const int gridCount = 10;
	const float gridSize = 1.0f;
	const float extent = gridCount * gridSize;

	const XMFLOAT4 gridColor(0.5f, 0.5f, 0.5f, 1.0f);

	// ｘ方向とｚ方向にグリッドを生成
	for (int i = -gridCount; i <= gridCount; i++)
	{
		// 中央線はｘｙｚ軸と重なるので描画しない
		if (i == 0) continue;

		float offset = i * gridSize;

		// ｚ方向の線
		Geometory::AddLine
		(
			XMFLOAT3(offset, 0, -extent),
			XMFLOAT3(offset, 0, extent),
			gridColor
		);

		// ｘ方向の線
		Geometory::AddLine
		(
			XMFLOAT3(-extent, 0, offset),
			XMFLOAT3(extent, 0, offset),
			gridColor
		);
	}

	// ｘ軸：赤
	Geometory::AddLine
	(
		XMFLOAT3(-extent, 0, 0),
		XMFLOAT3(extent, 0, 0),
		XMFLOAT4(1, 0, 0, 1)
	);

	// ｙ軸：緑
	Geometory::AddLine
	(
		XMFLOAT3(0, 0, 0),
		XMFLOAT3(0, 5, 0),
		XMFLOAT4(0, 1, 0, 1)
	);

	// ｚ軸：青
	Geometory::AddLine
	(
		XMFLOAT3(0, 0, -extent),
		XMFLOAT3(0, 0, extent),
		XMFLOAT4(0, 0, 1, 1)
	);

	Geometory::DrawLines();
#endif
}




//========================================
// Play Modeを開始する処理
//========================================
#ifdef _DEBUG
void SceneGame::EnterPlayMode()
{
	// すでにPlay Modeなら何もしない
	if (!m_isEditMode) return;

	//========================================
	// Play用のオブジェクトを作る
	//========================================
	vector<unique_ptr<GameObject>> playObjects;

	// vectorにどれぐらい要素を入れるかが分かっている場合、
	// reserve で、あらかじめ必要な容量を確保する
	playObjects.reserve(m_objects.size());

	for (const auto& pObject : m_objects)
	{
		if (!pObject)
		{
			MessageBoxA(
				nullptr,
				"Null GameObject was found.",
				"Play Mode Error",
				MB_OK | MB_ICONERROR
			);
			return;
		}

		// Cloneで別のオブジェクトを生成
		unique_ptr<GameObject> pClone = pObject->Clone();

		// 複製失敗時
		if (!pClone)
		{
			MessageBoxA(
				nullptr,
				"Failed to clone GameObject.",
				"Play Mode Error",
				MB_OK | MB_ICONERROR
			);
			return;
		}

		playObjects.push_back(move(pClone));
	}

	//========================================
	// 元の編集オブジェクトを保管
	//========================================
	m_editObjects.swap(m_objects);

	//========================================
	// Play用の複製へ切り替える
	//========================================
	m_objects.swap(playObjects);

	// Play Modeへ変更
	m_isEditMode = false;
	ResetGamePlay();
}




//========================================
// Play Modeを終了する処理
//========================================
void SceneGame::ExitPlayMode()
{
	// すでにExit Modeなら何もしない
	if (m_isEditMode) return;

	//========================================
	// 編集用オブジェクトを復元
	//========================================
	m_objects.swap(m_editObjects);

	//========================================
	// Play用のオブジェクトを破棄
	//========================================
	m_boardPointer.Reset();
	m_collision.Reset();
	m_editObjects.clear();
	m_gameReady = false;

	// Edit Modeへ変更
	m_isEditMode = true;
}
#endif
