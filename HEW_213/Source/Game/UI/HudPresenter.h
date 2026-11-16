#pragma once

#include "Game/Systems/GameTypes.h"

class HudPresenter
{
public:
	static HudView Build(
		const HudInput& input, float maxDrag);
};