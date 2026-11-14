#pragma once

#include "Engine/Scene/Component.h"
#include "ThirdParty/nlohmann/json.hpp"

#include <memory>
#include <string>
#include <vector>

class GameObject;

class ComponentRegistry
{
public:
	// コンポーネント1種類分の登録情報
	struct Entry
	{
		// JSON保存用のID
		const char* typeId;

		// Inspectorの表示名
		const char* displayName;

		// コンポーネント生成関数
		// ・関数ポインタ … 戻り値の型 (*ポインタ変数名)(引数)
		// 
		// 　GameObjectのポインタを受け取り、
		// 　Componentのunique_ptrを返す関数。
		std::unique_ptr<Component> (*create)(GameObject*);

		// JSON保存関数
		void (*save)(const Component&, nlohmann::json&);

		// JSON読込関数
		void (*load)(Component&, const nlohmann::json&);
	};

	// 登録されている全種類を取得
	static const std::vector<Entry>& GetEntries();

	// IDから登録情報を探す
	static const Entry* Find(const std::string& typeId);
};