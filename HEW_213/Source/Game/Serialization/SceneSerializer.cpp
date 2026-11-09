#include "SceneSerializer.h"

#include "Source/ThirdParty/nlohmann/json.hpp"

#include "Source/Engine/Scene/GameObject.h"
#include "Source/Engine/Camera/CameraPlay.h"

#include "Source/Game/Objects/GameObjectFactory.h"
#include "Source/Game/Components/AutoRotateComponent.h"

#include <fstream>
#include <stdexcept>
#include <utility>

using json = nlohmann::json;
using namespace std;
using namespace DirectX;

namespace
{
	// JSON配列からfloat3個を読み込む
	// 　ReadFloat3() はPosition・Rotation・Scaleのような
	// 　３つのfloat値を読み込むための共通関数
	void ReadFloat3(
		const json& data,
		float values[3],
		const char* fieldName)
	{
		if (!data.is_array() || data.size() != 3)
		{
			// JSONの内容が不正だった場合に例外を発生させる
			// ※throwを使う場合、必ずtry / catchで受け取る必要がある
			throw runtime_error(
				string(fieldName) +
				" must be an array of 3 numbers."
			);
		}

		for (int i = 0; i < 3; i++)
		{
			values[i] = data.at(i).get<float>();
		}
	}
}

bool SceneSerializer::Save(
	const string& filePath,
	const vector<unique_ptr<GameObject>>& objects,
	const CameraPlay& playCamera,
	string& error)
{
	error.clear();

	try
	{
		// JSON全体を作成
		json root;

		// 保存形式のバージョン
		root["version"] = 1;

		// GameObject一覧
		root["objects"] = json::array();

		for (const auto& pObject : objects)
		{
			if (!pObject) continue;

			json objectData;

			//========================================
			// GameObjectの種類
			//========================================
			// GameObjectFactoryから種類名を取得
			string type =
				GameObjectFactory::GetTypeName(*pObject);

			// 未対応の種類なら保存しない
			if (type.empty())
			{
				throw runtime_error(
					"Unsupported GameObject type."
				);
			}

			objectData["type"] = type;

			//========================================
			// オブジェクト名
			//========================================
			objectData["name"] = pObject->GetName();

			//========================================
			// Transform
			//========================================
			const Transform& t = pObject->GetTransform();

			objectData["transform"] = {
				{
					"position",
					{
						t.position[0],
						t.position[1],
						t.position[2]
					}
				},
				{
					"rotation",
					{
						t.rotation[0],
						t.rotation[1],
						t.rotation[2]
					}
				},
				{
					"scale",
					{
						t.scale[0],
						t.scale[1],
						t.scale[2]
					}
				}
			};

			//========================================
			// Components
			//========================================
			objectData["components"] = json::array();

			for (const auto& pComponent : pObject->GetComponents())
			{
				if (!pComponent) continue;

				const AutoRotateComponent* pRotate =
					dynamic_cast<const AutoRotateComponent*>(
						pComponent.get()
						);

				if (pRotate == nullptr)
				{
					throw runtime_error(
						"Unsupported Component type."
					);
				}

				const float* speed = pRotate->GetRotationSpeed();

				json componentData;

				componentData["type"] = "AutoRotate";

				componentData["enabled"] = pRotate->IsEnabled();

				componentData["speed"] = {
					speed[0],
					speed[1],
					speed[2]
				};

				objectData["components"].push_back(componentData);
			}

			root["objects"].push_back(objectData);
		}

		//========================================
		// Playカメラの情報を保存
		//========================================
		const auto& cameraPos = playCamera.GetPosition();
		const auto& cameraLook = playCamera.GetLook();

		root["playCamera"] = {
			{
				"position",
				{
					cameraPos.x,
					cameraPos.y,
					cameraPos.z
				}
			},
			{
				"look",
				{
					cameraLook.x,
					cameraLook.y,
					cameraLook.z
				}
			}
		};

		//========================================
		// JSONをファイルへ書き込む
		//========================================
		// 読みやすいようにインデント4で変換
		string text = root.dump(4);

		ofstream file(
			filePath,
			ios::binary | ios::trunc
		);

		if (!file.is_open())
		{
			throw runtime_error(
				"Cannot open file for writing: " +
				filePath
			);
		}

		file.write(
			text.data(),
			static_cast<streamsize>(text.size())
		);

		file.flush();

		if (!file)
		{
			throw runtime_error(
				"Failed to write stage file."
			);
		}

		return true;
	}
	catch (const std::exception& e)
	{
		error = e.what();
		return false;
	}
}

bool SceneSerializer::Load(
	const string& filePath,
	vector<unique_ptr<GameObject>>& objects,
	CameraPlay& playCamera,
	string& error)
{
	error.clear();

	try
	{
		//========================================
		// JSONファイルを開く
		//========================================
		ifstream file(
			filePath,
			ios::binary
		);

		if (!file.is_open())
		{
			throw runtime_error(
				"Cannot open stage file: " +
				filePath
			);
		}

		// JSONとして読み込む
		json root = json::parse(file);

		// 保存形式を確認
		int version = root.at("version").get<int>();

		if (version != 1)
		{
			throw runtime_error(
				"Unsupported stage version."
			);
		}

		const json& savedObjects = root.at("objects");

		if (!savedObjects.is_array())
		{
			throw runtime_error(
				"objects must be an array."
			);
		}

		// 復元先を一時的に作る
		vector<unique_ptr<GameObject>> loadedObjects;

		//========================================
		// GameObjectを復元する
		//========================================
		for (const auto& objectData : savedObjects)
		{
			string type = objectData.at("type").get<string>();

			// JSONに記録された種類から生成
			unique_ptr<GameObject> pObject =
				GameObjectFactory::Create(type);

			// 未対応の種類なら読み込み失敗
			if (!pObject)
			{
				throw runtime_error(
					"Unknown GameObject type:" + type
				);
			}

			//========================================
			// 名前を復元
			//========================================
			string name = objectData.at("name").get<string>();

			if (name.empty())
			{
				throw runtime_error(
					"GameObject name is empty"
				);
			}

			// JSONから読み取った名前を反映
			pObject->SetName(name);

			// 名前の重複を検査
			for (const auto& pExisting : loadedObjects)
			{
				if (pExisting->GetName() == name)
				{
					throw runtime_error(
						"Duplicate GameObject name."
					);
				}
			}

			//========================================
			// Transformを復元
			//========================================
			const json& savedTransform = objectData.at("transform");

			Transform& t = pObject->GetTransform();

			ReadFloat3(
				savedTransform.at("position"),
				t.position,
				"position"
			);

			ReadFloat3(
				savedTransform.at("rotation"),
				t.rotation,
				"rotation"
			);

			ReadFloat3(
				savedTransform.at("scale"),
				t.scale,
				"scale"
			);

			//========================================
			// Componentsを復元
			//========================================
			// Goalの初期コンポーネントも消す
			pObject->ClearComponents();

			const json& savedComponents = objectData.at("components");

			if (!savedComponents.is_array())
			{
				throw runtime_error(
					"components must be an array."
				);
			}

			for (const auto& componentData : savedComponents)
			{
				string componentType =
					componentData.at("type").get<string>();

				if (componentType == "AutoRotate")
				{
					float speed[3];

					ReadFloat3(
						componentData.at("speed"),
						speed,
						"speed"
					);

					unique_ptr<Component> pComponent(
						new AutoRotateComponent(
							pObject.get(),
							speed[0],
							speed[1],
							speed[2]
						)
					);

					bool enabled =
						componentData.at("enabled").get<bool>();

					pComponent->SetEnabled(enabled);

					if (!pObject->AddComponent(move(pComponent)))
					{
						throw runtime_error(
							"Duplicate Component."
						);
					}
				}
				else
				{
					throw runtime_error(
						"Unknown Component type: " +
						componentData
					);
				}
			}

			// 復元した座標を初期状態にする
			pObject->SaveInitialTransform();

			loadedObjects.push_back(move(pObject));
		}

		//========================================
		// Playカメラの情報を読み込む
		//========================================
		XMFLOAT3 cameraPos = playCamera.GetPosition();
		XMFLOAT3 cameraLook = playCamera.GetLook();

		if (root.contains("playCamera"))
		{
			const json& cameraData = root.at("playCamera");

			float pos[3];
			float look[3];

			ReadFloat3(
				cameraData.at("position"),
				pos,
				"playCamera.position"
			);

			ReadFloat3(
				cameraData.at("look"),
				look,
				"playCamera.look"
			);

			float dx = look[0] - pos[0];
			float dz = look[2] - pos[2];

			if (!(dx * dx + dz * dz > 0.0001f))
			{
				throw runtime_error(
					"Invaild Play Camera direction."
				);
			}

			cameraPos = XMFLOAT3(
				pos[0],
				pos[1],
				pos[2]
			);

			cameraLook = XMFLOAT3(
				look[0],
				look[1],
				look[2]
			);
		}

		//========================================
		// 読み込みが全て成功したら入れ替える
		//========================================
		objects.swap(loadedObjects);

		//========================================
		// Playカメラの位置も復元
		//========================================
		playCamera.SetPose(
			cameraPos,
			cameraLook
		);

		return true;
	}
	catch (const exception& e)
	{
		error = e.what();
		return false;
	}
}