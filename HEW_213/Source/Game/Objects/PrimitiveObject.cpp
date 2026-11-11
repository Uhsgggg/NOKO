#include "PrimitiveObject.h"

#include "Engine/Graphics/Geometory.h"

#include <DirectXMath.h>
#include <memory>

using namespace DirectX;
using namespace std;

//========================================
// コンストラクタ
//========================================
PrimitiveObject::PrimitiveObject(PrimitiveShape shape)
	: GameObject("Primitive")
	, m_shape(shape)
{
}


//========================================
// 描画
//========================================
void PrimitiveObject::Draw()
{
	// TransformからWorld行列を取得
	XMMATRIX world =
		GetTransform().GetWorldMatrix();

	// シェーダー用に転置
	world = XMMatrixTranspose(world);

	XMFLOAT4X4 worldMatrix;
	XMStoreFloat4x4(&worldMatrix, world);

	// 共通のWorld行列を設定
	Geometory::SetWorld(worldMatrix);

	// このオブジェクト自身の色を設定
	Geometory::SetColor(GetColor());

	//========================================
	// 形状ごとに描画を切り替える
	//========================================
	switch (m_shape)
	{
	case PrimitiveShape::Box:
		Geometory::DrawBox();
		break;

	case PrimitiveShape::Cylinder:
		Geometory::DrawCylinder();
		break;

	case PrimitiveShape::Sphere:
		Geometory::DrawSphere();
		break;
	}
}

//========================================
// 複製
//========================================
unique_ptr<GameObject> PrimitiveObject::Clone() const
{
	// 現在と同じ形状を持つオブジェクト
	unique_ptr<GameObject> pCopy(new PrimitiveObject(m_shape));

	// 名前・Transform・Componentを複製
	CopyCommonStateTo(*pCopy);

	return pCopy;
}

