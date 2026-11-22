#include "SceneGame.h"

#include "Engine/Camera/CameraPlay.h"
#include "Engine/Graphics/Geometory.h"

#include "Game/Serialization/SceneSerializer.h"
#include "Game/Components/BallMotionComponent.h"
#include "Game/Components/ColliderComponent.h"
#include "Game/Components/HealthComponent.h"
#include "Game/Components/MoveComponent.h"
#include "Game/Components/PlayerInputComponent.h"
#include "Game/Components/PlayerLaunchComponent.h"
#include "Game/Components/ReflectionCounterComponent.h"

#include "Game/Objects/PrimitiveObject.h"

#include "Game/Systems/DamageFormula.h"

#include "Game/UI/HudPresenter.h"

#ifdef _DEBUG
#include "imgui.h"
using namespace ImGui;
#endif

#include <Windows.h>
#include <cmath>
#include <string>

using namespace DirectX;
using namespace std;

namespace
{
	// ステージの保存先
	const char* const kStageFilePath =
		"Assets/Stages/Stage01.json";

	// プロトタイプの仮仕様。企画確定後に変更するところ
	constexpr int kShots = 10;
	constexpr int kWaveInterval = 3;
	constexpr float kBaseDamage = 10.0f;
	constexpr float kReflectionMultiplier = 1.1f;
	template <class T> T* Enabled(GameObject& o)
	{
		T* p = o.GetComponent<T>();
		if (!p || !p->IsEnabled()) return nullptr;
		return p;
	}
	bool Role(GameObject& o, GameRole role)
	{
		auto* p = Enabled<ColliderComponent>(o);
		return p && p->GetConfig().role == role;
	}
	GameObject* Player(const vector<unique_ptr<GameObject>>& objects)
	{
		for (const auto& p : objects)
		{
			if (p && p->IsActive() && Role(*p, GameRole::Player))
			{
				return p.get();
			}
		}

		return nullptr;
	}
	BoardVec2 Position(const GameObject& o)
	{
		const auto& transform = o.GetTransform();
		return { transform.position[0], transform.position[2] };
	}
}

//========================================
// エディタの処理とは分けて管理する
//========================================
bool SceneGame::LoadGameplayStage()
{
	string error;
	if (SceneSerializer::Load(
		kStageFilePath, m_objects, *m_pPlayCamera, error))
	{
		m_collision.Reset();
		return true;
	}
	m_gameReady = false;
	MessageBoxA(nullptr, error.c_str(), "Stage Load Error", MB_OK | MB_ICONERROR);
	return false;
}

// ゲーム初期化
void SceneGame::ResetGamePlay()
{
	m_collision.Reset();
	m_turn.Reset(kShots);
	m_boardPointer.Reset();
	m_result = 0;
	m_completedShots = 0;
	m_spawnSerial = 0;
	m_hasBoss = false;
	m_hadEnemies = false;
#ifdef _DEBUG
	m_damageDebugStats.clear();
#endif

	// 全てのオブジェクトからプレイヤーの情報を取得して無効化する
	for (auto& p : m_objects)
	{
		if (!p) continue;
		p->SetActive(true);

		auto* pMotion = Enabled<BallMotionComponent>(*p);
		auto* pOld = p->GetComponent<MoveComponent>();
		auto* pCounter = Enabled<ReflectionCounterComponent>(*p);
		auto* pHP = Enabled<HealthComponent>(*p);
		auto* pInput = Enabled<PlayerInputComponent>(*p);

		if (pMotion)
		{
			pMotion->Stop();
			if (pOld) pOld->SetEnabled(false);
		}
		if (pCounter) pCounter->Reset();
		if (pHP) pHP->Reset(pHP->GetMaxHP());
		if (pInput) pInput->Cancel();
		if (Role(*p, GameRole::Boss)) m_hasBoss = true;
		if (Role(*p, GameRole::Enemy)) m_hadEnemies = true;
	}

	// プレイヤーを取得
	GameObject* pPlayer = Player(m_objects);
	m_gameReady = false;
	// コンポーネントを取得
	if (pPlayer)
	{
		m_gameReady = Enabled<PlayerInputComponent>(*pPlayer) &&
			Enabled<PlayerLaunchComponent>(*pPlayer) &&
			Enabled<BallMotionComponent>(*pPlayer);
	}
	if (!m_gameReady)
	{
		MessageBoxA(nullptr, "Player components missing. Check Stage01.json", "Game Setup", MB_OK | MB_ICONERROR);
	}

	UpdateHUD();
}

//　マウスから発射まで
void SceneGame::UpdatePlayerInput()
{
	// プレイヤーが存在するか確認
	GameObject* pPlayer = Player(m_objects);
	if (!pPlayer)
	{
		m_boardPointer.Reset();
		return;
	}
	// 必要なComponentが存在するか確認
	auto* pInput = Enabled<PlayerInputComponent>(*pPlayer);
	auto* pLaunch = Enabled<PlayerLaunchComponent>(*pPlayer);
	auto* pMotion = Enabled<BallMotionComponent>(*pPlayer);
	if (!pInput || !pLaunch || !pMotion ||
		!m_pPlayCamera || !m_turn.CanLaunch())
	{
		m_boardPointer.Reset();
		if (pInput)
		{
			pInput->Cancel();
		}

		return;
	}

	// マウスの移動量を取得する
	PointerFrame frame = m_boardPointer.BuildFrame(*m_pPlayCamera);

	pInput->Sample(frame, true);

	// ドラッグを離したときだけ速度を作る
	BoardVec2 v;
	if (!pLaunch->TryBuildVelocity(pInput->GetAim(), v)) return;

	float speed = sqrt(v.x * v.x + v.z * v.z);
	if (speed <= pMotion->GetStopSpeed()) return;
	if (!m_turn.TryConsumeShot()) return;

	m_collision.Reset();
	for (auto& pObject : m_objects)
	{
		if (!pObject || !pObject->IsActive()) continue;
		auto* pCounter = Enabled<ReflectionCounterComponent>(*pObject);
		if (pCounter) pCounter->Reset();
	}
	pMotion->SetVelocity(v);
	pInput->Cancel();
}

// 毎フレーム処理
void SceneGame::UpdateGameplay(float deltaTime)
{
	if (!m_gameReady || m_result)
	{
		m_boardPointer.Reset();
		return;
	}

	// 発射中だけ物理更新
	UpdatePlayerInput();
	if (m_turn.IsResolving())
	{
		UpdatePhysics(deltaTime);
		// 全停止時だけFinishTurnが1回動く
		if (m_turn.EndIfStopped(AnyMoving()))
		{
			FinishTurn();
		}
	}
	UpdateHUD();
}

// 物理更新
void SceneGame::UpdatePhysics(float deltaTime)
{
	// 入力はここに入れない。移動と衝突を毎フレーム4回に分ける
	for (int i = 0; i < 4; ++i)
	{
		for (auto& p : m_objects)
		{
			if (p && p->IsActive())
			{// 各物体をdt/4秒ずつ移動する
				p->Update(deltaTime / 4);
			}
		}
		// 衝突を調べる。通知はすぐ消費する。入力は4回呼ばない
		vector<CollisionNotice> notices;
		m_collision.Step(m_objects, notices);
		ApplyCollisionNotices(notices);
	}
}

// HPへ通知を渡す
void SceneGame::ApplyCollisionNotices(const vector<CollisionNotice>& notices)
{
	// CollisionSystemが攻撃できる組み合わせだけを通知する
	for (const CollisionNotice& notice : notices)
	{
		GameObject* pTarget = notice.target;
		if (!notice.attacker || !pTarget) continue;
		if (!notice.attacker->IsActive() || !pTarget->IsActive()) continue;
		auto* pHP = Enabled<HealthComponent>(*pTarget);
		if (!pHP) continue;

		const int calculatedDamage = 
			DamageFormula::Calculate(
				kBaseDamage, 
				kReflectionMultiplier,
				notice.reflectionCount
			);

		// 実際に減少したHPを記録
		const int actualDamage = pHP->ApplyDamage(calculatedDamage);

#ifdef _DEBUG
		// 攻撃したGameObjectの履歴を取得
		DamageDebugStats& stats = m_damageDebugStats[notice.attacker];
		// 今回の攻撃情報を保存
		stats.lastCalculateDamage = calculatedDamage;
		stats.lastAppliedDamage = actualDamage;
		stats.lastReflectionCount = notice.reflectionCount;
		// 最後に攻撃した相手の名前
		stats.lastTargetName = pTarget->GetName();
		// 累計ダメージと攻撃回数
		stats.totalAppliedDamage += actualDamage;
		++stats.hitCount;
#endif

		pHP->ApplyDamage(calculatedDamage);
		if (pHP->IsDead())
		{
			auto* pMotion = Enabled<BallMotionComponent>(*pTarget);
			if (pMotion) pMotion->Stop();
			pTarget->SetActive(false); // vectorから削除せず、無効にする
		}
	}
}

// 全停止の確認
bool SceneGame::AnyMoving() const
{
	// 動いているプレイヤーと敵だけを順に調べる
	for (const auto& p : m_objects)
	{
		if (!p || !p->IsActive()) continue;
		if (!Role(*p, GameRole::Player) &&
			!Role(*p, GameRole::Enemy)) continue;
		// 1体でも動いていれば true
		auto* pMotion = Enabled<BallMotionComponent>(*p);
		if (pMotion && pMotion->IsMoving()) return true;
	}
	// 最後まで見つからなければ false
	return false;
}

// 勝敗と敵追加
void SceneGame::FinishTurn()
{
	++m_completedShots;
	bool bossAlive = false;
	bool enemyAlive = false;
	for (const auto& p : m_objects)
	{// ボス、敵が生きているか調べる
		if (!p || !p->IsActive()) continue;
		if (Role(*p, GameRole::Boss)) bossAlive = true;
		if (Role(*p, GameRole::Enemy)) enemyAlive = true;
	}

	// クリアしているか判定
	if (m_hasBoss && !bossAlive)
	{
		m_result = 1;
	}
	else if (!m_hasBoss && m_hadEnemies && !enemyAlive)
	{
		m_result = 1;
	}
	// 手数が 0 ならゲームオーバー
	else if (m_turn.GetRemainingShots() == 0)
	{
		m_result = 2;
	}
	// 〇手ごとに敵追加（プロトでは 3 手）
	else if (m_completedShots % kWaveInterval == 0)
	{
		SpawnEnemy();
	}
}

// 敵を空いている場所に作る
bool SceneGame::SpawnEnemy()
{
	// ステージの空き位置を決まった順で確認
	// 重なっている場所には出さないようにする
	auto p = make_unique<PrimitiveObject>(PrimitiveShape::Sphere);
	ColliderConfig config;
	config.role = GameRole::Enemy;
	config.shape = ColliderShape::Sphere;
	auto collider = make_unique<ColliderComponent>(p.get());
	collider->Configure(config);
	p->AddComponent(move(collider));
	p->AddComponent(make_unique<BallMotionComponent>(p.get()));
	p->AddComponent(make_unique<HealthComponent>(p.get()));
	p->AddComponent(make_unique<ReflectionCounterComponent>(p.get()));
	auto& t = p->GetTransform();
	t.position[1] = 0.55f;
	for (int z = -3; z <= 3; z += 2)
	{
		for (int x = -5; x <= 5; x += 2)
		{
			t.position[0] = float(x);
			t.position[2] = float(z);
			bool free = true;
			for (auto& other : m_objects)
			{
				if (other && other->IsActive())
				{
					Contact hit;
					if (CollisionSystem::Detect(*p, *other, hit))
					{
						free = false;
						break;
					}
				}
				if (free)
				{
					string name;
					bool exists;
					do
					{
						name = "WaveEnemy_" + to_string(++m_spawnSerial);
						exists = false;
						for (auto& o : m_objects)
						{
							if (o && o->GetName() == name)
							{
								exists = true;
							}
						}
					} while (exists);
					p->SetName(name);
					p->SetColor(XMFLOAT4(0, 1, 0, 1));
					p->SaveInitialTransform();
					m_objects.push_back(move(p));
					return true;
				}
			}
		}
	}
	return false;
}

// HUDに渡す数値を集める
void SceneGame::UpdateHUD()
{
	HudInput data;

	// 残りの発射回数
	data.remainingShots = m_turn.GetRemainingShots();
	// あと何手で敵が追加されるか
	int shotsUntilNextWave = kWaveInterval - (m_completedShots % kWaveInterval);
	data.shotsUntilNextWave = shotsUntilNextWave;
	// 残りの手数
	int shots = m_turn.GetRemainingShots();
	if (m_turn.IsResolving()) ++shots;

	vector<GameObject*> enemies;
	float maxDrag = 3;
	for (auto& p : m_objects)
	{
		if (p)
		{
			if (Role(*p, GameRole::Boss))
			{
				if (auto* h = Enabled<HealthComponent>(*p))
				{
					data.bossHP = h->GetHP();
					data.bossMaxHP = h->GetMaxHP();
				}
			}
			if (p->IsActive() && Role(*p, GameRole::Enemy))
			{
				if (auto* h = Enabled<HealthComponent>(*p))
				{
					enemies.push_back(p.get());
					data.enemyHPRatios.push_back(float(h->GetHP()) / h->GetMaxHP());
				}
			}
			if (p->IsActive() && Role(*p, GameRole::Player))
			{
				data.playerPosition = Position(*p);
				m_arrowY = p->GetTransform().position[1] + 0.6f;
				if (auto* input = Enabled<PlayerInputComponent>(*p))
				{
					data.aim = input->GetAim();
				}
				if (auto* l = Enabled<PlayerLaunchComponent>(*p))
				{
					maxDrag = l->GetMaxDrag();
				}
			}
		}
		m_hud = HudPresenter::Build(data, maxDrag);
		for (size_t i = 0; i < enemies.size(); ++i)
		{
			HudColor color = m_hud.enemyColors[i];
			enemies[i]->SetColor(XMFLOAT4(color.r, color.g, color.b, color.a));
		}
	}
}

// 矢印とHUDを描く
void SceneGame::DrawGameplayHUD()
{
	if (!m_gameReady) return;

	if (m_hud.showArrow)
	{
		BoardVec2 start = m_hud.arrowStart;
		BoardVec2 end = m_hud.arrowEnd;
		float x = end.x - start.x;
		float z = end.z - start.z;
		float length = sqrt(x * x + z * z);
		if (length > 0)
		{
			XMFLOAT4X4 identity;
			XMStoreFloat4x4(&identity, XMMatrixIdentity());
			Geometory::SetWorld(identity);
			XMFLOAT4 color(0.2f, 0.8f, 1.0f, 1.0f);
			float head = length * 0.4f;
			if (head > 0.7f) head = 0.7f;
			float ux = x / length;
			float uz = z / length;
			XMFLOAT3 tip(end.x, m_arrowY, end.z);
			XMFLOAT3 left(end.x - head * ux + head * 0.5f * uz,
				m_arrowY, end.z - head * uz - head * 0.5f * ux);
			XMFLOAT3 right(end.x - head * ux - head * 0.5f * uz,
				m_arrowY, end.z - head * uz + head * 0.5f * ux);
			Geometory::AddLine(XMFLOAT3(start.x, m_arrowY, start.z), tip, color);
			Geometory::AddLine(tip, left, color);
			Geometory::AddLine(tip, right, color);
			Geometory::DrawLines();
		}
	}
	m_hudRenderer.Draw(m_hud, m_hasBoss, m_result);
}

#ifdef _DEBUG
void SceneGame::DrawDamageMonitor()
{
	if (!m_showDamageMonitor) return;

	// ウィンドウ設定
	const ImGuiWindowFlags windowFlags =
		ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoSavedSettings;
	SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
	SetNextWindowBgAlpha(0.85f);

	if (Begin("戦闘デバッグ", nullptr, windowFlags))
	{
		// ゲーム全体の情報
		Text("基礎ダメ: %.1f　反射倍率: %.2f",
			kBaseDamage, kReflectionMultiplier);
		Text("残り手数: %d", m_turn.GetRemainingShots());
		Separator();

		// 各GameObjectの情報
		for (const auto& pObject : m_objects)
		{
			if (!pObject) continue;

			// Colliderからオブジェクトの役割を取得
			auto* pCollider = Enabled<ColliderComponent>(*pObject);

			if (!pCollider) continue;


			const GameRole role = pCollider->GetConfig().role;
			// ボスのHP表示
			if (role == GameRole::Boss)
			{
				PushStyleColor(
					ImGuiCol_Text,
					ImVec4(0.8f, 0.5f, 1.0f, 1.0f)
				);

				auto* pHP = Enabled<HealthComponent>(*pObject);

				if (pHP)
				{
					Text("[BOSS] %s HP: %d / %d",
						pObject->GetName().c_str(),
						pHP->GetHP(),
						pHP->GetMaxHP()
					);
				}

				PopStyleColor();
				continue;
			}

			// プレイヤーと雑魚敵だけ表示
			if (role != GameRole::Player &&
				role != GameRole::Enemy)
				continue;

			// 反射回数のコンポーネントを取得
			auto* pCounter = Enabled<ReflectionCounterComponent>(*pObject);

			// 現在の反射回数を使ってダメージを計算
			const int reflectionCount = 
				pCounter ? pCounter->GetCount() : 0;

			// 現在の攻撃力を計算
			const int currentDamage =
				DamageFormula::Calculate(
					kBaseDamage,
					kReflectionMultiplier,
					reflectionCount
				);

			// 保存済みの履歴を検索
			const auto statsIt =
				m_damageDebugStats.find(pObject.get());

			const DamageDebugStats* pStats =
				statsIt != m_damageDebugStats.end()
				? &statsIt->second
				: nullptr;

			const char* roleName =
				role == GameRole::Player
				? "PLAYER" : "ENEMY";
			const ImVec4 textColor =
				role == GameRole::Player
				? ImVec4(0.35f, 0.8f, 1.0f, 1.0f)
				: ImVec4(1.0f, 0.4f, 0.4f, 1.0f);
			PushStyleColor(ImGuiCol_Text, textColor);

			Text("[%s] %s", 
				roleName, 
				pObject->GetName().c_str()
			);
			Text("反射: %d　攻撃力: %d", 
				reflectionCount, 
				currentDamage
			);

			if (pStats)
			{
				Text("直近ダメージ: %d",
					pStats->lastAppliedDamage
				);
				Text("累計ダメージ: %lld",
					pStats->totalAppliedDamage
				);
				Text("攻撃回数: %d",
					pStats->hitCount
				);
				Text("最後の攻撃対象: %s",
					pStats->lastTargetName.c_str()
				);
				Text("攻撃時の反射: %d / 計算値: %d",
					pStats->lastReflectionCount,
					pStats->lastCalculateDamage
				);
			}
			else
			{
				TextUnformatted("まだ攻撃してません");
			}

			Separator();
			PopStyleColor();
		}
	}

	End();
}
#endif