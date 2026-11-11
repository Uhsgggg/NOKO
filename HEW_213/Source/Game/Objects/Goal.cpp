#include <DirectXMath.h>
#include <memory>
#include <utility>

#include "Goal.h"
#include "App/Defines.h"
#include "Engine/Graphics/Geometory.h"

#include "Game/Components/AutoRotateComponent.h"

using namespace DirectX;
using namespace std;

Goal::Goal()
	: GameObject("Goal")
{
	// 1ラジアン/秒 = 約57.2958度/秒
	const float rotationSpeed = 57.2958f;

	// 自動回転コンポーネントを生成
	// this は現在のGoalオブジェクト自身を指すポインタ
	unique_ptr<Component> pRotate(
		new AutoRotateComponent(
			this,
			rotationSpeed,
			rotationSpeed,
			rotationSpeed
		)
	);

	// Goalにコンポーネントを取り付ける
	AddComponent(move(pRotate));
}

Goal::~Goal()
{
}

// 描画処理
void Goal::Draw()
{
	// Transform から行列を取得
	// m_time を追加回転角として指定
	XMMATRIX world =
		GetTransform().GetWorldMatrix();

	// シェーダー用に転置
	world = XMMatrixTranspose(world);

	XMFLOAT4X4 worldMatrix;
	XMStoreFloat4x4(&worldMatrix, world);

	// 描画処理
	Geometory::SetWorld(worldMatrix);
	Geometory::SetColor(GetColor());

	Geometory::DrawBox();
}

// 座標の設定
void Goal::SetPos(float x, float y, float z)
{
	Transform& transform = GetTransform();

	transform.position[0] = x;
	transform.position[1] = y;
	transform.position[2] = z;
}

// 拡大率の設定
void Goal::SetScale(float x, float y, float z)
{
	Transform& transform = GetTransform();

	transform.scale[0] = x;
	transform.scale[1] = y;
	transform.scale[2] = z;
}

unique_ptr<GameObject> Goal::Clone() const
{
	// 新しいGoalを生成
	unique_ptr<GameObject> pCopy(new Goal());

	// 共通情報をコピー
	CopyCommonStateTo(*pCopy);

	return pCopy;
}