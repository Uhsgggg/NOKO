#pragma once

#include "Engine/Scene/Component.h"
#include "Game/Systems/GameTypes.h"

class PlayerInputComponent : public Component
{
public:
	explicit PlayerInputComponent(GameObject* owner);

	const char* GetTypeId() const override
	{
		return "PlayerInput";
	}

	const char* GetTypeName() const override
	{
		return "Player Input";
	}

	std::unique_ptr<Component> Clone(
		GameObject* pNewOwner) const override;

#ifdef _DEBUG
	void DrawDebugInspector() override;
#endif

	void Sample(const PointerFrame& frame, bool canAim);
	AimState GetAim()const;
	void Cancel();

private:
	AimState m_aim;
};