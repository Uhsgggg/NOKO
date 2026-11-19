#include "HudPresenter.h"
#include <cmath>

static float LimitRation(float ratio)
{
	if (!std::isfinite(ratio) || ratio < 0)
		return 0;
	if (ratio > 1)
		return 1;
	return ratio;
}

HudView HudPresenter::Build(const HudInput& input, float maxDrag)
{
	HudView view;
	int shots = input.remainingShots;
	int enemies = input.nextEnemyCount;
	if (shots < 0)shots = 0;
	if (enemies < 0)enemies = 0;
	view.shotsText = std::to_string(shots);
	view.nextEnemiesText = std::to_string(enemies);

	if (input.bossMaxHP > 0)
	{
		float ratio = float(input.bossHP) / input.bossMaxHP;
		view.bossRatio = LimitRation(ratio);
	}

	for (float hp:input.enemyHPRatios)
	{
		float ratio = LimitRation(hp);
		view.enemyRatios.push_back(ratio);
		HudColor color;
		if (ratio > 0.5f)
			color = { 0,1,0,1 }; //—Î
		else if (ratio > 0.25f)
			color = { 1,1,0,1 }; //‰©
		else
			color = { 1,0,0,1 }; //Ô
		view.enemyColors.push_back(color);
	}

	if (input.aim.dragging && maxDrag > 0)
	{
		float x = input.aim.anchor.x - input.aim.point.x;
		float z = input.aim.anchor.z - input.aim.point.z;
		float length = std::sqrt(x * x + z * z);
		if (std::isfinite(length) && length > 0)
		{
			if (length > maxDrag)
			{
				x = x / length * maxDrag;
				z = z / length * maxDrag;
			}
			view.arrowStart = input.playerPosition;
			view.arrowEnd.x = view.arrowStart.x + x;
			view.arrowEnd.z = view.arrowStart.z + x;
			view.showArrow = true;	
		}
	}
	return view;

}
