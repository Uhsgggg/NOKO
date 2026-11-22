//========================================//
// 
// DebugEditor.cpp
// 
//########################################
// ＊DebugEditor::DrawScene()
// 
// 　・Hierarchy
// 　　　…
// 
// 　・Inspector
// 　　　DrawObjectAppearance()
// 　　　　Shape・FBX・Color…
// 
// 　　　DrawObjectTransform()
// 　　　　Position・Rotation・Scale…
// 　　　
// 　　　DrawObjectComponents()
// 　　　　Component一覧・パラメータ・追加・削除
// 
//########################################
// 
//========================================//
#include "DebugEditor.h"

#ifdef _DEBUG
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

#include "Engine/Graphics/DirectX/DirectX.h"
#include "Engine/Scene/GameObject.h"
#include "Engine/Scene/Component.h"
#include "Engine/Camera/Camera.h"
#include "Engine/Camera/CameraPlay.h"

#include "Game/Objects/PrimitiveObject.h"
#include "Game/Objects/FbxObject.h"

#include "Game/Components/ComponentRegistry.h"

#include <Windows.h>
#include <algorithm>
#include <vector>
#include <string>
#include <cstdio>
#include <utility>

using namespace DirectX;
using namespace ImGui;
using namespace std;

//========================================
// エディタのレイアウト計算
//========================================
namespace
{
	struct EditorPanelLayout
	{
		ImVec2 position;
		ImVec2 size;

		float hierarchyWidth = 0.0f;
		float inspectorWidth = 0.0f;
		float inspectorHeight = 0.0f;
	};

	EditorPanelLayout CalculateEditorLayout()
	{
		const ImGuiViewport* pViewport =
			GetMainViewport();

		EditorPanelLayout layout;

		// ImGuiが現在使用できる描画領域
		layout.position = pViewport->WorkPos;
		layout.size = pViewport->WorkSize;

		//========================================
		// Hierarchyの横幅
		//========================================
		float leftWidth = layout.size.x * 0.22f;

		if (leftWidth < 240.0f) leftWidth = 240.0f;
		if (leftWidth > 300.0f) leftWidth = 300.0f;

		layout.hierarchyWidth = leftWidth;

		//========================================
		// Inspectorの横幅
		//========================================
		float rightWidth = layout.size.x * 0.26f;

		if (rightWidth < 190.0f) rightWidth = 190.0f;
		if (rightWidth > 250.0f) rightWidth = 250.0f;

		layout.inspectorWidth = rightWidth;

		// Inspectorは右側の上75%
		layout.inspectorHeight = layout.size.y * 0.75f;

		return layout;
	}

	//========================================
	// Assets/Model内のFBX一覧を取得
	//========================================
	vector<string> FindFbxFiles()
	{
		vector<string> files;
		WIN32_FIND_DATAA findData = {};

		// 指定したフォルダからFBXファイルを探す
		HANDLE hFind = FindFirstFileA(
			"Assets/Model/*.fbx",
			&findData
		);

		// FBXが見つからなかった場合
		if (hFind == INVALID_HANDLE_VALUE) return files;

		do
		{
			// フォルダは除外
			if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				continue;
			}

			// ファイル名から相対パスを作る
			string path = "Assets/Model/";

			path += findData.cFileName;

			files.push_back(path);
		}

		while (FindNextFileA(hFind, &findData));

		FindClose(hFind);

		// ファイル名順に並べる
		sort(files.begin(), files.end());

		return files;
	}

	void DrawObjectAppearance(GameObject* pObject)
	{
		//========================================
		// PrimitiveObjectの形状設定
		//========================================
		PrimitiveObject* pPrimitive =
			dynamic_cast<PrimitiveObject*>(pObject);

		if (pPrimitive)
		{
			// Comboに表示する名前
			// Combo : 選択肢の一覧から1つを選ぶUI
			const char* shapeNames[] =
			{
				"Box",
				"Cylinder",
				"Sphere"
			};

			// 現在の形状をintへ変換
			int shapeIndex =
				static_cast<int>(pPrimitive->GetShape());

			// 形状を変更するプルダウン
			if (Combo(
				"Shape",
				&shapeIndex,
				shapeNames, 3))
			{
				pPrimitive->SetShape(
					static_cast<PrimitiveShape>(shapeIndex)
				);
			}
		}

		Separator();

		//========================================
		// FBXモデルの設定
		//========================================
		FbxObject* pFbx =
			dynamic_cast<FbxObject*>(pObject);

		if (pFbx)
		{
			// 現在のモデルパスを表示
			TextWrapped(
				"Model: %s",
				pFbx->GetModelPath().c_str()
			);

			// モデル選択コンボ
			if (BeginCombo(
				"FBX File",
				pFbx->GetModelPath().c_str()))
			{
				const auto files = FindFbxFiles();
				for (const string& path : files)
				{
					bool selected =
						(pFbx->GetModelPath() == path);

					if (Selectable(path.c_str(), selected))
					{
						// 選択したFBXへ切り替える
						pFbx->SetModelPath(path);
					}

					if (selected)
					{
						SetItemDefaultFocus();
					}
				}

				EndCombo();
			}
		}

		Separator();

		//========================================
		// GameObjectの色編集
		//========================================
		// 現在の色を一時的に取得
		XMFLOAT4 color = pObject->GetColor();

		// ImGuiで色を変更
		if (ColorEdit4("Color", &color.x))
		{
			// 変更された色をGameObjectへ反映
			pObject->SetColor(color);
		}

		TextDisabled(
			"FBX alpha is not supported; primitive alpha is limited."
		);

		Separator();
	}

	void DrawObjectTransform(GameObject* pObject)
	{
		//========================================
		// Transform
		//========================================
		Transform& transform = pObject->GetTransform();

		PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0.0f, 5.0f));
		if (("Transform",
			ImGuiTreeNodeFlags_DefaultOpen))
		{
			DragFloat3(
				"  位置",
				transform.position,
				0.05f
			);
			DragFloat3(
				"  回転",
				transform.rotation,
				1.0f
			);
			DragFloat3(
				"  拡縮",
				transform.scale,
				0.05f,
				0.01f,
				20.0f
			);
		}
		PopStyleVar();

		if (Button("保存したデータを読み込む"))
		{
			pObject->ResetTransform();
		}

		Separator();
	}

	void DrawObjectComponents(GameObject* pObject)
	{
		//========================================
		// Components
		//========================================
		// 現在選択しているGameObjectの
		// コンポーネント一覧を取得
		const auto& components = pObject->GetComponents();

		// 削除予定のコンポーネント
		Component* pRemoveComponent = nullptr;

		// コンポーネントが1つもない場合
		if (components.empty())
		{
			TextDisabled("コンポーネントが設定されていません");
		}

		// コンポーネント一覧を表示
		for (const auto& pComponent : components)
		{
			if (!pComponent) continue;

			// 同じ種類のコンポーネントのID重複を防ぐ
			PushID(pComponent.get());

			// 有効・無効の切り替え
			bool enabled = pComponent->IsEnabled();

			if (Checkbox("有効", &enabled))
			{
				pComponent->SetEnabled(enabled);
			}

			SameLine();

			// 削除ボタン
			if (SmallButton("削除"))
			{
				// ※ここではまだ削除しない
				// vectorを順番に見ている途中で、そのvectorの要素を
				// 削除することでループが使用している状態が無効になり、
				// エラーに繋がる可能性があるから。
				pRemoveComponent = pComponent.get();
			}

			// コンポーネント専用Inspector
			if (CollapsingHeader(
				pComponent->GetTypeName(),
				ImGuiTreeNodeFlags_DefaultOpen))
			{
				const float availableWidth =
					GetContentRegionAvail().x;

				float inputWidth = availableWidth * 0.55f;

				inputWidth = 100.0f;

				PushItemWidth(inputWidth);

				pComponent->DrawDebugInspector();

				PopItemWidth();
			}

			Separator();
			PopID();
		}

		// for文が終わってから削除する
		if (pRemoveComponent != nullptr)
		{
			pObject->RemoveComponent(pRemoveComponent);
		}


		//========================================
		// Add Component
		//========================================
		// ボタンを押すとメニューを開く
		if (Button("コンポーネントを追加する"))
		{
			OpenPopup("AddComponentPopup");
		}

		// 追加メニュー
		if (BeginPopup("AddComponentPopup"))
		{
			// 全コンポーネントの登録情報
			const auto& entries =
				ComponentRegistry::GetEntries();

			// 登録されている種類を順番に表示
			for (const auto& entry : entries)
			{
				bool alreadyAdded = false;

				// 同じComponentが追加済みかどうか
				for (const auto& pExisting : pObject->GetComponents())
				{
					if (pExisting &&
						string(pExisting->GetTypeId())
						== entry.typeId)
					{
						alreadyAdded = true;
						break;
					}
				}

				// 追加済みなら選択できない
				if (MenuItem(
					entry.displayName,
					nullptr,
					false,
					!alreadyAdded))
				{
					// 登録されている生成関数を呼ぶ
					unique_ptr<Component> pNew =
						entry.create(pObject);

					if (pNew)
					{
						pObject->AddComponent(move(pNew));
					}
				}
			}

			EndPopup();
		}
	}
}

// Win32バックエンドのメッセージ処理
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
	HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam
);

// staticメンバ変数の実体を定義
bool DebugEditor::s_initialized = false;
int DebugEditor::s_selectedObjectIndex = 1;

bool DebugEditor::Initialize(HWND hWnd)
{
	if (s_initialized) return true;

	IMGUI_CHECKVERSION();

	CreateContext();

	// ImGuiの設定情報を取得
	ImGuiIO io = GetIO();

	// 日本語対応フォントを読み込む
	ImFont* pFont = io.Fonts->AddFontFromFileTTF(
		"C:\\Windows\\Fonts\\meiryo.ttc",
		20.0f,
		nullptr,
		io.Fonts->GetGlyphRangesJapanese()
	);

	// フォントの読み込みに失敗した場合
	if (pFont == nullptr)
	{
		MessageBoxA(
			hWnd,
			"Japanese font load failed.",
			"DebugEditor Error",
			MB_OK | MB_ICONERROR
		);

		DestroyContext();
		return false;
	}

	// ImGuiのスタイルを設定する
	StyleColorsDark();

	ImGuiStyle& style = GetStyle();

	//========================================
	// 余白・間隔
	//========================================
	// ウィンドウ内側の余白
	style.WindowPadding = ImVec2(10.0f, 8.0f);
	// ボタン・入力欄の内側の余白
	style.FramePadding = ImVec2(5.0f, 3.0f);
	// UI部品同士の間隔
	style.ItemSpacing = ImVec2(6.0f, 4.0f);
	// 1つのUI内の細かな間隔
	style.ItemInnerSpacing = ImVec2(4.0f, 3.0f);

	//========================================
	// 角の丸み
	//========================================
	// ウィンドウ
	style.WindowRounding = 3.0f;

	//========================================
	// 枠線
	//========================================
	// ウィンドウ
	style.WindowBorderSize = 2.0f;
	// ボタン・入力欄
	style.FrameBorderSize = 2.0f;


	// WinAPIの初期化
	bool winOK = ImGui_ImplWin32_Init(hWnd);

	// DirectX11の初期化
	bool dxOK = false;

	if (winOK)
	{
		dxOK = ImGui_ImplDX11_Init(
			GetDevice(),
			GetContext()
		);
	}

	// 初期化に失敗した場合
	if (!winOK || !dxOK)
	{
		if (dxOK)
		{
			ImGui_ImplDX11_Shutdown();
		}

		if (winOK)
		{
			ImGui_ImplWin32_Shutdown();
		}

		DestroyContext();

		MessageBoxA(
			hWnd,
			"DebugEditor initialization failed.",
			"DebugEditor Error",
			MB_OK | MB_ICONERROR
		);

		return false;
	}
	s_initialized = true;

	return true;
}

void DebugEditor::Shutdown()
{
	if (!s_initialized) return;

	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	DestroyContext();

	s_initialized = false;
}

void DebugEditor::BeginFrame()
{
	if (!s_initialized) return;

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();

	NewFrame();
}

void DebugEditor::Render()
{
	if (!s_initialized) return;

	ImGui::Render();

	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

bool DebugEditor::HandleWndProc(
	HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (!s_initialized) return false;

	if (GetCurrentContext() == nullptr) return false;

	return ImGui_ImplWin32_WndProcHandler(
		hWnd, message, wParam, lParam) != 0;
}

bool DebugEditor::WantsMouse()
{
	if (!s_initialized) return false;

	return GetIO().WantCaptureMouse;
}

bool DebugEditor::TryGetSceneClick(
	float& mouseX,
	float& mouseY,
	float& viewportWidth,
	float& viewportHeight)
{
	if (!s_initialized) return false;

	const ImGuiIO& io = GetIO();

	// ImGuiがマウスを使用中なら無視
	if (io.WantCaptureMouse) return false;

	// 左クリックした瞬間だけ判定
	if (!IsMouseClicked(ImGuiMouseButton_Left)) return false;

	// マウス位置
	mouseX = io.MousePos.x;
	mouseY = io.MousePos.y;

	// 現在の画面サイズ
	viewportWidth = io.DisplaySize.x;
	viewportHeight = io.DisplaySize.y;

	if (viewportWidth <= 0.0f || viewportHeight <= 0.0f) return false;

	// 画面外なら無視
	if (mouseX < 0.0f ||
		mouseY < 0.0f ||
		mouseX >= viewportWidth ||
		mouseY >= viewportHeight)
	{
		return false;
	}

	return true;
}

bool DebugEditor::WantsKeyboard()
{
	if (!s_initialized) return false;

	return GetIO().WantCaptureKeyboard;
}

EditorCommand DebugEditor::DrawScene(
	vector<unique_ptr<GameObject>>& objects,
	bool& showGrid)
{
	EditorCommand command;

	if (!s_initialized) return command;

	const int objectCount = static_cast<int>(objects.size());

	// 選択番号が無効になった場合は修正
	if (s_selectedObjectIndex < 0 ||
		s_selectedObjectIndex >= objectCount)
	{
		s_selectedObjectIndex =
			objectCount > 0 ? 0 : -1;
	}

	//========================================
	// Hierarchy
	//========================================
	const EditorPanelLayout layout =
		CalculateEditorLayout();

	//========================================
	// Hierarchyを左側へ配置
	//========================================
	SetNextWindowPos(
		layout.position,
		ImGuiCond_Always
	);

	SetNextWindowSize(
		ImVec2(
			layout.hierarchyWidth,
			layout.size.y
		),
		ImGuiCond_Always
	);

	Begin(
		"Hierarchy",
		nullptr,
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize);

	//====================
	// Create
	//====================
	if (Button("+ 生成"))
	{
		OpenPopup("CreateObjectPopup");
	}

	if (BeginPopup("CreateObjectPopup"))
	{
		if (MenuItem("Ground"))
		{
			command.action = EditorAction::CreateGround;
		}

		if (MenuItem("Goal"))
		{
			command.action = EditorAction::CreateGoal;
		}

		Separator();

		//====================
		// 基本図形
		//====================
		if (MenuItem("Box"))
		{
			command.action = EditorAction::CreateBox;
		}

		if (MenuItem("Cylinder"))
		{
			command.action = EditorAction::CreateCylinder;
		}

		if (MenuItem("Sphere"))
		{
			command.action = EditorAction::CreateSphere;
		}

		Separator();

		//====================
		// FBXモデルを生成
		//====================
		if (BeginMenu("FBX Models"))
		{
			// Assets/Modelからファイル一覧を取得
			vector<string> files = FindFbxFiles();

			if (files.empty())
			{
				TextDisabled(
					"No FBX files found."
				);
			}

			// 見つかったFBXをメニューに表示
			for (const string& path : files)
			{
				// メニューにはファイル名だけを表示
				size_t pos = path.find_last_of("/\\");

				string fileName =
					(pos == string::npos)
					? path
					: path.substr(pos + 1);

				if (MenuItem(fileName.c_str()))
				{
					command.action = EditorAction::CreateFbx;

					command.modelPath = path;
				}
			}

			ImGui::EndMenu();
		}

		EndPopup();
	}

	//====================
	// Duplicate
	//====================
	SameLine();

	bool hasSelection =
		s_selectedObjectIndex >= 0 &&
		s_selectedObjectIndex < objectCount;

	if (Button("複製") && hasSelection)
	{
		command.action = EditorAction::Duplicate;
		command.objectIndex = s_selectedObjectIndex;
	}

	//====================
	// Delete
	//====================
	SameLine();

	if (Button("削除") && hasSelection)
	{
		command.action = EditorAction::Delete;
		command.objectIndex = s_selectedObjectIndex;
	}

	Separator();

	//====================
	// Rename用の変数
	//====================
	// 名前を編集中のオブジェクト番号
	static int renamingIndex = -1;

	// 編集中の名前を保存する文字配列
	static char renameBuffer[256] = {};

	// 入力欄へフォーカスを移すかどうか
	static bool requestFocus = false;

	// 名前のエラー表示
	static bool renameError = false;

	//========================================
	// Scene Save / Load
	//========================================
	Separator();

	// 保存ボタン
	if (Button("シーンを保存"))
	{
		command.action = EditorAction::SaveScene;
	}

	SameLine();

	// 読み込みボタン
	if (Button("シーンを読み込む"))
	{
		command.action = EditorAction::LoadScene;
	}

	// 現在使用しているファイル
	TextDisabled(
		"Assets/Stages/Stage01.json"
	);

	Separator();

	//========================================
	// Hierarchy一覧
	//========================================
	for (int i = 0; i < objectCount; i++)
	{
		GameObject* pObject = objects[i].get();

		if (pObject == nullptr) continue;

		PushID(pObject);

		//====================
		// 名前を編集中の場合
		//====================
		if (renamingIndex == i)
		{
			// 初回だけ入力欄にフォーカスする
			if (requestFocus)
			{
				SetKeyboardFocusHere();
				requestFocus = false;
			}

			// Enterが押されたらtrue
			bool confirmed = InputText(
				"名前変更",
				renameBuffer,
				sizeof(renameBuffer),
				ImGuiInputTextFlags_EnterReturnsTrue
			);

			// Escでキャンセル
			if (ImGui::IsKeyPressed(ImGuiKey_Escape))
			{
				renamingIndex = -1;
				renameError = false;
			}
			else if (confirmed)
			{
				string newName = renameBuffer;

				bool duplicate = false;

				// 同じ名前のオブジェクトがあるか確認
				for (int j = 0; j < objectCount; j++)
				{
					if (j == i || !objects[j]) continue;

					if (objects[j]->GetName() == newName)
					{
						duplicate = true;
						break;
					}
				}

				// 空文字・重複名は設定しない
				if (!newName.empty() && !duplicate)
				{
					pObject->SetName(newName);

					renamingIndex = -1;
					renameError = false;
				}
				else
				{
					renameError = true;
				}
			}

			if (renameError && renamingIndex == i)
			{
				TextDisabled("名前が設定されていない、\nもしくは重複しています");
			}
		}
		//====================
		// 通常表示の場合
		//====================
		else
		{
			bool selected = s_selectedObjectIndex == i;

			if (Selectable(
				pObject->GetName().c_str(),
				selected))
			{
				s_selectedObjectIndex = i;
			}

			// ダブルクリックで名前編集開始
			if (IsItemHovered() &&
				IsMouseDoubleClicked(
					ImGuiMouseButton_Left))
			{
				s_selectedObjectIndex = i;
				renamingIndex = i;

				// 現在の名前を入力欄にコピー
				snprintf(
					renameBuffer,
					sizeof(renameBuffer),
					"%s",
					pObject->GetName().c_str()
				);

				requestFocus = true;
				renameError = false;
			}
		}

		PopID();
	}
	End();




	//========================================
	// Inspector
	//========================================
	// 
	//========================================
	// Inspectorを右上へ配置
	//========================================
	SetNextWindowPos(
		ImVec2(
			layout.position.x +
			layout.size.x -
			layout.inspectorWidth,
			layout.position.y
		),
		ImGuiCond_Always
	);

	SetNextWindowSize(
		ImVec2(
			layout.inspectorWidth,
			layout.inspectorHeight
		),
		ImGuiCond_Always
	);

	Begin(
		"Inspector",
		nullptr,
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize
	);


	//========================================
	// デバッググリッド
	//========================================
	Checkbox("グリッドを有効", &showGrid);

	Separator();

	if (s_selectedObjectIndex >= 0 &&
		s_selectedObjectIndex < objectCount)
	{
		GameObject* pObject =
			objects[s_selectedObjectIndex].get();

		if (pObject != nullptr)
		{
			// オブジェクト名
			Text(
				"オブジェクト名: %s",
				pObject->GetName().c_str()
			);

			Separator();

			// Shape・FBX・Color
			DrawObjectAppearance(pObject);

			// Position・Rotation・Scale
			DrawObjectTransform(pObject);

			// Component一覧・パラメータ・追加・削除
			DrawObjectComponents(pObject);
		}	
	}
	else
	{
		Text("オブジェクトが選択されていません");
	}

	End();

	return command;
}

// 選択番号を変更する
void DebugEditor::SetSelectedObjectIndex(int index)
{
	s_selectedObjectIndex = index;
}

void DebugEditor::DrawPlayCamera(
	CameraPlay& playCamera,
	Camera& editCamera)
{
	if (!s_initialized) return;

	const EditorPanelLayout layout =
		CalculateEditorLayout();

	//====================================
	// Play Cameraを右下へ配置
	//====================================
	SetNextWindowPos(
		ImVec2(
			layout.position.x +
			layout.size.x -
			layout.inspectorWidth,
			layout.position.y +
			layout.inspectorHeight
		),
		ImGuiCond_Always
	);

	SetNextWindowSize(
		ImVec2(
			layout.inspectorWidth,
			layout.size.y -
			layout.inspectorHeight
		),
		ImGuiCond_Always
	);

	Begin(
		"Play Camera",
		nullptr,
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize
	);

	//========================================
	// 現在のカメラ情報を取得
	//========================================
	XMFLOAT3 position = playCamera.GetPosition();
	XMFLOAT3 look = playCamera.GetLook();

	//========================================
	// Positionを編集
	//========================================
	bool positionChanged = DragFloat3(
		"座標",
		&position.x,
		0.1f
	);

	//========================================
	// Lookを編集
	//========================================
	bool lookChanged = DragFloat3(
		"注視点",
		&look.x,
		0.1f
	);

	//========================================
	// 編集された場合だけ反映
	//========================================
	if (positionChanged || lookChanged)
	{
		float dx = look.x - position.x;
		float dz = look.z - position.z;

		// 真上・真下を向くとビュー行列が
		// 不正になるため、この場合は反映しない
		if (dx * dx + dz * dz > 0.0001f)
		{
			playCamera.SetPose(position, look);
		}
		else
		{
			TextDisabled(
				"Camera direction is invaild."
			);
		}
	}

	Separator();

	//========================================
	// Editカメラの視点をコピー
	//========================================
	if (Button("※現在のカメラ位置をコピー"))
	{
		playCamera.SetPose(
			editCamera.GetPos(),
			editCamera.GetLook()
		);
	}

	TextDisabled(
		"F1を押してゲーム画面に移行"
	);

	End();
}
#endif