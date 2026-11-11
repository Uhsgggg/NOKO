#include <windows.h>

#include "Defines.h"
#include "Main.h"

#include <stdio.h>
#include <crtdbg.h>

#ifdef _DEBUG
#include "Engine/Debug/DebugEditor.h"
#endif

// timeGetTime周りの使用
#pragma comment(lib, "winmm.lib")

namespace
{
	bool g_resizePending = false;

	UINT g_pendingWidth = SCREEN_WIDTH;
	UINT g_pendingHeight = SCREEN_HEIGHT;
}

//--- プロトタイプ宣言
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

// エントリポイント
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	// ウィンドウ作成前にDPI対応を有効化
	SetProcessDpiAwarenessContext(
		DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2
	);

	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	//--- 変数宣言
	WNDCLASSEX wcex;
	HWND hWnd;
	MSG message;

	// ウィンドクラス情報の設定
	ZeroMemory(&wcex, sizeof(wcex));
	wcex.hInstance = hInstance;
	wcex.lpszClassName = "Class Name";
	wcex.lpfnWndProc = WndProc;
	wcex.style = CS_CLASSDC | CS_DBLCLKS;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wcex.hIconSm = wcex.hIcon;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);

	// ウィンドウクラス情報の登録
	if (!RegisterClassEx(&wcex))
	{
		MessageBox(NULL, "Failed to RegisterClassEx", "Error", MB_OK);
		return 0;
	}

	// ウィンドウの作成
	RECT rect = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
	DWORD style = WS_OVERLAPPEDWINDOW;
	DWORD exStyle = 0;
	AdjustWindowRectEx(&rect, style, FALSE, exStyle);
	hWnd = CreateWindowEx(
		exStyle, wcex.lpszClassName,
		APP_TITLE, style,
		CW_USEDEFAULT, CW_USEDEFAULT,
		rect.right - rect.left, rect.bottom - rect.top,
		HWND_DESKTOP,
		NULL, hInstance, NULL
	);

	// ウィンドウの表示
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	// 初期化処理
	if (FAILED(Init(hWnd, SCREEN_WIDTH, SCREEN_HEIGHT)))
	{
		Uninit();
		UnregisterClass(wcex.lpszClassName, hInstance);
		return 0;
	}

	// 初期化完了時の実際のクライアント領域を確認
	RECT clientRect = {};

	GetClientRect(hWnd, &clientRect);

	g_pendingWidth =
		clientRect.right - clientRect.left;
	g_pendingHeight =
		clientRect.bottom - clientRect.top;

	g_resizePending = true;

	//--- FPS制御
	timeBeginPeriod(1);
	DWORD countStartTime = timeGetTime();
	DWORD preExecTime = countStartTime;
#ifdef _DEBUG
	// 高精度タイマーの周波数
	LARGE_INTEGER performanceFrequency = {};
	QueryPerformanceFrequency(&performanceFrequency);

	// FPS計測を開始した時刻
	LARGE_INTEGER fpsStartTime = {};
	QueryPerformanceCounter(&fpsStartTime);

	// 計測期間内に描画したフレーム数
	unsigned int measuredFrames = 0;

	// Update + Drawにかかった合計時間
	double totalWorkMs = 0.0f;
#endif

	//--- ウィンドウの管理
	while (1)
	{
		if (PeekMessage(&message, NULL, 0, 0, PM_NOREMOVE))
		{
			if (!GetMessage(&message, NULL, 0, 0))
			{
				break;
			}
			else
			{
				TranslateMessage(&message);
				DispatchMessage(&message);
			}
		}
		else
		{
			if (IsIconic(hWnd))
			{
				WaitMessage();
				continue;
			}

			if (g_resizePending)
			{
				const UINT newWidth = g_pendingWidth;
				const UINT newHeight = g_pendingHeight;
				
				g_resizePending = false;

				HRESULT hr = ResizeGame(newWidth, newHeight);

				if (FAILED(hr))
				{
					char errorText[256];

					sprintf_s(
						errorText,
						"Failed to resize DirectX.\nHRESULT: 0x%08X",
						static_cast<unsigned int>(hr)
					);

					MessageBoxA(
						hWnd,
						errorText,
						"Resize Error",
						MB_OK | MB_ICONERROR
					);

					DestroyWindow(hWnd);
					continue;
				}
			}

			DWORD nowTime = timeGetTime();
			float diff = static_cast<float>(nowTime - preExecTime);
			if (diff >= 1000.0f / fFPS)
			{
#ifdef _DEBUG
				//========================================
				// フレーム処理開始時刻
				//========================================
				LARGE_INTEGER frameStart = {};
				QueryPerformanceCounter(&frameStart);

#endif
				//========================================
				// ゲームを更新・描画
				//========================================
				Update();
				Draw();

#ifdef _DEBUG
				//========================================
				// フレーム処理終了時刻
				//========================================
				LARGE_INTEGER frameEnd = {};
				QueryPerformanceCounter(&frameEnd);

				const double frequency =
					static_cast<double>(
						performanceFrequency.QuadPart
						);

				// 今回のUpdate + Drawの所要時間
				const double workMs =
					static_cast<double>(
						frameEnd.QuadPart -
						frameStart.QuadPart
						) * 1000.0 / frequency;

				totalWorkMs += workMs;

				++measuredFrames;

				//========================================
				// FPS計測期間の長さ
				//========================================
				const double elapsedSeconds =
					static_cast<double>(
						frameEnd.QuadPart -
						fpsStartTime.QuadPart
						) / frequency;

				//========================================
				// 約1秒ごとにタイトルを更新
				//========================================
				if (elapsedSeconds >= 1.0)
				{
					// 実際に描画されたFPS
					const double actualFPS =
						static_cast<double>(
							measuredFrames
							) / elapsedSeconds;

					// 平均処理時間
					const double averageWorkMs =
						totalWorkMs /
						static_cast<double>(
							measuredFrames
							);

					// タイトル文字列を作成
					char windowTitle[160] = {};

					sprintf_s(
						windowTitle,
						sizeof(windowTitle),
						"%s | FPS: %.1f | Work: %.2f ms",
						APP_TITLE,
						actualFPS,
						averageWorkMs
					);

					// Windowsのタイトルバーを更新
					SetWindowTextA(
						hWnd,
						windowTitle
					);

					// 次の計測期間を開始
					measuredFrames = 0;
					totalWorkMs = 0.0;
					fpsStartTime = frameEnd;
				}
#endif

				preExecTime = nowTime;
			}
		}
	}


	// 終了時
	timeEndPeriod(1);
	Uninit();
	UnregisterClass(wcex.lpszClassName, hInstance);

	return 0;
}

// ウィンドウプロシージャ
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_SIZE)
	{
		if (wParam != SIZE_MINIMIZED)
		{
			UINT width = LOWORD(lParam);
			UINT height = HIWORD(lParam);

			if (width > 0 && height > 0)
			{
				g_pendingWidth = width;
				g_pendingHeight = height;
				g_resizePending = true;
			}
		}
	}

#ifdef _DEBUG
	if (DebugEditor::HandleWndProc(
		hWnd, message, wParam, lParam))
	{
		return true;
	}
#endif

	switch (message)
	{
	case WM_GETMINMAXINFO:
	{
		MINMAXINFO* pInfo =
			reinterpret_cast<MINMAXINFO*>(lParam);

		pInfo->ptMinTrackSize.x = 900;
		pInfo->ptMinTrackSize.y = 600;

		return 0;
	}

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

#ifdef _DEBUG
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
		{
			DestroyWindow(hWnd);
			return 0;
		}
		break;
#endif
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}