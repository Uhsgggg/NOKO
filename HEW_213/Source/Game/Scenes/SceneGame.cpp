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
#include "ThirdParty/nlohmann/json.hpp"
#include "Engine/Input/Input.h"

#include "Engine/Graphics/DirectX/ShaderList.h"
#include "Engine/Graphics/Geometory.h"

#include "Engine/Camera/CameraDebug.h"
#include "Engine/Camera/CameraPlay.h"

#include "Game/Serialization/SceneSerializer.h"

#include "Game/Objects/Ground.h"
#include "Game/Objects/Goal.h"
#include "Game/Objects/FbxObject.h"

#include <string>
#include <Windows.h>

#ifdef _DEBUG
#include "Engine/Debug/DebugEditor.h"
#include "Engine/Scene/SceneRayPicker.h"
#include "Game/Editor/SceneEditActions.h"
#endif

using namespace DirectX;
using namespace std;

// namespace はこのcppファイルの中だけで使用する関数をまとめる仕組み
namespace
{
	// ステージの保存先
	const char* const kStageFilePath =
		"Assets/Stages/Stage01.json";
}




//========================================
// コンストラクタ
//========================================
SceneGame::SceneGame()
	: m_selectedObjectIndex(0)
	, m_pCamera(nullptr)
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

	// 最初はGoalを選択
	m_selectedObjectIndex = 1;


	//========================================
	// カメラ
	//========================================
	// 編集用カメラはDebugのみ
#ifdef _DEBUG
	m_pCamera = new CameraDebug;
#endif

	// Play用カメラはDebug・Release共通
	m_pPlayCamera = new CameraPlay;


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
SceneGame::~SceneGame()
{
	if (m_pCamera)
	{
		delete m_pCamera;
		m_pCamera = nullptr;
	}
	if (m_pPlayCamera)
	{
		delete m_pPlayCamera;
		m_pPlayCamera = nullptr;
	}
}




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
#endif

	//========================================
	// ゲームの更新処理
	//========================================
	if (shouldUpdateGame)
	{
		// 全GameObjectのコンポーネントを更新
		for (auto& pObject : m_objects)
		{
			if (pObject)
			{
				pObject->Update(deltaTime);
			}
		}

		// Enemyの更新
		for (auto& pEnemy : m_enemies)
		{
			if (pEnemy)
			{
				pEnemy->Update(deltaTime);
			}
		}
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
	Camera* pRenderCamera = m_pPlayCamera;

#ifdef _DEBUG
	// Edit Modeの場合だけDebugカメラを使用
	if (m_isEditMode)
	{
		pRenderCamera = m_pCamera;
	}
#endif

	//========================================
	// カメラ行列の設定
	//========================================
	const XMFLOAT4X4 view = pRenderCamera->GetViewMatrix();
	const XMFLOAT4X4 projection = pRenderCamera->GetProjectionMatrix();

	Geometory::SetView(view);
	Geometory::SetProjection(projection);

	// FBXオブジェクトにもカメラ行列を設定
	FbxObject::SetCameraMatrices(view, projection);

	//========================================
	// すべてのGameOjbectを描画
	//========================================
	for (auto& pObject : m_objects)
	{
		pObject->Draw();
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

	//========================================
	// 敵の描画
	//========================================
	for (auto& pEnemy : m_enemies)
	{
		pEnemy->Draw(view, projection);
	}

#ifdef _DEBUG
	//========================================
	// Edit Modeのエディタ処理
	//========================================
	if (m_isEditMode)
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
		DebugEditor::DrawPlayCamera(
			*m_pPlayCamera,
			*m_pCamera
		);
	}
#endif
}




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
	m_editObjects.clear();

	// Edit Modeへ変更
	m_isEditMode = true;
}
#endif
