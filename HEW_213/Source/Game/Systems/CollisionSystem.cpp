#include "CollisionSystem.h"
#include "Engine/Scene/GameObject.h"
#include "Game/Components/ColliderComponent.h"
#include "Game/Components/BallMotionComponent.h"
#include "Game/Components/ReflectionCounterComponent.h"
#include <cmath>

//-------------
// 位置を読む
//-------------
//XZ平面上の位置を取得する
static BoardVec2 Position(const GameObject& object)
{
	const Transform& t = object.GetTransform();
	return { t.position[0], t.position[2]};
}

//---------------
// 範囲内に収める
//---------------
//数値を指定範囲からはみ出さないようにする
static float Limit(float value, float low, float high)
{
	if (value < low) return low;
	if (value > high) return high;
	return value;
}

//---------------
// 内積を計算する
//---------------
//二つの方向がどれくらい同じ方向を向いているか
static float Dot(BoardVec2 a, BoardVec2 b)
{
	return a.x * b.x + a.z * b.z;
}

//-------------
// 球と球
//-------------
//normalはB→Aへ向かう方向。
static bool SpherSpher(BoardVec2 a, float ra, BoardVec2 b, float rb, Contact& out)
{
	float dx = a.x - b.x;
	float dz = a.z - b.z;
	float distance = std::sqrt(dx * dx + dz * dz);
	if (distance > ra + rb)
	return false;

	out.normal = { 1, 0 };  //中心が完全に同じならX方向に逃がす。
	if (distance > 0)
		out.normal = { dx / distance, dz / distance };
	out.penetration = ra + rb - distance;
	return true;

}


//-------------
// 球と箱
//-------------
//箱はX/Z軸に平行とする。
static bool SpherBox(BoardVec2 ball, float radius, BoardVec2 box, BoardVec2 half, Contact& out)
{
	float dx = ball.x - box.x;
	float dz = ball.z - box.z;
	float nearX = Limit(dx, -half.x, half.x);
	float nearZ = Limit(dz, -half.z, half.z);
	float x = dx - nearX;
	float z = dz - nearZ;
	float distance = std::sqrt(x * x + z * z);
	if (distance > radius)
		return false;


	if(distance > 0)
	{
		out.normal = { x / distance, z / distance };
		out.penetration = radius - distance;
		return true;
	}

	//球の中心が箱の中なら、近い方の面から出す
	float gapX = half.x - std::fabs(dx);
	float gapZ = half.z - std::fabs(dz);
	if (gapX <= gapZ)
	{
		out.normal = {1, 0};
		if (dx < 0) out.normal.x = -1;
		out.penetration = radius + gapX;
	}
	else
	{
		out.normal = {0, 1};
		if (dz < 0) out.normal.z = -1;
		out.penetration = radius + gapZ;
	}
	return true;
}

//---------------------
// 形に応じて判定を選ぶ
//---------------------
bool CollisionSystem::Detect(GameObject& a, GameObject& b, Contact& out)
{
	out = {};
	if (&a == &b || !a.IsActive() || !b.IsActive())
		return false;
	auto* pA = a.GetComponent<ColliderComponent>();
	auto* pB = b.GetComponent<ColliderComponent>();
	if (!pA || !pB || !pA->IsEnabled() || !pB->IsEnabled())
		return false;
	const ColliderConfig& ca = pA->GetConfig();
	const ColliderConfig& cb = pB->GetConfig();

	BoardVec2 pa = Position(a);
	BoardVec2 pb = Position(b);
	if (!std::isfinite(pa.x) || !std::isfinite(pa.z) || 
		!std::isfinite(pb.x) || !std::isfinite(pb.z))
		return false;
	bool hit = false;
	if (ca.shape == ColliderShape::Sphere && cb.shape == ColliderShape::Sphere)
		hit = SpherSpher(pa, ca.radius, pb, cb.radius, out);
	else if (ca.shape == ColliderShape::Sphere && cb.shape == ColliderShape::Box)
		hit = SpherBox(pa, ca.radius, pb, cb.halfExtent, out);
	else if (ca.shape == ColliderShape::Box && cb.shape == ColliderShape::Sphere)
	{
		hit = SpherBox(pb, cb.radius, pa, ca.halfExtent, out);
		out.normal.x = -out.normal.x;
		out.normal.z = -out.normal.z;
	}

	if (hit)
	{
		out.a = &a;
		out.b = &b;
	
	}
	return hit;
}

//--------
// 壁反射
//--------
//壁に向かっている速度だけを跳ね返す。
BoardVec2 CollisionSystem::Reflect(BoardVec2 v, BoardVec2 n, float e)
{
	float dot = Dot(v, n);
	//壁に向かっていないなら跳ね返さない
	if (dot >= 0)
		return v; 
	v.x - (1 + e) * dot * n.x;
	v.z - (1 + e) * dot * n.z;
	return v;
}

//-----------------
// 動く球をみつける
//-----------------
static BallMotionComponent* GetMotion(GameObject& object)
{
	GameRole role = object.GetComponent<ColliderComponent>()->GetConfig().role;
	if (role != GameRole::Player && role != GameRole::Enemy)
		return nullptr; //壁とボスは動かない
	if(object.GetComponent<ColliderComponent>()->GetConfig().shape != ColliderShape::Sphere)
		return nullptr; //球以外は動かない
	auto* pMotion = object.GetComponent<BallMotionComponent>();
	if (pMotion && pMotion->IsEnabled())
		return pMotion;
	return nullptr;
}

//-----------------
// めり込みを直す
//-----------------
static void PushOut(GameObject& object, BoardVec2 n, float distance)
{
	object.GetTransform().position[0] += n.x * distance;
	object.GetTransform().position[2] += n.z * distance;
}

//--------------
// 球同士の反射
//--------------
//二つの球は同じ重さとする。衝突方向に沿った速度だけ交換する。
static void BounceBalls(GameObject& a, BallMotionComponent& ma,
	GameObject& b, BallMotionComponent& mb,
	const Contact& c, float e)
{
	BoardVec2 n = c.normal;
	PushOut(a, n, c.penetration / 2);
	PushOut(b, n, -c.penetration / 2);
	BoardVec2 va = ma.GetVelocity();
	BoardVec2 vb = mb.GetVelocity();
	//近づいていない球はもう反射させない。
	float speed = Dot(va, n) - Dot(vb, n);
	if (speed >= 0)
		return;
	float change = -(1 + e) * speed / 2;
	va.x += change * n.x;
	va.z += change * n.z;
	ma.SetVelocity(va);
	mb.SetVelocity(vb);
}

//----------------
// 固定物との反射
//----------------
static void BounceWall(GameObject& ball, BallMotionComponent& motion,
	BoardVec2 n, float depth, float e)
{
	PushOut(ball, n, depth);
	BoardVec2 v = motion.GetVelocity();
	if (Dot(v, n) >= 0)
		return;
	motion.SetVelocity(CollisionSystem::Reflect(v, n, e));
	auto* pCounter = ball.GetComponent<ReflectionCounterComponent>();
	if (pCounter && pCounter->IsEnabled())
		pCounter->Increment();
}


//-------------------------
// ダメージが入る組み合わせ
//-------------------------
static bool CanDamage(GameRole attacker, GameRole target)
{
	if (attacker == GameRole::Player)
		return target == GameRole::Enemy || target == GameRole::Boss;
	return attacker == GameRole::Enemy && target == GameRole::Boss;
}

//--------------
// 通知を作る
//--------------
static void AddNotice(GameObject& a, GameObject& b, std::vector<CollisionNotice>& out)
{
	CollisionNotice notice;
	notice.attacker = &a;
	notice.target = &b;
	auto* pCounter = a.GetComponent<ReflectionCounterComponent>();
	if (pCounter && pCounter->IsEnabled())
		notice.reflectionCount = pCounter->GetCount();
	out.push_back(notice);
}

//---------------
// 一組を解決する
//---------------
//ここでは最初にResetを定義します。前回接触の記録をclear()で消します。
//その後、SolvePairで一組の物体を処理します。
void CollisionSystem::Reset()
{
	m_previousContacts.clear();
}

//一組の物体を処理する。Stepから呼ぶだけなのでここだけを先に読める
void CollisionSystem::SolvePair(GameObject& a, GameObject& b,
	std::vector<CollisionNotice>& out,
	std::vector<HitPair>& current)
{
	Contact c;
	if (!Detect(a, b, c))
		return;
	auto* ma = GetMotion(a);
	auto* mb = GetMotion(b);
	if (!ma && !mb)
		return;

	//前の更新でも接触していたペアにはダメージを送らない。
	bool fresh = true;
	for (const HitPair& pair : m_previousContacts)
	{
		if (pair.a == &a && pair.b == &b)
			fresh = false;
	}
	current.push_back({ &a, &b });

	const ColliderConfig& ca = a.GetComponent<ColliderComponent>()->GetConfig();
	const ColliderConfig& cb = b.GetComponent<ColliderComponent>()->GetConfig();
	float e = ca.restitution;
	if (cb.restitution < e) e = cb.restitution;
	BoardVec2 va, vb;
	if (ma) va = ma->GetVelocity();
	if (mb) vb = mb->GetVelocity();
	float da = Dot(va, c.normal);
	float db = Dot(vb, c.normal);
	bool attackA = fresh && da - db < 0 && ma && da < 0 && CanDamage(ca.role, cb.role);
	bool attackB = fresh && da - db < 0 && mb && db > 0 && CanDamage(cb.role, ca.role);

	if (ma && mb)
		BounceBalls(a, *ma, b, *mb, c, e);
	else if (ma)
		BounceWall(a, *ma, c.normal, c.penetration, e);
	else if (mb)
	{
		BoardVec2 n = { -c.normal.x, -c.normal.z };
		BounceWall(b, *mb, n, c.penetration, e);
	}
	if (attackA)AddNotice(a, b, out);
	if (attackB)AddNotice(b, a, out);
}

//---------------
// 一組を解決する
//---------------
void CollisionSystem::Step(const std::vector<std::unique_ptr<GameObject>>& objects,
	std::vector<CollisionNotice>& out)
{
	out.clear();
	std::vector<HitPair> current;
	//iとjの組み合わせを一回ずつ調べる。
	for (size_t i = 0; i < objects.size(); ++i)
	{
		for (size_t j = i + 1; j < objects.size(); ++j)
		{
			if (!objects[i] || !objects[j])
				continue;
			SolvePair(*objects[i], *objects[j], out, current);
		}
	}
	m_previousContacts = current;
}
