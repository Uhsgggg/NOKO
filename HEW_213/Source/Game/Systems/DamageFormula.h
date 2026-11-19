#pragma once

class DamageFormula
{
public:
	static int Calculate(
		float baseDamage, float multiplier, int reflectionCount);
};