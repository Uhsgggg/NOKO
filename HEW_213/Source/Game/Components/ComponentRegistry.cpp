#include "ComponentRegistry.h"

#include "Game/Components/AutoRotateComponent.h"
#include "Game/Components/MoveComponent.h"

#include <stdexcept>

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
			"AutoRotate",
			"Auto Rotate",

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
			const AutoRotateComponent& c =
				static_cast<const AutoRotateComponent&>(base);

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
				static_cast<AutoRotateComponent&>(base);

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
					static_cast<const MoveComponent&>(base);

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
					static_cast<MoveComponent&>(base);

				c.SetVelocity(
					velocity[0],
					velocity[1],
					velocity[2]
				);
			}
		}

		//========================================
		// 今後追加予定
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

