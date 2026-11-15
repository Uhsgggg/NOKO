#include "Main.h"
#include "Defines.h"
#include <memory>
#include <cmath>

#ifdef _DEBUG
#include "Engine/Debug/DebugEditor.h"
#endif

#include "Engine/Graphics/DirectX/DirectX.h"
#include "Engine/Graphics/DirectX/ShaderList.h"
#include "Engine/Graphics/Geometory.h"
#include "Engine/Graphics/Sprite.h"
#include "Engine/Input/Input.h"
#include "Game/Scenes/SceneGame.h"

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