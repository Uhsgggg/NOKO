//========================================//
// 
// CollisionSystem.cpp
// 
//########################################
// 変数・引数名
// 
// object　　　　衝突する物体
// pCollider　 　物体の当たり判定
// config　　　　物体の当たり判定設定
// position　　　位置
// radius　　　　半径
// contact　　 　衝突情報
// outContact　　衝突判定結果の出力先
// contactNormal 衝突面の法線ベクトル(n)
// isColliding 　衝突しているか
// restitution　 反発係数(e)
// motion　　　　移動コンポーネント
// velocity　　　速度
// 
//########################################
// 
//========================================//
#include "CollisionSystem.h"
#include "Engine/Scene/GameObject.h"
#include "Game/Components/ColliderComponent.h"
#include "Game/Components/BallMotionComponent.h"
#include "Game/Components/ReflectionCounterComponent.h"
#include <cmath>

using namespace std;

//-------------
// 位置を読む
//-------------
// XZ平面上の位置を取得する
static BoardVec2 Position(const GameObject& object)
{
	const Transform& t = object.GetTransform();
	return
	{
		t.position[0], t.position[2]
	};
}

//---------------
// 範囲内に収める
//---------------
// 数値を指定範囲からはみ出さないようにする
static float Limit(float value, float minValue, float maxValue)
{
	if (value < minValue) return minValue;
	if (value > maxValue) return maxValue;
	return value;
}

//---------------
// 内積を計算する
//---------------
// 二つの方向がどれくらい同じ方向を向いているか
static float Dot(BoardVec2 objectA, BoardVec2 objectB)
{
	return objectA.x * objectB.x + objectA.z * objectB.z;
}

//-------------
// 球と球
//-------------
// normalはB→Aへ向かう方向
static bool SphereSphere(BoardVec2 objectA, float radiusA,
						 BoardVec2 objectB, float radiusB,
						 Contact& outContacts)
	
{
	float dx = objectA.x - objectB.x;
	float dz = objectA.z - objectB.z;
	float distance = sqrt(dx * dx + dz * dz);
	if (distance > radiusA + radiusB)
	{
		return false;
	}

	outContacts.normal = { 1, 0 };  // 中心が完全に同じならX方向に逃がす
	if (distance > 0)
	{
		outContacts.normal = { dx / distance, dz / distance };
	}

	outContacts.penetration = radiusA + radiusB - distance;

	return true;
}


//-------------
// 球と箱
//-------------
// 箱はX/Z軸に平行とする
static bool SphereBox(BoardVec2 ball, float radius, 
					  BoardVec2 box, BoardVec2 half, Contact& outContact)
{
	float dx = ball.x - box.x;
	float dz = ball.z - box.z;
	float nearX = Limit(dx, -half.x, half.x);
	float nearZ = Limit(dz, -half.z, half.z);
	float x = dx - nearX;
	float z = dz - nearZ;
	float distance = sqrt(x * x + z * z);
	if (distance > radius) return false;


	if(distance > 0)
	{
		outContact.normal = { x / distance, z / distance };
		outContact.penetration = radius - distance;
		return true;
	}

	// 球の中心が箱の中なら、近い方の面から出す
	float remainingDistanceX = half.x - fabs(dx);
	float remainingDistanceZ = half.z - fabs(dz);
	if (remainingDistanceX <= remainingDistanceZ)
	{
		outContact.normal = {1, 0};
		if (dx < 0)
		{
			outContact.normal.x = -1;
		}
		outContact.penetration = radius + remainingDistanceX;
	}
	else
	{
		outContact.normal = {0, 1};
		if (dz < 0)
		{
			outContact.normal.z = -1;
		}
		outContact.penetration = radius + remainingDistanceZ;
	}

	return true;
}

//---------------------
// 形に応じて判定を選ぶ
//---------------------
bool CollisionSystem::Detect(GameObject& objectA,
							 GameObject& objectB, Contact& outContact)
{
	// 衝突判定結果の出力先
	outContact = {};

	if (&objectA == &objectB ||
		!objectA.IsActive() || !objectB.IsActive())
	{
		return false;
	}

	// objectA/Bの当たり判定
	auto* pColliderA = objectA.GetComponent<ColliderComponent>();
	auto* pColliderB = objectB.GetComponent<ColliderComponent>();

	if (!pColliderA || !pColliderB ||
		!pColliderA->IsEnabled() || !pColliderB->IsEnabled())
	{
		return false;
	}

	// objectA/Bの当たり判定設定
	const ColliderConfig& configA = pColliderA->GetConfig();
	const ColliderConfig& configB = pColliderB->GetConfig();

	BoardVec2 positionA = Position(objectA);
	BoardVec2 positionB = Position(objectB);
	if (!isfinite(positionA.x) || !isfinite(positionA.z) ||
		!isfinite(positionB.x) || !isfinite(positionB.z))
	{
		return false;
	}

	// 衝突しているか
	bool isColliding = false;

	if (configA.shape == ColliderShape::Sphere &&
		configB.shape == ColliderShape::Sphere)
	{
		isColliding = SphereSphere(positionA, configA.radius, positionB, configB.radius, outContact);
	}
	else if (configA.shape == ColliderShape::Sphere &&
		configB.shape == ColliderShape::Box)
	{
		isColliding = SphereBox(positionA, configA.radius, positionB, configB.halfExtent, outContact);
	}
	else if (configA.shape == ColliderShape::Box &&
		configB.shape == ColliderShape::Sphere)
	{
		isColliding = SphereBox(positionB, configB.radius, positionA, configA.halfExtent, outContact);
		outContact.normal.x = -outContact.normal.x;
		outContact.normal.z = -outContact.normal.z;
	}

	if (isColliding)
	{
		outContact.objectA = &objectA;
		outContact.objectB = &objectB;
	}
	return isColliding;
}

//--------
// 壁反射
//--------
// 壁に向かっている速度だけを跳ね返す
BoardVec2 CollisionSystem::Reflect(
	BoardVec2 velocity, BoardVec2 contactNormal, float restitution)
{
	const float velocityAlongNormal = Dot(velocity, contactNormal);
	// 壁に向かっていないなら跳ね返さない
	if (velocityAlongNormal >= 0)
	{
		return velocity;
	}

	velocity.x -=
		(1 + restitution) * velocityAlongNormal * contactNormal.x;
	velocity.z -=
		(1 + restitution) * velocityAlongNormal * contactNormal.z;

	return velocity;
}

//-----------------
// 動く球をみつける
//-----------------
static BallMotionComponent* GetMotion(GameObject& object)
{
	GameRole role = object.GetComponent<ColliderComponent>()->GetConfig().role;
	
	// 壁とボスは動かない
	if (role != GameRole::Player && role != GameRole::Enemy)
	{
		return nullptr;
	}

	// 球以外は動かない
	if (object.GetComponent<ColliderComponent>()
		->GetConfig().shape != ColliderShape::Sphere)
	{
		return nullptr;
	}

	auto* pMotion = object.GetComponent<BallMotionComponent>();
	if (pMotion && pMotion->IsEnabled())
	{
		return pMotion;
	}

	return nullptr;
}

//-----------------
// めり込みを直す
//-----------------
static void PushOut(GameObject& object, BoardVec2 contactNormal, float distance)
{
	object.GetTransform().position[0] += contactNormal.x * distance;
	object.GetTransform().position[2] += contactNormal.z * distance;
}

//--------------
// 球同士の反射
//--------------
// 二つの球は同じ重さとする
// 衝突方向に沿った速度だけ交換する
static void BounceBalls(
	GameObject& objectA, BallMotionComponent& motionA,
	GameObject& objectB, BallMotionComponent& motionB,
	const Contact& contact, float restitution)
{
	BoardVec2 contactNormal = contact.normal;

	PushOut(objectA, contactNormal,  contact.penetration / 2);
	PushOut(objectB, contactNormal, -contact.penetration / 2);

	BoardVec2 velocityA = motionA.GetVelocity();
	BoardVec2 velocityB = motionB.GetVelocity();

	// 近づいていない球はもう反射させない
	float relativeNormalSpeed = 
		Dot(velocityA, contactNormal) - Dot(velocityB, contactNormal);

	if (relativeNormalSpeed >= 0) return;
	
	float velocityChange =
		-(1.0f + restitution) * relativeNormalSpeed / 2.0f;

	velocityA.x += velocityChange * contactNormal.x;
	velocityA.z += velocityChange * contactNormal.z;

	velocityB.x -= velocityChange * contactNormal.x;
	velocityB.z -= velocityChange * contactNormal.z;

	motionA.SetVelocity(velocityA);
	motionB.SetVelocity(velocityB);
}

//----------------
// 固定物との反射
//----------------
static void BounceWall(
	GameObject& ball, BallMotionComponent& motion,
	BoardVec2 contactNormal, float penetrationDepth, float restitution)
{
	PushOut(ball, contactNormal, penetrationDepth);

	BoardVec2 velocity = motion.GetVelocity();
	
	if (Dot(velocity, contactNormal) >= 0) return;

	motion.SetVelocity(
		CollisionSystem::Reflect(velocity, contactNormal, restitution));
	
	auto* pCounter = ball.GetComponent<ReflectionCounterComponent>();
	if (pCounter && pCounter->IsEnabled())
	{
		pCounter->Increment();
	}
}


//-------------------------
// ダメージが入る組み合わせ
//-------------------------
static bool CanDamage(GameRole attacker, GameRole target)
{
	if (attacker == GameRole::Player)
	{
		return target == GameRole::Enemy || target == GameRole::Boss;
	}

	return attacker == GameRole::Enemy && target == GameRole::Boss;
}

//--------------
// 通知を作る
//--------------
static void AddNotice(
	GameObject& objectA, GameObject& objectB,
	vector<CollisionNotice>& outNotices)
{
	CollisionNotice notice;
	notice.attacker = &objectA;
	notice.target = &objectB;
	auto* pCounter = objectA.GetComponent<ReflectionCounterComponent>();
	if (pCounter && pCounter->IsEnabled())
	{
		notice.reflectionCount = pCounter->GetCount();
	}
	outNotices.push_back(notice);
}

//---------------
// 一組を解決する
//---------------
void CollisionSystem::Reset()
{
	m_previousContacts.clear();
}

// 一組の物体を処理する
// Stepから呼ぶだけなのでここだけを先に読める
void CollisionSystem::SolvePair(
	GameObject& objectA, GameObject& objectB,
	vector<CollisionNotice>& outNotices, vector<HitPair>& currentContacts)
{
	Contact contact;

	if (!Detect(objectA, objectB, contact)) return;

	auto* motionA = GetMotion(objectA);
	auto* motionB = GetMotion(objectB);
	if (!motionA && !motionB) return;

	// 前の更新でも接触していたペアにはダメージを送らない
	bool isNewContact = true;
	for (const HitPair& pair : m_previousContacts)
	{
		if (pair.objectA == &objectA && pair.objectB == &objectB)
		{
			isNewContact = false;
		}
	}
	currentContacts.push_back({ &objectA, &objectB });

	const ColliderConfig& configA =
		objectA.GetComponent<ColliderComponent>()->GetConfig();
	const ColliderConfig& configB =
		objectB.GetComponent<ColliderComponent>()->GetConfig();

	float restitution = configA.restitution;
	if (configB.restitution < restitution)
	{
		restitution = configB.restitution;
	}
	
	BoardVec2 velocityA, velocityB;
	if (motionA)
	{
		velocityA = motionA->GetVelocity();
	}
	if (motionB)
	{
		velocityB = motionB->GetVelocity();
	}
	float normalSpeedA = Dot(velocityA, contact.normal);
	float normalSpeedB = Dot(velocityB, contact.normal);

	bool attackA = isNewContact &&
		normalSpeedA - normalSpeedB < 0 &&
		motionA && normalSpeedA < 0 &&
		CanDamage(configA.role, configB.role);

	bool attackB = isNewContact &&
		normalSpeedA - normalSpeedB < 0 &&
		motionB && normalSpeedB > 0 &&
		CanDamage(configB.role, configA.role);

	if (motionA && motionB)
	{
		BounceBalls(objectA, *motionA,
					objectB, *motionB, contact, restitution);
	}
	else if (motionA)
	{
		BounceWall(objectA, *motionA,
				   contact.normal, contact.penetration, restitution);
	}
	else if (motionB)
	{
		BoardVec2 contactNormal =
		{
			-contact.normal.x, -contact.normal.z
		};
		BounceWall(objectB, *motionB,
				   contactNormal, contact.penetration, restitution);
	}
	if (attackA)
	{
		AddNotice(objectA, objectB, outNotices);
	}
	if (attackB)
	{
		AddNotice(objectB, objectA, outNotices);
	}
}

//---------------
// 一組を解決する
//---------------
void CollisionSystem::Step(
	const vector<unique_ptr<GameObject>>& objects,
	vector<CollisionNotice>& outNotices)
{
	outNotices.clear();

	vector<HitPair> currentContacts;
	//iとjの組み合わせを一回ずつ調べる。
	for (size_t i = 0; i < objects.size(); ++i)
	{
		for (size_t j = i + 1; j < objects.size(); ++j)
		{
			if (!objects[i] || !objects[j]) continue;
			SolvePair(*objects[i], *objects[j], outNotices, currentContacts);
		}
	}

	m_previousContacts = currentContacts;
}
