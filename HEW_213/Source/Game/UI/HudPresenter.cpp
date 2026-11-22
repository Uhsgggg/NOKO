#include "HudPresenter.h"
#include <cmath>

using namespace std;

static float LimitRation(float ratio)
{
	if (!isfinite(ratio) || ratio < 0) return 0;
	if (ratio > 1) return 1;
	return ratio;
}

HudView HudPresenter::Build(const HudInput& input, float maxDrag)
{
	HudView view;
	int shots = input.remainingShots;
	int enemies = input.shotsUntilNextWave;
	if (shots < 0)
	{
		shots = 0;
	}
	if (enemies < 0)
	{
		enemies = 0;
	}
	view.shotsText = to_string(shots);
	view.shotsUntilNextWaveText = to_string(enemies);

	if (input.bossMaxHP > 0)
	{
		float ratio = float(input.bossHP) / input.bossMaxHP;
		view.bossRatio = LimitRation(ratio);
	}

	for (float hp : input.enemyHPRatios)
	{
		const float hpRatio = LimitRation(hp);
		view.enemyRatios.push_back(hpRatio);
		HudColor color;

		if (hpRatio >= 0.5f)
		{// HP 100%～50%：緑→黄
			color.r = (1.0f - hpRatio) * 2.0f;
			color.g = 1.0f;
		}
		else
		{// HP 50%～0%：黄→赤
			color.r = 1.0f;
			color.g = hpRatio * 2.0f;
		}

		color.b = 0.0f;
		color.a = 1.0f;

		view.enemyColors.push_back(color);
	}

	if (input.aim.dragging && maxDrag > 0)
	{
		float x = input.aim.anchor.x - input.aim.point.x;
		float z = input.aim.anchor.z - input.aim.point.z;
		float length = sqrt(x * x + z * z);

		if (isfinite(length) && length > 0)
		{
			if (length > maxDrag)
			{
				x = x / length * maxDrag;
				z = z / length * maxDrag;
			}
			view.arrowStart = input.playerPosition;
			view.arrowEnd.x = view.arrowStart.x + x;
			view.arrowEnd.z = view.arrowStart.z + z;
			view.showArrow = true;	
		}
	}

	return view;
}
