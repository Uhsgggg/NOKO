#pragma once

#include "Engine/Scene/Component.h"

class MoveComponent : public Component
{
public:
	MoveComponent(
		GameObject* pOwner,
		float x, float y, float z);
	~MoveComponent() override = default;

	// 毎フレームの移動処理
	void Update(float deltaTime) override;

	// JSON保存用のID
	const char* GetTypeId() const override
	{
		return "Move";
	}

	// Inspectorの表示名
	const char* GetTypeName() const override
	{
		return "Move";
	}

	// コンポーネント複製
	std::unique_ptr<Component> Clone(
		GameObject* pNewOwner) const override;

	// 移動速度を設定
	void SetVelocity(float x, float y, float z);

	// 移動速度を取得
	const float* GetVelocity() const
	{
		return m_velocity;
	}

#ifdef _DEBUG
	void DrawDebugInspector() override;
#endif

private:
	// ｘｙｚ方向の移動速度
	float m_velocity[3];
};

