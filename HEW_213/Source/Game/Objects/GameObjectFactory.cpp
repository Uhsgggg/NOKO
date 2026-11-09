#include "GameObjectFactory.h"

#include "Source/Engine/Scene/GameObject.h"
#include "Source/Game/Objects/Ground.h"
#include "Source/Game/Objects/Goal.h"

#include <memory>

using namespace std;
using namespace DirectX;

unique_ptr<GameObject> GameObjectFactory::Create(
	const string& type)
{
	// Ground‚ğ¶¬
	if (type == "Ground")
	{
		return unique_ptr<GameObject>(new Ground());
	}

	// Goal‚ğ¶¬
	if (type == "Goal")
	{
		return unique_ptr<GameObject>(new Goal());
	}

	// –¢‘Î‰‚Ìí—Ş
	return nullptr;
}

string GameObjectFactory::GetTypeName(
	const GameObject& object)
{
	// Ground‚ğæ“¾
	if (dynamic_cast<const Ground*>(&object))
	{
		return "Ground";
	}

	// Goal‚ğæ“¾
	if (dynamic_cast<const Goal*>(&object))
	{
		return "Goal";
	}

	// –¢‘Î‰‚Ìí—Ş
	return "";
}

#ifdef _DEBUG
unique_ptr<GameObject> GameObjectFactory::CreateForEditor(
	const string& type)
{
	// ‚Ü‚¸Šî–{ó‘Ô‚Å¶¬
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

	return pObject;
}
#endif