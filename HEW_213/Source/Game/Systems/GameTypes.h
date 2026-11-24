#pragma once

#include <vector>
#include <string>
#include <cstdint>

class GameObject;

// XZ平面。ｚは画面ＹではなくワールドＺ
struct BoardVec2
{
	float x = 0.0f;
	float z = 0.0f;
};
enum class GameRole // 役職
{
	Player,
	Enemy,
	Boss,
	Wall,
};
enum class ColliderShape // 形状の当たり判定
{
	Sphere,
	Box
};
struct ColliderConfig // 当たり判定の設定
{
	GameRole role = GameRole::Wall;
	ColliderShape shape = ColliderShape::Box;
	float radius = 0.5f;
	BoardVec2 halfExtent = { 0.5f,0.5f }; // 横・奥の半サイズ
	float restitution = 1.0f;
};
struct PointerFrame
{
	bool valid = false; // フォーカス喪失・平面交差失敗時 : false
	bool pressed = false;
	bool held = false;
	bool released = false;
	BoardVec2 point;
};
struct AimState
{
	bool dragging = false;
	bool released = false;
	BoardVec2 anchor;
	BoardVec2 point;
};
struct Contact
{
	GameObject* objectA = nullptr; // 非所有・この物理ステップ内のみ
	GameObject* objectB = nullptr;
	BoardVec2 normal; // B→A、単位長
	float penetration = 0.0f; // 重なった距離
};
struct CollisionNotice
{
	GameObject* attacker = nullptr;
	GameObject* target = nullptr;
	int reflectionCount = 0; // 反射回数
	int attackLevel = 0; // 強化値
};
struct HudInput
{
	int remainingShots = 0;
	int shotsUntilNextWave = 0;
	int bossHP = 0;
	int bossMaxHP = 0;
	std::vector<float> enemyHPRatios; // SceneGameの該当フレームの敵順
	AimState aim;
	BoardVec2 playerPosition;
};
struct HudColor
{
	float r = 1, g = 1, b = 1, a = 1;
};
struct HudView
{
	std::string shotsText, shotsUntilNextWaveText;
	float bossRatio = 0.0f;
	std::vector<float> enemyRatios;
	std::vector<HudColor> enemyColors; // 同じ敵順、描画はリード側
	bool showArrow = false;
	BoardVec2 arrowStart;
	BoardVec2 arrowEnd;
};