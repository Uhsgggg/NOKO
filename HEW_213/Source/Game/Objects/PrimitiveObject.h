#pragma once

#include "Source/Engine/Scene/GameObject.h"
#include "Source/Engine/Graphics/PrimitiveShape.h"

// 基本図形を表示するGameObject
class PrimitiveObject : public GameObject
{
public:
	// コンストラクタ
	explicit PrimitiveObject(
		PrimitiveShape shape = PrimitiveShape::Box
	);

	// デストラクタ
	~PrimitiveObject() override = default;

	// 描画
	void Draw() override;

	// 複製
	std::unique_ptr<GameObject> Clone() const override;

	// 形状を設定
	void SetShape(PrimitiveShape shape)
	{
		m_shape = shape;
	}

	// 現在の形状を取得
	PrimitiveShape GetShape() const
	{
		return m_shape;
	}

private:
	// このオブジェクトが使用する形状
	PrimitiveShape m_shape;
};