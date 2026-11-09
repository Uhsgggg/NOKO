#include "SceneEditActions.h"

#ifdef _DEBUG
#include "Source/Engine/Scene/GameObject.h"
#include "Source/Engine/Camera/CameraPlay.h"

#include "Source/Game/Objects/GameObjectFactory.h"
#include "Source/Game/Serialization/SceneSerializer.h"

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

