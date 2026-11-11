#pragma once

#include "Engine/Scene/GameObject.h"

class Goal : public GameObject
{
public:
	Goal();
	~Goal() override;

	// Update‚ÍGameObject‚©‚çŒp³‚·‚é

	// •`‰æˆ—
	void Draw() override;	

	// À•W‚Ìİ’è
	void SetPos(float x, float y, float z);

	// Šg‘å—¦‚Ìİ’è
	void SetScale(float x, float y, float z);

	std::unique_ptr<GameObject> Clone() const override;
};

