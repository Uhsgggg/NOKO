#pragma once

#include "Engine/Scene/Component.h"
#include "Game/Systems/GameTypes.h"
//
class BallMotionComponent : public Component
{
public:
	explicit BallMotionComponent(GameObject* owner);
	
	const char* GetTypeId() const override
	{
		return "BallMotion";
	}

	const char* GetTypeName() const override
	{
		return "î≠éÀà⁄ìÆ";
	}

	std::unique_ptr<Component> Clone(
		GameObject* pNewOwner) const override;

#ifdef _DEBUG
	void DrawDebugInspector() override;
#endif

	void Update(float deltaTime) override;
	void SetVelocity(BoardVec2 velocity);
	BoardVec2 GetVelocity() const;
	bool IsMoving() const;
	void Stop();
	void Configure(float deceleration, float stopSpeed, float maxSpeed);
	float GetDeceleration() const;
	float GetStopSpeed() const;
	float GetMaxSpeed() const;

private:
	// åªç›ÇÃë¨ìx
	BoardVec2 m_velocity;

	float m_deceleration = 4.0f;
	float m_stopSpeed = 0.05f;
	float m_maxSpeed = 10.0f;
};