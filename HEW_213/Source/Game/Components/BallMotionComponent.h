#pragma once

#include "Engine/Scene/Component.h"
#include "Game/Systems/GameTypes.h"

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
		return "Ball Motion";
	}

	std::unique_ptr<Component> Clone(
		GameObject* pNewOwner) const override;

#ifdef _DEBUG
	void DrawDebugInspector() override;
#endif

	void Update(float dt) override;
	void SetVelocity(BoardVec2 v);
	BoardVec2 GetVelocity() const;
	bool IsMoving() const;
	void Stop();
	void Configure(float deceleration, float stopSpeed, float maxSpeed);
	float GetDeceleration() const;
	float GetStopSpeed() const;
	float GetMaxSpeed() const;

private:
	BoardVec2 m_velocity;
	float m_deceleration = 4.0f;
	float m_stopSpeed = 0.05f;
	float m_maxSpeed = 10.0f;
};