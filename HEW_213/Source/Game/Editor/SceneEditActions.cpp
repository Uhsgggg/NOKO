#include "SceneEditActions.h"

#ifdef _DEBUG
#include "Engine/Scene/GameObject.h"
#include "Engine/Camera/CameraPlay.h"

#include "Game/Serialization/SceneSerializer.h"
#include "Game/Objects/GameObjectFactory.h"
#include "Game/Objects/PrimitiveObject.h"
#include "Game/Objects/FbxObject.h"

#include <utility>
#include <string>

using namespace std;

namespace
{
	// 同じ名前が存在しないようにする
	string MakeUniqueObjectName(
		const vector<unique_ptr<GameObject>>& objects,
		const string& baseName)
	{
		string candidate = baseName;
		int number = 1;

		while (true)
		{
			bool exists = false;

			for (const auto& pObject : objects)
			{
				if (pObject &&
					pObject->GetName() == candidate)
				{
					exists = true;
					break;
				}
			}

			// 重複していなければ使用可能
			if (!exists)
			{
				return candidate;
			}

			candidate =
				baseName + "_" + to_string(number);

			++number;
		}
	}
}

SceneEditResult SceneEditActions::Execute(
	const EditorCommand& command, 
	vector<unique_ptr<GameObject>>& objects, 
	CameraPlay& playCamera, 
	const string& stageFilePath)
{
	SceneEditResult result;

	// 新しく追加するオブジェクト
	unique_ptr<GameObject> pNewObject;

	switch (command.action)
	{
	//========================================
	// Ground / Goalの新規生成
	//========================================
	case EditorAction::CreateGround:
	case EditorAction::CreateGoal:
	{
		const string type =
			command.action == EditorAction::CreateGround
			? "Ground" : "Goal";

		pNewObject =
			GameObjectFactory::CreateForEditor(type);

		if (!pNewObject)
		{
			result.error =
				"Failed to create GameObject: " + type;
		}

		break;
	}

	//========================================
	// 基本図形の新規生成
	//========================================
	case EditorAction::CreateBox:
	case EditorAction::CreateCylinder:
	case EditorAction::CreateSphere:
	{
		// 生成する図形の種類
		PrimitiveShape shape = PrimitiveShape::Box;

		// 生成時の基本名
		string name = "Box";

		if (command.action == EditorAction::CreateCylinder)
		{
			shape = PrimitiveShape::Cylinder;
			name = "Cylinder";
		}
		else if (command.action == EditorAction::CreateSphere)
		{
			shape = PrimitiveShape::Sphere;
			name = "Sphere";
		}

		// FactoryからPrimitiveを生成
		pNewObject =
			GameObjectFactory::CreateForEditor("Primitive");

		if (!pNewObject)
		{
			result.error =
				"Failed to create PrimitiveObject.";
			break;
		}

		// PrimitiveObjectとして取得
		PrimitiveObject* pPrimitive =
			dynamic_cast<PrimitiveObject*>(pNewObject.get());

		if (!pPrimitive)
		{
			result.error =
				"Create object is not PrimitiveObject.";

			pNewObject.reset();
			break;
		}

		// 継承と名前を設定
		pPrimitive->SetShape(shape);
		pPrimitive->SetName(name);

		break;
	}

	//========================================
	// FBXモデルを新規生成
	//========================================
	case EditorAction::CreateFbx:
	{
		// GameObjectFactoryから生成
		pNewObject = 
			GameObjectFactory::CreateForEditor("Fbx");

		FbxObject* pFbx = 
			dynamic_cast<FbxObject*>(pNewObject.get());

		if (!pFbx)
		{
			result.error =
				"Failed to create FbxObject.";

			pNewObject.reset();
			break;
		}

		// FBXモデルを設定
		if (!pFbx->SetModelPath(command.modelPath))
		{
			result.error =
				"Failet to load FBX:" + command.modelPath;

			pNewObject.reset();
			break;
		}

		//========================================
		// ファイル名をオブジェクト名にする
		//========================================
		string name = command.modelPath;

		// フォルダ部分を除去
		size_t slash = name.find_last_of("/\\");
		if (slash != string::npos)
		{
			name = name.substr(slash + 1);
		}

		// .fbxを除去
		size_t dot = name.find_last_of('.');
		if (dot != string::npos)
		{
			name = name.substr(0, dot);
		}

		pFbx->SetName(name);

		break;
	}

	//========================================
	// 複製
	//========================================
	case EditorAction::Duplicate:
	{
		const int index = command.objectIndex;

		// 選択番号が正常か確認
		if (index >= 0 &&
			index < static_cast<int>(objects.size()) &&
			objects[index])
		{
			// Clone()で複製
			pNewObject = objects[index]->Clone();

			if (pNewObject)
			{
				// 複製したことが分かるように少しずつずらす
				pNewObject->GetTransform().position[0] 
					+= 1.0f;

				// 複製用の名前
				pNewObject->SetName(
					pNewObject->GetName() + "_Copy"
				);
			}
			else
			{
				result.error =
					"Failed to clone GameObject.";
			}
		}

		break;
	}

	//========================================
	// 削除
	//========================================
	case EditorAction::Delete:
	{
		const int index = command.objectIndex;

		// 選択番号が正常か確認
		if (index >= 0 &&
			index < static_cast<int>(objects.size()) &&
			objects[index])
		{
			// 選択したGameObjectを削除
			objects.erase(objects.begin() + index);

			// 削除後の選択番号を調整
			int nextIndex = index;

			if (nextIndex >=
				static_cast<int>(objects.size()))
			{
				nextIndex =
					static_cast<int>(objects.size()) - 1;
			}

			result.selectionChanged = true;
			result.selectedIndex = nextIndex;
		}

		break;
	}

	//========================================
	// JSONファイルに保存
	//========================================
	case EditorAction::SaveScene:
	{
		string error;

		// 現在のステージをJSONに保存
		if (!SceneSerializer::Save(
			stageFilePath,
			objects,
			playCamera,
			error))
		{
			result.error = "Save failed:" + error;
		}

		break;
	}

	//========================================
	// JSONファイルに読み込み
	//========================================
	case EditorAction::LoadScene:
	{
		string error;

		// 現在のステージにJSONのデータを読み込み
		if (!SceneSerializer::Load(
			stageFilePath,
			objects,
			playCamera,
			error))
		{
			// 読み込み失敗時
			result.error = "Load failed: " + error;
		}
		else
		{
			// 読み込み成功時
			result.selectionChanged = true;

			result.selectedIndex =
				objects.empty() ? -1 : 0;
		}

		break;
	}

	case EditorAction::None:
	default:
		break;
	}

	//========================================
	// 新しいGameObjectの登録
	//========================================
	if (pNewObject)
	{
		// 名前の重複を防ぐ
		const string uniqueName =
			MakeUniqueObjectName(
				objects,
				pNewObject->GetName()
			);

		pNewObject->SetName(uniqueName);

		// 初期Transformを保存
		pNewObject->SaveInitialTransform();

		// vectorに追加
		objects.push_back(move(pNewObject));

		// 新しいオブジェクトを自動選択
		result.selectionChanged = true;
		result.selectedIndex =
			static_cast<int>(objects.size()) - 1;
	}

	return result;
}
#endif

