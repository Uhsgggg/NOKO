#include "SceneRayPicker.h"

#ifdef _DEBUG

#include "Engine/Scene/GameObject.h"
#include "Engine/Camera/Camera.h"

#include "Game/Objects/PrimitiveObject.h"

#include <DirectXMath.h>
#include <DirectXCollision.h>

#include <cfloat>
#include <cmath>

using namespace DirectX;
using namespace std;

int SceneRayPicker::Pick(
	const vector<unique_ptr<GameObject>>& objects,
	Camera& camera,
	float mouseX,
	float mouseY,
	float viewportWidth,
	float viewportHeight)
{
	// 画面サイズが無効なら判定しない
	if (viewportWidth <= 0.0f || viewportHeight <= 0.0f)
	{
		return -1;
	}

	//========================================
	// カメラ行列を取得
	//========================================
	// 転置前の行列を使う
	XMFLOAT4X4 viewData =
		camera.GetViewMatrix(false);
	XMFLOAT4X4 projectionData =
		camera.GetProjectionMatrix(false);
	XMMATRIX view =
		XMLoadFloat4x4(&viewData);
	XMMATRIX projection =
		XMLoadFloat4x4(&projectionData);
	XMMATRIX identity =
		XMMatrixIdentity();

	//========================================
	// 画面座標から3DのRayを作る
	//========================================
	// 
	// XMVector3Unproject : 画面上の2D座標を、
	// 　カメラ行列を使って3D座標へ逆変換する関数。
	// 
	// XMMatrixInverse : World行列の逆行列を計算する関数。
	// 
	// 画面上の同じ位置で、奥行だけ変える
	XMVECTOR nearPoint = XMVectorSet(
		mouseX, mouseY, 0.0f, 1.0f
	);
	XMVECTOR farPoint = XMVectorSet(
		mouseX, mouseY, 1.0f, 1.0f
	);

	// ニアクリップ面の3D座標
	XMVECTOR rayNear = XMVector3Unproject(
		nearPoint,
		0.0f, 0.0f,
		viewportWidth, viewportHeight,
		0.0f, 1.0f,
		projection, view, identity
	);

	// ファークリップ面の3D座標
	XMVECTOR rayFar = XMVector3Unproject(
		farPoint,
		0.0f, 0.0f,
		viewportWidth, viewportHeight,
		0.0f, 1.0f,
		projection, view, identity
	);

	// Rayの長さの2乗
	float rayLengthSq = XMVectorGetX(
		XMVector3LengthSq(
			XMVectorSubtract(rayFar, rayNear)
		)
	);

	//========================================
	// 最も手前のオブジェクトを探す
	//========================================
	int selectedIndex = -1;
	float nearestDistanceSq = FLT_MAX;

	// GroundとGoalは
	// ローカル座標で一辺1.0の立方体
	BoundingBox localBox(
		XMFLOAT3(0.0f, 0.0f, 0.0f),
		XMFLOAT3(0.5f, 0.5f, 0.5f)
	);

	BoundingSphere localSphere(
		XMFLOAT3(0.0f, 0.0f, 0.0f),
		0.5f
	);

	for (int i = 0;
		i < static_cast<int>(objects.size());
		++i)
	{
		GameObject* pObject = objects[i].get();

		if (!pObject) continue;

		//========================================
		// オブジェクトのWorld行列
		//========================================
		XMMATRIX world =
			pObject->GetTransform().GetWorldMatrix();
		XMVECTOR determinant =
			XMMatrixDeterminant(world);

		// Scaleが0などで逆行列を作れない場合
		if (fabs(XMVectorGetX(determinant)) < 0.000001f) continue;

		// World行列の逆行列
		XMMATRIX inverseWorld =
			XMMatrixInverse(&determinant, world);

		//========================================
		// Rayをローカル座標へ変換
		//========================================
		XMVECTOR localNear =
			XMVector3TransformCoord(
				rayNear, inverseWorld
			);
		XMVECTOR localFar =
			XMVector3TransformCoord(
				rayFar, inverseWorld
			);
		XMVECTOR localDirection =
			XMVectorSubtract(localFar, localNear);

		float localLengthSq = XMVectorGetX(
			XMVector3LengthSq(localDirection)
		);

		if (localLengthSq < 0.00000001f) continue;

		localDirection =
			XMVector3Normalize(localDirection);

		//========================================
		// 形状に応じた交差判定
		//========================================
		float hitDistance = 0.0f;
		bool isHit = false;

		// PrimitiveObjectかどうか調べる
		const PrimitiveObject* pPrimitive =
			dynamic_cast<const PrimitiveObject*>(pObject);

		// 球の場合はBoundingSphereで判定
		if (pPrimitive && pPrimitive->GetShape() == PrimitiveShape::Sphere)
		{
			isHit = localSphere.Intersects(
				localNear,
				localDirection,
				hitDistance
			);
		}
		else
		{
			// Box・Cylinder・従来のGround/Goal
			isHit = localBox.Intersects(
				localNear,
				localDirection,
				hitDistance
			);
		}

		// 何も当たらなかった場合
		if (!isHit) continue;

		if (!localBox.Intersects(
			localNear,
			localDirection,
			hitDistance))
		{
			continue;
		}

		// ローカル座標の交点
		XMVECTOR localHit = XMVectorAdd(
			localNear,
			XMVectorScale(localDirection, hitDistance)
		);

		// ワールド座標に戻す
		XMVECTOR worldHit =
			XMVector3TransformCoord(
				localHit, world
			);

		// Rayの始点から交点までの距離の2乗
		float distanceSq = XMVectorGetX(
			XMVector3LengthSq(
				XMVectorSubtract(worldHit, rayNear)
			)
		);

		// カメラのFarより奥なら選択しない
		if (distanceSq > rayLengthSq) continue;

		//========================================
		// 最も手前のオブジェクトを記録
		//========================================
		if (distanceSq < nearestDistanceSq)
		{
			nearestDistanceSq = distanceSq;
			selectedIndex = i;
		}
	}

	return selectedIndex;
}
#endif
