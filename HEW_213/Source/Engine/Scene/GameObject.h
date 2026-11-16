//========================================//
// 
// GameObject.h
// 
// クラスの宣言
// public関数の宣言
// メンバ変数
// テンプレート関数
// 短いGetter / Setter
// 
//========================================//
#pragma once

#include <string>
#include <vector>
#include <memory>

#include <DirectXMath.h>

#include "Engine/Math/Transform.h"
#include "Engine/Scene/Component.h"

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

	// ・所有者のアドレスを変えない。複製はClone()を使用する。
	// 　　GameObjectはComponentを所有しており、
	// 　　Componentは所有者のアドレスを覚えている。
	// 　　そのため、GameObjectそのものを直接コピーしたり、
	// 　　別のアドレスに移動したりすると、所有者のポインタに
	// 　　問題が発生する可能性がある。
	// 
	// 　　ただし、objects.push_back(move(pObject));は使える。
	// 　　これはGameObjectそのものを移動しているわけではなく、
	// 　　GameObjectを所有しているunique_ptrを移動しているだけだから。
	GameObject(const GameObject&) = delete;
	GameObject& operator = (const GameObject&) = delete;
	GameObject(GameObject&&) = delete;
	GameObject& operator = (GameObject&&) = delete;

	// 継承先を安全に破棄するためにvirtualを付ける
	virtual ~GameObject() = default;

	// 共通の更新処理
	// 更新は派生クラスで実装しない
	virtual void Update(float deltaTime) final;

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

	// 実行時の有効状態（JSONには保存しない）
	void SetActive(bool active)
	{
		m_active = active;
	}
	bool IsActive() const
	{
		return m_active;
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
	bool AddComponent(std::unique_ptr<Component> pComponent);

	// 指定した種類のコンポーネントが存在するか確認
	// ・template は型を後から指定できるようする仕組み
	// 　pObject->HasComponent<AutoRotateComponent>();みたいに書くと
	// 　T には AutoRotateComponentが入る。
	template <typename T>
	T* GetComponent()
	{
		// 全コンポーネントを調べる
		for (const auto& pComponent : m_components)
		{
			// 指定した型のコンポーネントか確認
			if (auto* p = dynamic_cast<T*>(pComponent.get())) return p;
		}

		return nullptr;
	}

	// constオブジェクトから読み取り専用のComponentを取得する用
	template <typename T>
	const T* GetComponent() const
	{
		// 全コンポーネントを調べる
		for (const auto& pComponent : m_components)
		{
			// 指定した型のコンポーネントか確認
			if (auto* p = dynamic_cast<T*>(pComponent.get())) return p;
		}

		return nullptr;
	}

	template <typename T>
	bool HasComponent() const
	{
		return GetComponent<T>() != nullptr;
	}

	// RegistryやEditor向け：保存用IDで存在確認する
	bool HasComponentId(const std::string& typeId) const;

	// コンポーネントを削除する
	bool RemoveComponent(Component* pTarget);
	
	//全コンポーネントを取り除く
	void ClearComponents();

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
	void CopyCommonStateTo(GameObject& destination) const;

private:
	std::string m_name;
	bool m_active = true;

	Transform m_transform;
	Transform m_initialTransform;

	// GameObjectがコンポーネントを所有
	std::vector<std::unique_ptr<Component>> m_components;

	// オブジェクトごとの色（RGBA）
	DirectX::XMFLOAT4 m_color;
};