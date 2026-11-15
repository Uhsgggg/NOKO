//========================================//
// 
// SceneSerialize.cpp
// 
//########################################
// ・SceneSerializer::Save()
// 　保存全体の流れを管理
// 
// 　・SerializeObject()
// 　　GameObject 1個をJSONに変換
// 　
// 　・SerializeTransform()
// 　　Transform担当
// 　
// 　・SerializeComponents()
// 　　Component担当
// 　
// 　・SerializeCamera()
// 　　カメラ担当
// 　
// 　・WriteStageFile()
// 　　ファイル書き込み担当
//########################################
// 
//========================================//
#include "SceneSerializer.h"

#include "ThirdParty/nlohmann/json.hpp"

#include "Engine/Scene/GameObject.h"
#include "Engine/Camera/CameraPlay.h"

#include "Game/Objects/GameObjectFactory.h"
#include "Game/Objects/PrimitiveObject.h"
#include "Game/Objects/FbxObject.h"

#include "Game/Components/ComponentRegistry.h"

#include <Windows.h>
#include <cmath>
#include <fstream>
#include <stdexcept>
#include <unordered_set>
#include <utility>

using json = nlohmann::json;
using namespace std;
using namespace DirectX;

namespace
{
	constexpr int kStageVersion = 1;

	// JSONの数値配列を検証して読み込む（配列の長さは型から取得）
	template<size_t N>
	void ReadFloatArray(
		const json& data,
		float(&values)[N],
		const char* field)
	{
		if (!data.is_array() || data.size() != N)
			throw runtime_error(string(field) +
				" has an invalid array size.");

		for (size_t i = 0; i < N; ++i)
		{
			values[i] = data.at(i).get<float>();
			if (!isfinite(values[i]))
				throw runtime_error(string(field) +
					" contains a non-finite number.");
		}
	}

	const char* ShapeToString(PrimitiveShape shape)
	{
		switch (shape)
		{
		case PrimitiveShape::Box: return "Box";
		case PrimitiveShape::Cylinder: return "Cylinder";
		case PrimitiveShape::Sphere: return "Sphere";
		default: throw runtime_error("Unknown PrimitiveShape.");
		}
	}

	PrimitiveShape StringToShape(const string& name)
	{
		if (name == "Box") return PrimitiveShape::Box;
		if (name == "Cylinder") return PrimitiveShape::Cylinder;
		if (name == "Sphere") return PrimitiveShape::Sphere;
		throw runtime_error("Unknown PrimitiveShape: " + name);
	}

	// TransformとComponentをそれぞれ独立して保存・復元する
	json SerializeTransform(const Transform& t)
	{
		return {
			{"position", {
				t.position[0],
				t.position[1],
				t.position[2]
			}},
			{"rotation", {
				t.rotation[0],
				t.rotation[1],
				t.rotation[2]
			}},
			{"scale", {
				t.scale[0],
				t.scale[1],
				t.scale[2]
			}}
		};
	}

	void DeserializeTransform(const json& data, Transform& t)
	{
		ReadFloatArray(data.at("position"), t.position, "position");
		ReadFloatArray(data.at("rotation"), t.rotation, "rotation");
		ReadFloatArray(data.at("scale"), t.scale, "scale");
	}

	json SerializeComponents(const GameObject& object)
	{
		json result = json::array();

		for (const auto& pComponent : object.GetComponents())
		{
			if (!pComponent)
				throw runtime_error("Null Component.");

			const auto* pEntry = ComponentRegistry::Find(pComponent->GetTypeId());

			if (!pEntry)
				throw runtime_error("Unregistered Component: " +
					string(pComponent->GetTypeId()));

			json data;
			data["type"] = pEntry->typeId;
			data["enabled"] = pComponent->IsEnabled();
			pEntry->save(*pComponent, data);
			result.push_back(move(data));
		}

		return result;
	}

	void DeserializeComponents(const json& data, GameObject& object)
	{
		if (!data.is_array())
			throw runtime_error("components must be an array.");

		// Goalなどの初期Componentは保存内容で置き換える
		object.ClearComponents();
		for (const auto& componentData : data)
		{
			const string type = componentData.at("type").get<string>();
			const auto* pEntry = ComponentRegistry::Find(type);
			if (!pEntry)
				throw runtime_error("Unknown Component type: " + type);

			auto pComponent = pEntry->create(&object);
			if (!pComponent)
				throw runtime_error("Component creation failed: " + type);

			pEntry->load(*pComponent, componentData);
			pComponent->SetEnabled(componentData.at("enabled").get<bool>());

			if (!object.AddComponent(move(pComponent)))
				throw runtime_error("Invalid or duplicate Component: " + type);
		}
	}

	// GameObject 1個分のJSONを必ず戻り値として返す
	json SerializeObject(const GameObject& object)
	{
		const string type =
			GameObjectFactory::GetTypeName(object);

		if (type.empty())
			throw runtime_error("Unsupported GameObject type.");
		if (object.GetName().empty())
			throw runtime_error("GameObject name is empty.");

		json data;

		data["type"] = type;
		data["name"] = object.GetName();

		//========================================
		// Primitiveの保存
		//========================================
		if (const auto* pPrimitive =
			dynamic_cast<const PrimitiveObject*>(&object))
		{
			data["shape"] = ShapeToString(pPrimitive->GetShape());
		}

		//========================================
		// FBXの保存
		//========================================
		if (const auto* pFbx =
			dynamic_cast<const FbxObject*>(&object))
		{
			if (pFbx->GetModelPath().empty())
			{
				throw runtime_error("FBX model path is empty.");
			}

			data["modelPath"] = pFbx->GetModelPath();
		}

		//========================================
		// 色の保存
		//========================================
		const auto& color = object.GetColor();

		for (float value : {color.x, color.y, color.z, color.w})
		{
			if (!(value >= 0.0f && value <= 1.0f))
				throw runtime_error("color values must be between 0 and1.");
		}

		data["color"] = { color.x, color.y, color.z, color.w };

		//========================================
		// Transform・Componentの保存
		//========================================
		data["transform"] = 
			SerializeTransform(object.GetTransform());
		data["components"] = 
			SerializeComponents(object);

		//========================================
		// JSONを返す
		//========================================
		return data;
	}

	unique_ptr<GameObject> DeserializeObject(const json& data)
	{
		const string type = data.at("type").get<string>();
		auto pObject = GameObjectFactory::Create(type);
		if (!pObject)
			throw runtime_error("Unknown GameObject type: " + type);
	
		const string name = data.at("name").get<string>();
		if (name.empty())
			throw runtime_error("GameObject name is empty.");
		pObject->SetName(name);
	
		if (auto* pFbx = dynamic_cast<FbxObject*>(pObject.get()))
		{
			const string path = data.at("modelPath").get<string>();
			if (!pFbx->SetModelPath(path))
				throw runtime_error("Failed to load FBX: " + path);
		}
	
		if (auto* pPrimitive = dynamic_cast<PrimitiveObject*>(pObject.get()))
		{
			pPrimitive->SetShape(
				StringToShape(data.value("shape", string("Box"))));
		}
	
		XMFLOAT4 color(1, 1, 1, 1);
		if (data.contains("color"))
		{
			float rgba[4];
			ReadFloatArray(data.at("color"), rgba, "color");
	
			for (float value : rgba)
			{
				if (value < 0.0f || value > 1.0f)
					throw runtime_error("color values must be between 0 and 1.");
			}
	
			color = XMFLOAT4(rgba[0], rgba[1], rgba[2], rgba[3]);
		}
	
		pObject->SetColor(color);
		DeserializeTransform(data.at("transform"), pObject->GetTransform());
		DeserializeComponents(data.at("components"), *pObject);
		pObject->SaveInitialTransform();
	
		return pObject;
	}

	json SerializeCamera(const CameraPlay& camera)
	{
		const auto& pos = camera.GetPosition();
		const auto& look = camera.GetLook();
		
		return {
			{"position", {
				pos.x,
				pos.y,
				pos.z
			}},
			{"look", {
				look.x,
				look.y,
				look.z
			}}
		};
	}

	struct CameraPose
	{
		XMFLOAT3 position;
		XMFLOAT3 look;
	};

	CameraPose DeserializeCamera(const json& root, const CameraPlay& camera)
	{
		CameraPose pose
		{
			camera.GetPosition(),
			camera.GetLook()
		};
		if (!root.contains("playCamera")) return pose;
		const auto& data = root.at("playCamera");
		float pos[3], look[3];
		ReadFloatArray(data.at("position"), pos, "playCamera.position");
		ReadFloatArray(data.at("look"), look, "playCamera.look");
		
		// 既存のカメラ方向の制約を維持
		const float dx = look[0] - pos[0];
		const float dz = look[2] - pos[2];
		if (!(dx * dx + dz * dz > 0.0001f))
			throw runtime_error("Invalid Play Camera direction.");
		pose.position = XMFLOAT3(pos[0], pos[1], pos[2]);
		pose.look = XMFLOAT3(look[0], look[1], look[2]);

		return pose;
	}

	// NaN/InfはJSON化するとnullになるため書き込み前に拒否する
	void ValidateFiniteNumbers(const json& data)
	{
		if (data.is_number_float() && !isfinite(data.get<double>()))
			throw runtime_error("Stage containes a non-finite number.");
		if (data.is_structured())
		{
			for (const auto& value : data)
			{
				ValidateFiniteNumbers(value);
			}
		}
	}

	json ReadStageFile(const string& path)
	{
		ifstream file(path, ios::binary);
		if (!file)
			throw runtime_error("Cannot open stage file: " + path);

		return json::parse(file);
	}

	// 一時ファイルへ書き込み、成功した場合だけ本ファイルを置換する
	// 単一エディタ・同一ステージへの逐次保存を前提とする
	void WriteStageFile(const string& path, const string& text)
	{
		const string tempPath = path + ".tmp";
		try
		{
			ofstream file(tempPath, ios::binary | ios::trunc);
			if (!file) throw runtime_error("Cannot open temporary stage: " + tempPath);
			file.write(text.data(), static_cast<streamsize>(text.size()));
			file.flush();
			file.close();
			if (!file) throw runtime_error("Failed to write temporary stage: " + tempPath);

			//========================================
			// ・MoveFileExA()
			// 　書き込みとファイルの置換を分離できる
			//========================================
			if (!MoveFileExA(tempPath.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
			{
				const DWORD code = GetLastError();
				throw runtime_error("Failed to replace stage. WIn32 error: " + to_string(code));
			}
		}
		catch (...)
		{
			DeleteFileA(tempPath.c_str());
			throw;
		}
	}
}




//========================================
// JSON保存処理
//========================================
bool SceneSerializer::Save(
	const string& filePath,
	const vector<unique_ptr<GameObject>>& objects,
	const CameraPlay& playCamera, string& error)
{
	error.clear();
	try
	{
		// JSON全体を作成
		json root;

		// 保存形式のバージョン
		root["version"] = kStageVersion;

		// GameObject一覧
		root["objects"] = json::array();

		unordered_set<string> names;
		for (const auto& pObject : objects)
		{
			if (!pObject)
				throw runtime_error("Null GameObject.");
			if (!names.insert(pObject->GetName()).second)
				throw runtime_error("Duplicate GameObject name: " +
					pObject->GetName());

			// 生成と配列への追加を1行にまとめ、追加忘れを防ぐ
			root["objects"].push_back(SerializeObject(*pObject));
		}

		root["playCamera"] = SerializeCamera(playCamera);
		ValidateFiniteNumbers(root);
		DeserializeCamera(root, playCamera);
		WriteStageFile(filePath, root.dump(4));

		return true;
	}
	catch (const std::exception& e)
	{
		error = e.what();
		return false;
	}
}




//========================================
// JSON読み込み処理
//========================================
bool SceneSerializer::Load(
	const string& filePath,
	vector<unique_ptr<GameObject>>& objects,
	CameraPlay& playCamera,
	string& error)
{
	error.clear();
	try
	{
		const json root = ReadStageFile(filePath);
		if (root.at("version").get<int>() != kStageVersion)
			throw runtime_error("Unsupported stage version.");

		const auto& savedObjects = root.at("objects");
		if (!savedObjects.is_array())
			throw runtime_error("objects must be an array.");

		vector<unique_ptr<GameObject>> loadedObjects;
		loadedObjects.reserve(savedObjects.size());
		unordered_set<string> names;
		for (const auto& data : savedObjects)
		{
			auto pObject = DeserializeObject(data);
			if (!names.insert(pObject->GetName()).second)
				throw runtime_error("Duplicate GameObject name: " + 
					pObject->GetName());
			loadedObjects.push_back(move(pObject));
		}

		const auto pose = DeserializeCamera(root, playCamera);

		//========================================
		// 全て成功するまで現在のシーンには触れない
		//========================================
		playCamera.SetPose(pose.position, pose.look);
		objects.swap(loadedObjects);

		return true;
	}
	catch (const exception& e)
	{
		error = e.what();
		return false;
	}
}