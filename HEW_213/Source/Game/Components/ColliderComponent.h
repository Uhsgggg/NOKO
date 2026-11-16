#pragma once

#include "Engine/Scene/Component.h"
#include "Game/Systems/GameTypes.h"

class ColliderComponent : public Component
{
public:
	explicit ColliderComponent(GameObject* owner);

	const char* GetTypeId() const override
	{
		return "Collider";
	}

	const char* GetTypeName() const override
	{
		return "Collider";
	}

	std::unique_ptr<Component> Clone(
		GameObject* pNewOwner) const override;

#ifdef _DEBUG
	void DrawDebugInspector() override;
#endif

	void Configure(const ColliderConfig& config);

	const ColliderConfig& GetConfig() const;

private:
	ColliderConfig m_config;
};