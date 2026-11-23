#pragma once

#include "Engine/Scene/Component.h"
#include "Game/Systems/GameTypes.h"

class PlayerInputComponent : public Component
{
public:
	explicit PlayerInputComponent(GameObject* pOwner);

	const char* GetTypeId() const override
	{
		return "PlayerInput";
	}

	const char* GetTypeName() const override
	{
		return "プレイヤー入力";
	}

	std::unique_ptr<Component> Clone(
		GameObject* pNewOwner) const override;

#ifdef _DEBUG
	void DrawDebugInspector() override;
#endif

	// マウスの状態を記録する
	void Sample(const PointerFrame& pointerFrame, bool canAim);
	// 現在のドラッグ情報を取得する
	AimState GetAim()const;
	// ドラッグ状態をリセットする
	void Cancel();

private:
	AimState m_aim;
};