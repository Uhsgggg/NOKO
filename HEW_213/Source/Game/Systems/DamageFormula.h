#pragma once

class DamageFormula
{
public:
	static int Calculate(float baseDamage,
						 float bonusRatePerReflection,
						 int reflectionCount);
};