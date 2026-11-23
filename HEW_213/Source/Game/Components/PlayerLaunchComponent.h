#pragma once

#include "Engine/Scene/Component.h"
#include "Game/Systems/GameTypes.h"

class PlayerLaunchComponent : public Component
{
public:
	explicit PlayerLaunchComponent(GameObject* pOwner);

	const char* GetTypeId() const override
	{
		return "PlayerLaunch";
	}

	const char* GetTypeName() const override
	{
		return "ÉvÉåÉCÉÑÅ[î≠éÀ";
	}

	std::unique_ptr<Component> Clone(
		GameObject* pNewOwner) const override;

#ifdef _DEBUG
	void DrawDebugInspector() override;
#endif

	bool TryBuildVelocity(const AimState& aim, BoardVec2& out) const;
	void Configure(float minDrag, float maxDrag, float maxSpeed);
	float GetMinDrag() const;
	float GetMaxDrag() const;
	float GetMaxSpeed() const;

private:
	float m_minDrag = 0.1f;
	float m_maxDrag = 3.0f;
	float m_maxSpeed = 10.0f;
};