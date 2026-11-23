#pragma once

#include "Engine/Scene/Component.h"
#include "Game/Systems/GameTypes.h"

class ColliderComponent : public Component
{
public:
	explicit ColliderComponent(GameObject *pOwner);

	const char *GetTypeId() const override
	{
		return "Collider";
	}

	const char *GetTypeName() const override
	{
		return "“–‚½‚è”»’è";
	}

	std::unique_ptr<Component> Clone(
		GameObject *pNewOwner) const override;

#ifdef _DEBUG
	void DrawDebugInspector() override;
	void DrawDebug() const; 
	void NotifyDebugHit();
	void UpdateDebugHit();
	bool IsDebugHit()const;
#endif

	void Configure(const ColliderConfig& config);
	const ColliderConfig& GetConfig() const;

private:
	ColliderConfig m_config;
#ifdef _DEBUG
	int m_nDebugHitFrames = 0;
#endif
};