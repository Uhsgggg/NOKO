#pragma once

#include <string>
#include <vector>
#include <memory>
#include <utility>
#include <typeinfo>

#include <DirectXMath.h>

#include "Source/Engine/Math/Transform.h"
#include "Source/Engine/Scene/Component.h"

// すべての基本的なゲームオブジェクトの共通クラス
class GameObject
{
public:
	// 名前を受け取って初期化
	explicit GameObject(const std::string& name)
		: m_name(name)
		, m_color(1.0f, 1.0f, 1.0f, 1.0f)
	{

	}

	// 継承先を安全に破棄するためにvirtualを付ける
	virtual ~GameObject() = default;

	// 共通の更新処理
	// 更新は派生クラスで実装しない
	virtual void Update(float deltaTime) final
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

	// 描画は派生クラスで実装する
	virtual void Draw() = 0;

	// 自分自身を複製する
	virtual std::unique_ptr<GameObject> Clone() const = 0;

	// オブジェクト名を取得
	const std::string& GetName() const
	{
		return m_name;
	}

	// GameObjectの名前を変更
	void SetName(const std::string& name)
	{
		m_name = name;
	}

	// Transformを取得
	Transform& GetTransform()
	{
		return m_transform;
	}

	// Transformの初期状態を保存
	void SaveInitialTransform()
	{
		m_initialTransform = m_transform;
	}

	// 保存した初期状態に戻す
	void ResetTransform()
	{
		m_transform = m_initialTransform;
	}

	// 色を設定
	void SetColor(const DirectX::XMFLOAT4& color)
	{
		m_color = color;
	}

	// 現在の色を取得
	const DirectX::XMFLOAT4& GetColor() const
	{
		return m_color;
	}

	// コンポーネントを追加する
	bool AddComponent(std::unique_ptr<Component> pComponent)
	{
		// nullptrは追加できない
		if (!pComponent) return false;

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

	// 指定した種類のコンポーネントが存在するか確認
	// ・template は型を後から指定できるようする仕組み
	// 　pObject->HasComponent<AutoRotateComponent>();みたいに書くと
	// 　T には AutoRotateComponentが入る。
	template <typename T>
	bool HasComponent() const
	{
		// 全コンポーネントを調べる
		for (const auto& pComponent : m_components)
		{
			if (!pComponent) continue;

			// 指定した型のコンポーネントか確認
			if (dynamic_cast<T*>(pComponent.get()) != nullptr)
			{
				return true;
			}
		}

		return false;
	}

	// コンポーネントを削除する
	bool RemoveComponent(Component* pTarget)
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
	
	//全コンポーネントを取り除く
	void ClearComponents()
	{
		m_components.clear();
	}

	// 取り付けられたコンポーネント一覧を取得
	const std::vector<std::unique_ptr<Component>>& GetComponents() const
	{
		return m_components;
	}

	// 読み取り専用
	const Transform& GetTransform() const
	{
		return m_transform;
	}

protected:
	// GameObjectの共通情報を複製先へコピー
	void CopyCommonStateTo(GameObject& destination) const
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

private:
	std::string m_name;

	Transform m_transform;
	Transform m_initialTransform;

	// GameObjectがコンポーネントを所有
	std::vector<std::unique_ptr<Component>> m_components;

	// オブジェクトごとの色（RGBA）
	DirectX::XMFLOAT4 m_color;
};