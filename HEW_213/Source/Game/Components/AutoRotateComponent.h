#pragma once

#include "Engine/Scene/Component.h"

// オブジェクトを自動回転させるコンポーネント
class AutoRotateComponent : public Component
{
public:
	AutoRotateComponent(
		GameObject* pOwner, float speedX, float speedY, float speedZ);

	~AutoRotateComponent() override = default;

	// 毎フレームの更新
	void Update(float deltaTime) override;

	const char* GetTypeName() const override
	{
		return "Auto Rotate";
	}

	// 自動回転コンポーネントを複製
	std::unique_ptr<Component> Clone(
		GameObject* pNewOwner) const override;

	// 回転速度を読み取り専用で取得
	const float* GetRotationSpeed() const
	{
		return m_rotationSpeed;
	}

#ifdef _DEBUG
	void DrawDebugInspector() override;
#endif

private:
	// 1秒間に何度回転するか
	float m_rotationSpeed[3];
};

