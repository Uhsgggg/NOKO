#pragma once
#include "Camera.h"
#include "Engine/Input/Input.h"

// デバッグ用カメラ
class CameraDebug : public Camera
{
public:
    CameraDebug();
    ~CameraDebug();

    // 更新処理
    void Update() final;

private:
    // カメラの向き（ラジアン）
    float m_yaw;   // 左右回転角度
    float m_pitch; // 上下回転角度

    // 操作パラメータ
    float m_moveSpeed;
    float m_fastMultiplier;
    float m_mouseSensitivity;

    // マウスの前フレーム座標
    POINT m_previousMousePos; // マウスの移動量を計算する用

    // 前フレームで右クリックされていたか
    bool m_wasRightDown; // 右クリックを押した瞬間を判定する用

private:
    void UpdateRotation();
    void UpdateMovement();
    void UpdateLook();
    void Reset();
};

