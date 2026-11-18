#include "BoardPointer.h"

#include "Engine/Camera/Camera.h"

#include <cmath>

#ifdef _DEBUG
#include "Engine/Debug/DebugEditor.h"
#endif

// WndProcとBuildFrameは同じメインスレッドで実行される。
// マウス1台分の移動量を次のUpdateまで貯める。
namespace
{
	HWND g_hInputWindow = nullptr;
	float g_dx = 0.0f;
	float g_dy = 0.0f;
	bool g_collecting = false;
	bool g_released = false;
	bool g_cancelled = false;
	// kMouseSensitivity … 1カウント当たりのワールド単位
	//						最大ドラッグ3なら、約300カウント。
	const float kMouseSensitivity = 0.01f;
}

bool BoardPointer::RegisterMouse(HWND hWnd)
{
	if (!hWnd) return false;

	RAWINPUTDEVICE device{};
	device.usUsagePage = 0x01;
	device.usUsage = 0x02;		// マウス
	device.dwFlags = 0;			// 通常のマウス入力も残す
	device.hwndTarget = hWnd;

	if (!RegisterRawInputDevices(&device, 1, sizeof(device))) return false;

	g_hInputWindow = hWnd;

	return true;
}

void BoardPointer::ReceiveRawInput(LPARAM lParam)
{
	if (!g_collecting) return;

	RAWINPUT input{};
	UINT size = sizeof(input);
	UINT result = GetRawInputData(
		reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT,
		&input, &size, sizeof(RAWINPUTHEADER));

	if (result == static_cast<UINT>(-1) ||
		result < sizeof(RAWINPUTHEADER) + sizeof(RAWMOUSE) ||
		input.header.dwType != RIM_TYPEMOUSE) return;

	// 絶対座標型の機器の数値は移動量として扱わない
	const RAWMOUSE& mouse = input.data.mouse;
	if (!(mouse.usFlags & MOUSE_MOVE_ABSOLUTE))
	{
		g_dx += static_cast<float>(mouse.lLastX);
		g_dy += static_cast<float>(mouse.lLastY);
	}

	// ボタンを離した後の移動は次の発射に混ぜない
	if (mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_UP)
	{
		g_released = true;
		g_collecting = false;
	}
}

void BoardPointer::CancelRawInput()
{
	g_collecting = false;
	g_released = false;
	g_dx = 0.0f;
	g_dy = 0.0f;
	g_cancelled = true;
}

void BoardPointer::Reset()
{
	m_bDragging = false;
	m_bWaitForRelease = true;
	m_point = {};
	m_right = {};
	CancelRawInput();
	g_cancelled = false;
}

PointerFrame BoardPointer::BuildFrame(Camera& camera)
{
	PointerFrame frame{};
	HWND hWnd = g_hInputWindow;

	// フォーカス喪失・最小化・サイズ変更などは発射せず中断
	if (g_cancelled ||
		!hWnd ||
		GetForegroundWindow() != hWnd ||
		IsIconic(hWnd))
	{
		Reset();
		return frame;
	}

	bool held = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
	if (m_bWaitForRelease)
	{
		if (held) return frame;
		m_bWaitForRelease = false;
	}

	// すでにゲーム側で開始したドラッグは画面内外を問わず続ける
	if (m_bDragging)
	{
		float right = g_dx * kMouseSensitivity;
		float forward = -g_dy * kMouseSensitivity;
		g_dx = 0.0f;
		g_dy = 0.0f;

		// 画面の右・上方向をXZ平面の方向に変換する
		m_point.x += right * m_right.x - forward * m_right.z;
		m_point.z += right * m_right.z + forward * m_right.x;

		frame.valid = true;
		frame.point = m_point;
		if (g_released || !held)
		{
			frame.released = true;
			Reset();
		}
		else
		{
			frame.held = true;
		}

		return frame;
	}

	// ここはドラッグ開始前だけ。UIや画面外からは開始しない
#ifdef _DEBUG
	if (DebugEditor::WantsMouse)
	{
		Reset();
		return frame;
	}
#endif
	POINT mouse{};
	RECT client{};
	if (!GetCursorPos(&mouse) ||
		!ScreenToClient(hWnd, &mouse) ||
		!GetClientRect(hWnd, &client) ||
		!PtInRect(&client, mouse))
	{
		Reset();
		return frame;
	}

	frame.valid = true;
	if (!held) return frame;

	// 転置前のView行列から、カメラの右方向(XZ)を取得する
	DirectX::XMFLOAT4X4 view = camera.GetViewMatrix(false);
	float length = 
		std::sqrt(view._11 * view._11 + view._31 * view._31);
	if (!std::isfinite(length) || length < 0.0001f)
	{
		Reset();
		return PointerFrame{};
	}
	m_right.x = view._11 / length;
	m_right.z = view._31 / length;

	// クリック位置に関係なく、仮想座標(0,0)から開始する
	m_point = {};
	m_bDragging = true;
	g_dx = 0.0f;
	g_dy = 0.0f;
	g_released = false;
	g_collecting = true;
	frame.pressed = true;
	frame.held = true;
	frame.point = m_point;
	return frame;
}