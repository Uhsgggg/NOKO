#include "GameObjectFactory.h"

#include "Source/Engine/Scene/GameObject.h"
#include "Source/Game/Objects/Ground.h"
#include "Source/Game/Objects/Goal.h"
#include "Source/Game/Objects/PrimitiveObject.h"

#include <memory>

using namespace std;
using namespace DirectX;

unique_ptr<GameObject> GameObjectFactory::Create(
	const string& type)
{
	// Groundを生成
	if (type == "Ground")
	{
		return unique_ptr<GameObject>(new Ground());
	}

	// Goalを生成
	if (type == "Goal")
	{
		return unique_ptr<GameObject>(new Goal());
	}

	// 基本図形オブジェクト
	if (type == "Primitive")
	{
		return unique_ptr<GameObject>(new PrimitiveObject());
	}

	// 未対応の種類
	return nullptr;
}

string GameObjectFactory::GetTypeName(
	const GameObject& object)
{
	// Groundを取得
	if (dynamic_cast<const Ground*>(&object))
	{
		return "Ground";
	}

	// Goalを取得
	if (dynamic_cast<const Goal*>(&object))
	{
		return "Goal";
	}

	// Primitiveを取得
	if (dynamic_cast<const PrimitiveObject*>(&object))
	{
		return "Primitive";
	}

	// 未対応の種類
	return "";
}

#ifdef _DEBUG
unique_ptr<GameObject> GameObjectFactory::CreateForEditor(
	const string& type)
{
	// まず基本状態で生成
	unique_ptr<GameObject> pObject = Create(type);

	if (!pObject) return nullptr;

	Transform& transform = pObject->GetTransform();

	if (type == "Ground")
	{
		transform.position[0] = 1.0f;

		transform.scale[0] = 1.00f;
		transform.scale[1] = 0.01f;
		transform.scale[2] = 1.00f;
	}
	else if (type == "Goal")
	{
		transform.position[0] = 1.0f;
		transform.position[1] = 1.0f;

		transform.scale[0] = 0.5f;
		transform.scale[1] = 0.5f;
		transform.scale[2] = 0.5f;
	}
	else if (type == "Primitive")
	{
		transform.position[0] = 0.0f;
		transform.position[1] = 1.0f;
		transform.position[2] = 0.0f;

		transform.scale[0] = 1.0f;
		transform.scale[1] = 1.0f;
		transform.scale[2] = 1.0f;
	}

	return pObject;
}
#endif