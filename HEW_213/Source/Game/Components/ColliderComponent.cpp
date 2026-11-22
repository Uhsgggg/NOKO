#include "ColliderComponent.h"
#include"Engine/Scene/GameObject.h"
#include<cmath>
#ifdef _DEBUG
#include"imgui.h"
#include "Engine/Graphics/Geometory.h"
using namespace ImGui;
using namespace DirectX;
#endif

using namespace std;

ColliderComponent::ColliderComponent(GameObject *pOwner)
	: Component(pOwner)
{
}

void ColliderComponent::Configure(const ColliderConfig& config)
{
	//不正な設定なら変更しない
	if (static_cast<unsigned>(config.role) > 3 ||
		static_cast<unsigned>(config.shape) > 1)
		return;
	if (!isfinite(config.radius) ||
		!isfinite(config.halfExtent.x) ||
		!isfinite(config.halfExtent.z) ||
		!isfinite(config.restitution))
		return;
	if (config.radius <= 0 ||
		config.halfExtent.x <= 0 ||
		config.halfExtent.z <= 0)
		return;
	if (config.restitution < 0 ||
		config.restitution > 1)
		return;

	m_config = config;
	
}

const ColliderConfig& ColliderComponent::GetConfig() const
{
	return m_config;
}

unique_ptr<Component> ColliderComponent::Clone(GameObject *pNewOwner) const
{
	auto pCopy = make_unique<ColliderComponent>(pNewOwner);
	pCopy->Configure(m_config);
	pCopy->SetEnabled(IsEnabled());
	return pCopy;
}

#ifdef _DEBUG
void ColliderComponent::DrawDebugInspector()
{
	ColliderConfig c = m_config;
	int role = int(c.role);
	int shape = int(c.shape);
	Combo("役職", &role, "プレイヤー\0敵\0ボス\0壁\0");
	Combo("形状", &shape, "球体\0箱\0");
	c.role = GameRole(role);
	c.shape = ColliderShape(shape);
	DragFloat("半径", &c.radius, 0.05f, 0.01f, 100);
	DragFloat("Ｘ方向の半サイズ", &c.halfExtent.x, 0.05f, 0.01f, 100); // 横方向の半サイズ
	DragFloat("Ｚ方向の半サイズ", &c.halfExtent.z, 0.05f, 0.01f, 100); // 奥行方向の半サイズ
	DragFloat("反発係数", &c.restitution, 0.01f, 0, 1.0f); // 反発係数
	Configure(c);
}

// 衝突した瞬間に呼ぶ。約0.15秒間だけ色を変える
void ColliderComponent::NotifyDebugHit()
{
	m_nDebugHitFrames = 0;
}

// SceneGameから毎フレーム1回だけ呼ぶ
void ColliderComponent::UpdateDebugHit()
{
	if (!m_pOwner || !m_pOwner->IsActive() || !IsEnabled())
	{
		m_nDebugHitFrames = 0;
		return;
	}

	if (m_nDebugHitFrames > 0)
	{
		--m_nDebugHitFrames;
	}
}

bool ColliderComponent::IsDebugHit() const
{
	return m_nDebugHitFrames > 0;
}

void ColliderComponent::DrawDebug() const
{
	if (!m_pOwner || !m_pOwner->IsActive() || !IsEnabled())
	{
		return;
	}

	const Transform& t = m_pOwner->GetTransform();
	float x = t.position[0];
	float y = t.position[1];
	float z = t.position[2];
	if (!isfinite(x) || !isfinite(y) || !isfinite(z)) return;

	// 通常は種類ごとに色を変える
	XMFLOAT4 color(1.0f, 0.85f, 0.15f, 1.0f); // Wall:YELLOW
	if (m_config.role == GameRole::Player)
	{
		color = XMFLOAT4(0.1f, 0.65f, 1.0f, 1.0f);
	}
	else if (m_config.role == GameRole::Enemy)
	{
		color = XMFLOAT4(1.0f, 0.2f, 0.2f, 1.0f);
	}
	else if (m_config.role == GameRole::Boss)
	{
		color = XMFLOAT4(0.8f, 0.3f, 1.0f, 1.0f);
	}

	// 衝突した直後は種類に関係なく白く光らせる
	if (IsDebugHit())
	{
		color = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.95f);
	}

	if (m_config.shape == ColliderShape::Sphere)
	{
		const int DIV = 32;
		const float PI = 3.1415926535f;
		float radius = m_config.radius;
		for (int i = 0; i < DIV; ++i)
		{
			float a = 2.0f * PI * i / DIV;
			float b = 2.0f * PI * (i + 1) / DIV;
			XMFLOAT3 start(x + cos(a) * radius, y,
						   z + sin(a) * radius);
			XMFLOAT3 end(x + cos(b) * radius, y,
						 z + sin(b) * radius);
			Geometory::AddLine(start, end, color);
		}
	}
	else if (m_config.shape == ColliderShape::Box)
	{
		float hx = m_config.halfExtent.x;
		float hz = m_config.halfExtent.z;
		XMFLOAT3 p1(x - hx, y, z - hz);
		XMFLOAT3 p2(x + hx, y, z - hz);
		XMFLOAT3 p3(x + hx, y, z + hz);
		XMFLOAT3 p4(x - hx, y, z + hz);
		Geometory::AddLine(p1, p2, color);
		Geometory::AddLine(p2, p3, color);
		Geometory::AddLine(p3, p4, color);
		Geometory::AddLine(p4, p1, color);
	}

	// 1物体ずつ描くので既存の1000本の線バッファを超えない
	Geometory::DrawLines();
}
#endif
