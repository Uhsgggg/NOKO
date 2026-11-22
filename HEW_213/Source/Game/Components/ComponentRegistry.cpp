#include "ComponentRegistry.h"

#include "Game/Components/AutoRotateComponent.h"
#include "Game/Components/MoveComponent.h"
#include "Game/Components/ColliderComponent.h"
#include "Game/Components/BallMotionComponent.h"
#include "Game/Components/HealthComponent.h"
#include "Game/Components/PlayerInputComponent.h"
#include "Game/Components/PlayerLaunchComponent.h"
#include "Game/Components/ReflectionCounterComponent.h"

#include <stdexcept>
#include <cmath>

using namespace std;
using json = nlohmann::json;

namespace
{
	// JSONから3つのfloat値を読み込み
	void ReadVector3(
		const json& jsonValue,
		float outValue[3],
		const char* fieldName)
	{
		if (!jsonValue.is_array() || jsonValue.size() != 3)
		{
			throw runtime_error(
				string(fieldName) +
				"must contain 3 numbers."
			);
		}

		for (int i = 0; i < 3; ++i)
		{
			outValue[i] = jsonValue.at(i).get<float>();
		}
	}
}

const vector<ComponentRegistry::Entry>&
ComponentRegistry::GetEntries()
{
	// ・ラムダ式で記述
	// 　　別の名前付き関数を定義しなくても
	// 　　その場で関数の処理を記述できる記法
	static const vector<Entry> entries =
	{
		//========================================
		// AutoRotate
		//========================================
		{
			"AutoRotate",  // JSON保存用のID
			"自動回転", // Inspectorの表示名

			// 生成
			[](GameObject* pOwner)->unique_ptr<Component>
			{
				return unique_ptr<Component>(
					new AutoRotateComponent(
						pOwner,
						0.0f,
						360.0f,
						0.0f
					)
				);
			},

			// 保存
			[](const Component& baseComponent, json& jsonData)
			{
				// 今回の場合、
				// 〇dynamic_cast … 実際に指定した派生型へ
				// 　　　　　　　　　変換できるか確認する。
				// ✕static_cast  … 基底クラスからの変換では、
				// 　　　　　　　　　実際の派生型を実行時に確認しない。
				// これは登録ミスによって別のComponentが渡された場合、
				// 安全に失敗するとは限らないため。
				const auto& autoRotate =
					dynamic_cast<const AutoRotateComponent&>(baseComponent);

				const float* speed = autoRotate.GetRotationSpeed();

				jsonData["speed"] = {
					speed[0],
					speed[1],
					speed[2]
				};
			},

			// 読み込み
			[](Component& baseComponent, const json& jsonData)
			{
				float speed[3];

				ReadVector3(
					jsonData.at("speed"),
					speed,
					"speed"
				);

				auto& autoRotate =
					dynamic_cast<AutoRotateComponent&>(baseComponent);

				autoRotate.SetRotationSpeed(
					speed[0],
					speed[1],
					speed[2]
				);
			}
		},

		//========================================
		// Move
		//========================================
		{
			"Move",
			"自動移動",

			// 生成
			[](GameObject* pOwner)->unique_ptr<Component>
			{
				return unique_ptr<Component>(
					new MoveComponent(
						pOwner,
						1.0f,
						1.0f,
						0.0f
					)
				);
			},

			// 保存
			[](const Component& baseComponent, json& jsonData)
			{
				const auto& moveComponent =
					dynamic_cast<const MoveComponent&>(baseComponent);

				const float* velocity = moveComponent.GetVelocity();

				jsonData["velocity"] = {
					velocity[0],
					velocity[1],
					velocity[2]
				};
			},

			// 読み込み
			[](Component& baseComponent, const json& jsonData)
			{
				float velocity[3];

				ReadVector3(
					jsonData.at("velocity"),
					velocity,
					"velocity"
				);

				auto& moveComponent =
					dynamic_cast<MoveComponent&>(baseComponent);

				moveComponent.SetVelocity(
					velocity[0],
					velocity[1],
					velocity[2]
				);
			}
		},

		//========================================
		// BallMotion
		//========================================
		{
			"BallMotion",
			"発射移動",

			// 生成
			[](GameObject* pOwner)->unique_ptr<Component>
			{
				return make_unique<BallMotionComponent>(pOwner);
			},

			// 保存
			[](const Component& baseComponent, json& jsonData)
			{
				const auto& ballMotion =
					dynamic_cast<const BallMotionComponent&>(baseComponent);

				jsonData["deceleration"] = ballMotion.GetDeceleration();
				jsonData["stopSpeed"] = ballMotion.GetStopSpeed();
				jsonData["maxSpeed"] = ballMotion.GetMaxSpeed();
			},

			// 読み込み
			[](Component& baseComponent, const json& jsonData)
			{
				auto& ballMotion =
					dynamic_cast<BallMotionComponent&>(baseComponent);

				ballMotion.Configure(
					jsonData.at("deceleration").get<float>(),
					jsonData.at("stopSpeed").get<float>(),
					jsonData.at("maxSpeed").get<float>()
				);
			}
		},

		//========================================
		// PlayerLaunch
		//========================================
		{
			"PlayerLaunch",
			"プレイヤー発射",

			// 生成
			[](GameObject* pOwner)->unique_ptr<Component>
			{
				return make_unique<PlayerLaunchComponent>(pOwner);
			},

			// 保存
			[](const Component& baseComponent, json& jsonData)
			{
				const auto& playerLaunch =
					dynamic_cast<const PlayerLaunchComponent&>(baseComponent);

				jsonData["minDrag"] = playerLaunch.GetMinDrag();
				jsonData["maxDrag"] = playerLaunch.GetMaxDrag();
				jsonData["maxSpeed"] = playerLaunch.GetMaxSpeed();
			},

			// 読み込み
			[](Component& baseComponent, const json& jsonData)
			{
				auto& playerLaunch =
					dynamic_cast<PlayerLaunchComponent&>(baseComponent);

				playerLaunch.Configure(
					jsonData.at("minDrag").get<float>(),
					jsonData.at("maxDrag").get<float>(),
					jsonData.at("maxSpeed").get<float>()
				);
			}
		},

		//========================================
		// PlayerInput
		//========================================
		{
			"PlayerInput",
			"プレイヤー入力",

			// 生成
			[](GameObject* pOwner)->unique_ptr<Component>
			{
				return make_unique<PlayerInputComponent>(pOwner);
			},

			// 保存
			[](const Component& baseComponent, json& jsonData)
			{
				(void)baseComponent;
				(void)jsonData;
			},

			// 読み込み
			[](Component& baseComponent, const json& jsonData)
			{
				(void)baseComponent;
				(void)jsonData;
			}
		},

		//========================================
		// ReflectionCounter
		//========================================
		{
			"ReflectionCounter",
			"反発回数カウンター",

			// 生成
			[](GameObject* pOwner)->unique_ptr<Component>
			{
				return make_unique<ReflectionCounterComponent>(pOwner);
			},

			// 保存
			[](const Component& baseComponent, json& jsonData)
			{
				(void)baseComponent;
				(void)jsonData;
			},

			// 読み込み
			[](Component& baseComponent, const json& jsonData)
			{
				(void)baseComponent;
				(void)jsonData;
			}
		},

		//========================================
		// Health
		//========================================
		{
			"Health",
			"体力",

			// 生成
			[](GameObject* pOwner)->unique_ptr<Component>
			{
				return make_unique<HealthComponent>(pOwner);
			},

			// 保存
			[](const Component& baseComponent, json& jsonData)
			{
				const auto& health =
					dynamic_cast<const HealthComponent&>(baseComponent);

				jsonData["maxHP"] = health.GetMaxHP();
			},

			// 読み込み
			[](Component& baseComponent, const json& jsonData)
			{
				auto& health =
					dynamic_cast<HealthComponent&>(baseComponent);

				health.Reset(jsonData.at("maxHP").get<int>());
			}
		},

		//========================================
		// Collider
		//========================================
		{
			"Collider",
			"当たり判定",

			// 生成
			[](GameObject* pOwner)->unique_ptr<Component>
			{
				return make_unique<ColliderComponent>(pOwner);
			},

			// 保存
			[](const Component& baseComponent, json& jsonData)
			{
				const auto& collider =
					dynamic_cast<const ColliderComponent&>(baseComponent);

				const auto& colliderConfig = collider.GetConfig();
				jsonData["role"] = int(colliderConfig.role);
				jsonData["shape"] = int(colliderConfig.shape);
				jsonData["radius"] = colliderConfig.radius;
				jsonData["halfExtent"] = { 
					colliderConfig.halfExtent.x,
					colliderConfig.halfExtent.z
				};
				jsonData["restitution"] = colliderConfig.restitution;
			},

			// 読み込み
			[](Component& baseComponent, const json& jsonData)
			{
				auto& collider =
					dynamic_cast<ColliderComponent&>(baseComponent);

				ColliderConfig colliderConfig;

				colliderConfig.role = GameRole(jsonData.at("role").get<int>());
				colliderConfig.shape = ColliderShape(jsonData.at("shape").get<int>());
				colliderConfig.radius = jsonData.at("radius").get<float>();
				const auto& halfExtentJson = jsonData.at("halfExtent");
				
				if (!halfExtentJson.is_array() ||
					halfExtentJson.size() != 2)
				{
					throw runtime_error("Invalid halfExtent");
				}

				colliderConfig.halfExtent = {
					halfExtentJson.at(0).get<float>(),
					halfExtentJson.at(1).get<float>() 
				};
				colliderConfig.restitution = jsonData.at("restitution").get<float>();
				collider.Configure(colliderConfig);
			}
		}
	};

	return entries;
}

const ComponentRegistry::Entry*
ComponentRegistry::Find(const std::string& typeId)
{
	const vector<Entry>& entries = GetEntries();

	// 登録一覧を先頭から調べる
	for (const Entry& entry : entries)
	{
		if (typeId == entry.typeId)
		{
			return &entry;
		}
	}

	// 見つからなかった場合はnullptrで返す
	return nullptr;
}

