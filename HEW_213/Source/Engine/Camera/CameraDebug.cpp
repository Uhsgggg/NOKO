#include "CameraDebug.h"
#include <iostream>
#include <algorithm>
#include <cmath>

#include "Engine/Debug/DebugEditor.h"

using namespace DirectX;
using namespace std;

CameraDebug::CameraDebug()
    : m_yaw(0.0f)
    , m_pitch(-atan2f(3.5f, 5.0f))
    , m_moveSpeed(5.0f)
    , m_fastMultiplier(3.0f)
    , m_mouseSensitivity(0.003f)
    , m_previousMousePos{}
    , m_wasRightDown(false)
{

}

CameraDebug::~CameraDebug()
{

}

// 更新処理
void CameraDebug::Update()
{
    //========================================
    // ImGuiがキーボードを使用中なら
    // カメラの操作を受け付けない
    //========================================
    if (DebugEditor::WantsKeyboard())
    {
        // マウスによる回転状態も解除する
        m_wasRightDown = false;
        return;
    }

    // 初期位置に戻す
    if (IsKeyTrigger('R'))
    {
        Reset();
        return;
    }

    UpdateRotation();
    UpdateMovement();
    UpdateLook();
}

// マウスによる視点回転 
void CameraDebug::UpdateRotation()
{
    bool uiMouse = DebugEditor::WantsMouse();

    // マウスによる回転
    POINT currentMousePos{};

    // 右クリックの状態
    bool rightDown =
        !uiMouse && (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;

    if (GetCursorPos(&currentMousePos))
    {
        // 右クリックを押している間だけ回転
        if (rightDown && m_wasRightDown)
        {
            LONG deltaX =
                currentMousePos.x - m_previousMousePos.x;

            LONG deltaY =
                currentMousePos.y - m_previousMousePos.y;

            m_yaw += deltaX * m_mouseSensitivity;
            m_pitch -= deltaY * m_mouseSensitivity;
        }

        // マウスの状態を保存
        m_previousMousePos = currentMousePos;
        m_wasRightDown = rightDown;
    }
    else
    {
        m_wasRightDown = false;
    }

    // 矢印キーによる回転
    constexpr float rotationSpeed = XMConvertToRadians(90.0f) / FPS;

    if (IsKeyPress(VK_LEFT))  m_yaw   -= rotationSpeed;
    if (IsKeyPress(VK_RIGHT)) m_yaw   += rotationSpeed;
    if (IsKeyPress(VK_UP))    m_pitch += rotationSpeed;
    if (IsKeyPress(VK_DOWN))  m_pitch -= rotationSpeed;

    // 真上・真下を向かないように制限
    constexpr float maxPitch = XMConvertToRadians(89.0f);

    if (m_pitch > maxPitch)  m_pitch = maxPitch;
    else if (m_pitch < -maxPitch) m_pitch = -maxPitch;
}

// キーボードによる移動
void CameraDebug::UpdateMovement()
{
    if (DebugEditor::WantsKeyboard()) return;

    // 60FPS
    constexpr float deltaTime = 1.0f / FPS;
    float speed = m_moveSpeed * deltaTime;

    if (IsKeyPress(VK_SHIFT))
    {
        speed *= m_fastMultiplier;
    }

    // 前進方向
    XMFLOAT3 forward =
    {
        sinf(m_yaw) * cosf(m_pitch),
        sinf(m_pitch),
        cosf(m_yaw) * cosf(m_pitch)
    };

    // 水平方向の右ベクトル
    XMFLOAT3 right =
    {
        cosf(m_yaw),
        0.0f,
        -sinf(m_yaw)
    };

    XMVECTOR direction = XMVectorZero();

    XMVECTOR vForward = XMLoadFloat3(&forward);
    XMVECTOR vRight = XMLoadFloat3(&right);

    if (IsKeyPress('W')) direction += vForward;
    if (IsKeyPress('S')) direction -= vForward;
    if (IsKeyPress('D')) direction += vRight;
    if (IsKeyPress('A')) direction -= vRight;

    if (IsKeyPress(VK_SPACE))
    {
        direction += XMVectorSet(0, 1, 0, 0);
    }
    if (IsKeyPress(VK_CONTROL))
    {
        direction -= XMVectorSet(0, 1, 0, 0);
    }

    // 斜め移動が速くならないようにする
    if (XMVectorGetX(XMVector3LengthSq(direction)) > 0.0f)
    {
        direction = XMVector3Normalize(direction);
        direction *= speed;

        XMVECTOR position = XMLoadFloat3(&m_pos);
        position += direction;

        XMStoreFloat3(&m_pos, position);
    }
}

// 注視点を更新
void CameraDebug::UpdateLook()
{
    XMFLOAT3 forward =
    {
        sinf(m_yaw) * cosf(m_pitch),
        sinf(m_pitch),
        cosf(m_yaw) * cosf(m_pitch)
    };

    m_look.x = m_pos.x + forward.x;
    m_look.y = m_pos.y + forward.y;
    m_look.z = m_pos.z + forward.z;
}

// 初期位置へ戻す
void CameraDebug::Reset()
{
    m_pos = { 0.0f, 3.5f, -5.0f };
    m_yaw = 0.0f;
    m_pitch = -atan2f(3.5f, 5.0f);
    m_wasRightDown = false;
    UpdateLook();
}