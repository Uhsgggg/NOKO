#include "ComponentRegistry.h"

#include "Game/Components/AutoRotateComponent.h"
#include "Game/Components/MoveComponent.h"

#include <stdexcept>
#include <cmath>

using namespace std;
using json = nlohmann::json;

namespace
{
	// JSONから3つのfloat値を読み込み
	void ReadVector3(
		const json& value,
		float result[3],
		const char* field)
	{
		if (!value.is_array() || value.size() != 3)
		{
			throw runtime_error(
				string(field) +
				"must contain 3 numbers."
			);
		}

		for (int i = 0; i < 3; ++i)
		{
			result[i] = value.at(i).get<float>();
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
			"Auto Rotate", // Inspectorの表示名

			// 生成
			[](GameObject* owner)->unique_ptr<Component>
			{
				return unique_ptr<Component>(
					new AutoRotateComponent(
						owner,
						0.0f,
						360.0f,
						0.0f
					)
				);
			},

		// 保存
		[](const Component& base, json& data)
		{
			// 今回の場合、
			// 〇dynamic_cast … 実際に指定した派生型へ
			// 　　　　　　　　　変換できるか確認する。
			// ✕static_cast  … 基底クラスからの変換では、
			// 　　　　　　　　　実際の派生型を実行時に確認しない。
			// これは登録ミスによって別のComponentが渡された場合、
			// 安全に失敗するとは限らないため。
			const AutoRotateComponent& c =
				dynamic_cast<const AutoRotateComponent&>(base);

			const float* speed = c.GetRotationSpeed();

			data["speed"] = {
				speed[0],
				speed[1],
				speed[2]
			};
		},

		// 読み込み
		[](Component& base, const json& data)
		{
			float speed[3];

			ReadVector3(
				data.at("speed"),
				speed,
				"speed"
			);

			AutoRotateComponent& c =
				dynamic_cast<AutoRotateComponent&>(base);

			c.SetRotationSpeed(
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
			"Move",

			// 生成
			[](GameObject* owner)->unique_ptr<Component>
			{
				return unique_ptr<Component>(
					new MoveComponent(
						owner,
						1.0f,
						1.0f,
						0.0f
					)
				);
			},

			// 保存
			[](const Component& base, json& data)
			{
				const MoveComponent& c =
					dynamic_cast<const MoveComponent&>(base);

				const float* velocity = c.GetVelocity();

				data["velocity"] = {
					velocity[0],
					velocity[1],
					velocity[2]
				};
			},

			// 読み込み
			[](Component& base, const json& data)
			{
				float velocity[3];

				ReadVector3(
					data.at("velocity"),
					velocity,
					"velocity"
				);

				MoveComponent& c =
					dynamic_cast<MoveComponent&>(base);

				c.SetVelocity(
					velocity[0],
					velocity[1],
					velocity[2]
				);
			}
		}

		//========================================
		// 追加はここから
		//========================================
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

