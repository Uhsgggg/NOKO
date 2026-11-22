#include "DamageFormula.h"
#include <cmath>
#include <climits>

using namespace std;

int DamageFormula::Calculate(float baseDamage,
							 float reflectionMultiplier,
							 int reflectionCount)
{
	// 異常な値ならダメージを与えない
	if (!isfinite(baseDamage) || !isfinite(reflectionMultiplier))
	{
		return 0;
	}
	// ダメージがマイナスなら 0 を返す
	if (baseDamage <= 0 || reflectionMultiplier < 0)
	{
		return 0;
	}
	// 反射回数がマイナスにならないようにする
	if (reflectionCount < 0)
	{
		reflectionCount = 0;
	}

	// ダメージの計算
	float calculateDamage = 
		2.0f * baseDamage * 
		(pow(reflectionMultiplier, reflectionCount + 1) - 1.0f);
	// 最大ダメージが2,147,483,647ダメージを超えないようにする
	if (static_cast<double>(calculateDamage) >= INT_MAX)
	{
		return INT_MAX;
	}
	return static_cast<int>(calculateDamage);
}