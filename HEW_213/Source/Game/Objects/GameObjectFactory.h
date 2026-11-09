#pragma once

#include <memory>
#include <string>

class GameObject;

class GameObjectFactory
{
public:
	// 種類名からGameObjectを生成する
	// JSON読み込みなど、基本状態で生成したいとき
	static std::unique_ptr<GameObject> Create(
		const std::string& type
	);

	// GameObjectの実際の種類名を取得する
	static std::string GetTypeName(
		const GameObject& object
	);

#ifdef _DEBUG
	// エディタで新規配置する時の初期設定付き生成
	// Hierarchyから新規生成するとき
	static std::unique_ptr<GameObject> CreateForEditor(
		const std::string& type
	);
#endif
};

