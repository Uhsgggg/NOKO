#pragma once

#include <memory>

// 前方宣言
class GameObject;

// すべてのコンポーネントの基底クラス
class Component
{
public:
	// このコンポーネントが所属するGameObjevt
	explicit Component(GameObject* pOwner)
		: m_pOwner(pOwner)
		, m_enabled(true)
	{
	}

	// 継承先の破棄も正しく実行する
	virtual ~Component() = default;

	// 毎フレームの更新処理
	virtual void Update(float deltaTime)
	{
	}

	// コンポーネントの種類名
	virtual const char* GetTypeName() const
	{
		return "Component";
	}

	bool IsEnabled() const
	{
		return m_enabled;
	}

	void SetEnabled(bool enabled)
	{
		m_enabled = enabled;
	}

	// 新しいGameObject用にコンポーネントを複製する
	virtual std::unique_ptr<Component> Clone(
		GameObject* pNewOwner) const = 0;

#ifdef _DEBUG
	// 開発中だけ使うInspector描画
	virtual void DrawDebugInspector()
	{

	}
#endif

protected:
	// コンポーネントを所有しているオブジェクト
	// m_pOwner : そのコンポーネントがどのGameObjectに
	//			　取り付けられているかを示すポインタ。
	//			　所有権を持たないポインタ。
	GameObject* m_pOwner;

private:
	bool m_enabled;
};