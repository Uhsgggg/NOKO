#include <DirectXMath.h>

#include "Ground.h"
#include "Source/Engine/Graphics/Geometory.h"

using namespace DirectX;
using namespace std;

Ground::Ground()
	: GameObject("Ground")
{
}

Ground::~Ground()
{
}

void Ground::Draw()
{
	// GameObjectが持つTransformからワールド行列を取得
	XMMATRIX world =
		GetTransform().GetWorldMatrix();

	// シェーダーへ渡すために転置
	world = XMMatrixTranspose(world);

	XMFLOAT4X4 worldMatrix;
	XMStoreFloat4x4(&worldMatrix, world);

	// 描画処理
	Geometory::SetWorld(worldMatrix);
	Geometory::DrawBox();
}

// 座標の設定
void Ground::SetPos(float x, float y, float z)
{
	Transform& transform = GetTransform();

	transform.position[0] = x;
	transform.position[1] = y;
	transform.position[2] = z;
}

// 拡大率の設定
void Ground::SetScale(float x, float y, float z)
{
	Transform& transform = GetTransform();

	transform.scale[0] = x;
	transform.scale[1] = y;
	transform.scale[2] = z;
}

unique_ptr<GameObject> Ground::Clone() const
{
	// 新しいGroundを生成
	unique_ptr<GameObject> pCopy(new Ground());

	// 共通情報をコピー
	CopyCommonStateTo(*pCopy);

	return pCopy;
}
