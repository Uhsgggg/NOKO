#pragma once

#include "Engine/Scene/Component.h"
#include "Game/Systems/GameTypes.h"

class HealthComponent : public Component
{
public:
	explicit HealthComponent(GameObject* owner);

	const char* GetTypeId() const override
	{
		return "Health";
	}

	const char* GetTypeName() const override
	{
		return "Health";
	}

	std::unique_ptr<Component> Clone(
		GameObject* pNewOwner) const override;

#ifdef _DEBUG
	void DrawDebugInspector() override;
#endif

	void Reset(int maxHP);
	int ApplyDamage(int amount); // é¿ç€Ç…å∏ÇÈHP
	int GetHP() const;
	int GetMaxHP() const;
	bool IsDead() const;

private:
	int m_hp = 10;
	int m_maxHP = 10;
};