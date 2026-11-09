#include "Main.h"
#include "Defines.h"
#include <memory>
#include <cmath>

#ifdef _DEBUG
#include "Source/Engine/Debug/DebugEditor.h"
#endif

#include "Source/Engine/Graphics/DirectX/DirectX.h"
#include "Source/Engine/Graphics/DirectX/ShaderList.h"
#include "Source/Engine/Graphics/Geometory.h"
#include "Source/Engine/Graphics/Sprite.h"
#include "Source/Engine/Input/Input.h"
#include "Source/Game/Scenes/SceneGame.h"

//--- グローバル変数
Scene* g_pScene = nullptr;

HRESULT Init(HWND hWnd, UINT width, UINT height)
{
	HRESULT hr;
	// DirectX初期化
	hr = InitDirectX(hWnd, width, height, false);
	if (FAILED(hr)) { return hr; }

#ifdef _DEBUG
	if (!DebugEditor::Initialize(hWnd)) return E_FAIL;
#endif

	// 他機能初期化
	Geometory::Init();
	Sprite::Init();
	InitInput();
	ShaderList::Init();

	// シーン
	g_pScene = new SceneGame();
	g_pScene->OnResize(width, height);

	return hr;
}

void Uninit()
{
	delete g_pScene;
	g_pScene = nullptr;

#ifdef _DEBUG
	DebugEditor::Shutdown();
#endif

	ShaderList::Uninit();
	UninitInput();
	Sprite::Uninit();
	Geometory::Uninit();
	UninitDirectX();
}

void Update()
{
	UpdateInput();

	if (g_pScene)
	{
		g_pScene->RootUpdate();
	}
}

void Draw()
{
	// DirectX11の描画開始
	BeginDrawDirectX();

#ifdef _DEBUG
	// ImGuiのフレームを開始
	DebugEditor::BeginFrame();
#endif

	// ゲームシーンを描画
	if (g_pScene)
	{
		g_pScene->RootDraw();
	}

	//==================================================
	// ImGui : Debug Panel
	//==================================================
#ifdef _DEBUG
	// ImGuiの描画を確定して出力
	DebugEditor::Render();
#endif
	// Begin     : ウィンドウ開始
	// End       : ウィンドウ終了
	// Text      : 文字表示
	// Separator : 区切り線
	// このFPSはImGui側で計算したもの。
	// ゲーム更新FPSとは一致しない場合がある
	
	// Button : クリックされたフレームのみtrueを返す
	// Checkbox : boolを受け取り、チェックで値を書き換える
	// SameLine : 次の項目を同じ行に並べられる
	// ここでは見た目だけ。実際のグリッド切り替えは後で行う
	//static bool showGrid = true;
	//static int resetCount = 0;
	//if (ImGui::Button("Reset"))
	//{
	//	resetCount++;
	//}
	//ImGui::SameLine();
	//ImGui::Text("%d times", resetCount);
	//
	//ImGui::Checkbox("Show Grid", &showGrid);
	//if (showGrid)
	//{
	//	ImGui::Text("Grid is enabled");
	//}
	//ImGui::End();
	
	// SliderFloat : 決められた範囲の調整に向く
	// DragFloat   : 数値をドラッグして変更
	// InputFloat  : 直接数字を打ち込める
	// &speed      : speed のアドレス。ImGuiが元の変数へ書き戻す
	//ImGui::Begin("Sliders");
	//static float speed = 5.0f;
	//static float bounce = 0.85f;
	//static float cameraDist = 10.0f;
	//ImGui::SliderFloat("Speed", &speed, 0.0f, 20.0f);
	//ImGui::DragFloat("Bounce", &bounce, 0.01f, 0.0f, 1.0f);
	//ImGui::InputFloat("Camera Distance", &cameraDist);
	//ImGui::Text("Speed = %.2f", speed);
	//ImGui::End();
	
	// ※上のやつの続き
	// ・ローカル変数は毎回初期化
	// 通常のfloatはDrawUI()を呼ぶたびに再生成される。
	// スライダーの変更が保持されない。
	// 
	// ・本番はメンバ変数にする
	// staticは練習用。実際のゲームではSceneGameのメンバや
	// ParameterManagerなどに状態を持たせる。
	//void DrawUI()
	//{
	//	float speed = 5.0f;
	//	ImGui::SliderFloat("Speed", &speed, 0, 20);
	//}
	//
	//void DrawUI()
	//{
	//	static float speed = 5.0f;
	//	ImGui::SliderFloat("Speed", &speed, 0, 20);
	//}
	
	// ・UnityのInspectorらしく整理（CollapsingHeader）
	// Position/Rotation/Scale : float[3]を想定
	// CollapsingHeader : 展開・折り込みができる
	// UIをたくさん作るほど分類が重要
	//ImGui::Begin("Stage Inspector");
	//static float pos[3] = {};
	//static float rot[3] = {};
	//static float scale[3] = {};
	//if (ImGui::CollapsingHeader("Transform"))
	//{
	//	ImGui::DragFloat3("Position", pos, 0.1f);
	//	ImGui::DragFloat3("Rotation", rot, 0.1f);
	//	ImGui::DragFloat3("Scale", scale, 0.1f);
	//}
	//if (ImGui::CollapsingHeader("Debug"))
	//{
	//	ImGui::Checkbox("Show Grid", &showGrid);
	//}
	//ImGui::End();
	//==================================================

	// DirectX11の描画終了
	EndDrawDirectX();
}

HRESULT ResizeGame(UINT width, UINT height)
{
	if (width == 0 || height == 0) return S_FALSE;

	HRESULT hr = ResizeDirectX(width, height);

	if (FAILED(hr)) return hr;

	if (g_pScene)
	{
		g_pScene->OnResize(width, height);
	}

	return S_OK;
}
