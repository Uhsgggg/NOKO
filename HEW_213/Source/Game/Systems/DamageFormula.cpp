#include "DamageFormula.h"
#include <cmath>
#include <climits>

int DamageFormula::Calculate(float baseDamege, float multiplier, int reflectionCount)
{
	if (!std::isfinite(baseDamege) || !std::isfinite(multiplier)) return 0;
	if (baseDamege <= 0 || multiplier < 0) return 0;

	if (reflectionCount < 0)
	{
		reflectionCount = 0;
	}

	float damage = baseDamege * (1.0f + multiplier * reflectionCount);

	//int‚É“ü‚ç‚È‚¢’l‚Íint“à‚É‚Æ‚Ç‚ß‚é
	if (static_cast<double>(damage) >= INT_MAX) return INT_MAX;
	return static_cast<int>(damage);
}