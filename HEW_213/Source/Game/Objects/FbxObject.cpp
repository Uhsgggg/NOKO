#include "FbxObject.h"

#include "Engine/Graphics/ModelManager.h"

#include <utility>

using namespace DirectX;
using namespace std;

// staticメンバの実体
XMFLOAT4X4 FbxObject::s_view = {};
XMFLOAT4X4 FbxObject::s_projection = {};

//========================================
// コンストラクタ
//========================================
FbxObject::FbxObject()
	: GameObject("FBX Model")
{
}

//========================================
// FBXファイルの設定
//========================================
bool FbxObject::SetModelPath(const string& filePath)
{
	if (filePath.empty())
	{
		return false;
	}

	// 同じモデルなら読み込み直さない
	if (m_modelPath == filePath && m_model)
	{
		return true;
	}

	// ModelManagerからモデルを取得
	shared_ptr<StaticFbxModel> pLoaded =
		ModelManager::GetInstance().Load(filePath);

	if (!pLoaded)
	{// 読み込み失敗時は以前のモデルを維持
		return false;
	}

	// 読み込み成功後に切り替える
	m_model = move(pLoaded);
	m_modelPath = filePath;

	return true;
}

//========================================
// ファイルパス取得
//========================================
const string& FbxObject::GetModelPath() const
{
	return m_modelPath;
}


//========================================
// カメラ行列設定
//========================================
void FbxObject::SetCameraMatrices(
	const XMFLOAT4X4& view, 
	const XMFLOAT4X4& projection)
{
	s_view = view;
	s_projection = projection;
}

//========================================
// 描画
//========================================
void FbxObject::Draw()
{
	if (!m_model) return;

	// TransformからWorld行列を作成
	XMMATRIX world =
		GetTransform().GetWorldMatrix();

	// シェーダーへ渡すために転置
	world = XMMatrixTranspose(world);

	XMFLOAT4X4 worldMatrix;
	XMStoreFloat4x4(&worldMatrix, world);

	// StaticFbxModelで描画
	m_model->Draw(
		worldMatrix,
		s_view,
		s_projection,
		GetColor()
	);
}

//========================================
// 複製
//========================================
unique_ptr<GameObject> FbxObject::Clone() const
{
	unique_ptr<FbxObject> pCopy(new FbxObject());

	// モデルを再読み込みせず共有
	pCopy->m_modelPath = m_modelPath;
	pCopy->m_model = m_model;

	// 名前・Transform・色・Componentをコピー
	CopyCommonStateTo(*pCopy);

	return pCopy;
}