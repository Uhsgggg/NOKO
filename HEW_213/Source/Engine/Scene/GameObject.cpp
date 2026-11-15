//========================================//
// 
// GameObject.cpp
// 
//########################################
// コンポーネント更新
// 追加・削除処理
// 重複チェック
// 複数処理
// 詳細な内部実装
//########################################
// 
//========================================//

#include "GameObject.h"

#include <utility>
#include <typeinfo>

void GameObject::Update(float deltaTime)
{
	for (auto& pComponent : m_components)
	{
		// 有効なコンポーネントだけ更新
		if (pComponent && pComponent->IsEnabled())
		{
			pComponent->Update(deltaTime);
		}
	}
}

bool GameObject::AddComponent(std::unique_ptr<Component> pComponent)
{
	// nullptrや、別のGameObjectを所有者にしたComponentは拒否
	// 　Enemyに取り付けたComponentがPlayerの座標を変更してしまう
	// 　みたいな間違いを防げる。
	if (!pComponent || pComponent->GetOwner() != this) return false;

	const char* id = pComponent->GetTypeId();

	if (!id || !*id || HasComponentId(id)) return false;

	// 既存のコンポーネントと型を比較
	for (const auto& pExisting : m_components)
	{
		if (!pExisting) continue;

		// 同じ型なら追加しない
		if (typeid(*pExisting) == typeid(*pComponent))
		{
			return false;
		}
	}

	// 重複していなければ追加する
	m_components.push_back(std::move(pComponent));

	return true;
}

bool GameObject::HasComponentId(const std::string& typeId) const
{
	for (const auto& pComponent : m_components)
	{
		if (pComponent && pComponent->GetTypeId() &&
			typeId == pComponent->GetTypeId()) return true;
	}

	return false;
}

bool GameObject::RemoveComponent(Component* pTarget)
{
	if (pTarget == nullptr) return false;

	// コンポーネントを先頭から調べる
	// ・it はイテレータ：vectorの要素を順番に調べるため
	// 　begin() : 最初の要素
	// 　end() 　: 最後の要素の1つ後ろの位置
	// 杉浦先生の言っていた内容は、
	// 複数の要素を削除したい時に書く
	for (auto it = m_components.begin();
		it != m_components.end(); ++it)
	{
		// 削除対象を見つけたら
		if (it->get() == pTarget)
		{
			// vectorから削除
			m_components.erase(it);

			// 削除したらループから抜けるため
			// 一つ飛ばしにならず、エラーにならない
			return true;
		}
	}

	return false;
}

void GameObject::ClearComponents()
{
	m_components.clear();
}

void GameObject::CopyCommonStateTo(GameObject& destination) const
{
	// 名前をコピー
	destination.m_name = m_name;

	// Transformをコピー
	destination.m_transform = m_transform;

	// Colorをコピー
	destination.m_color = m_color;

	// 複製先のコンポーネントを一度空にする
	destination.m_components.clear();

	// 元のコンポ―ネントを順番に複製
	for (const auto& pComponent : m_components)
	{
		if (!pComponent) continue;

		// 新しい所有者を渡して複製
		std::unique_ptr<Component> pCopy =
			pComponent->Clone(&destination);

		if (pCopy)
		{
			destination.AddComponent(std::move(pCopy));
		}
	}

	// 複製時の状態を初期状態として保存
	destination.SaveInitialTransform();
}