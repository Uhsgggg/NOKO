#include "ModelManager.h"
#include "StaticFbxModel.h"

using namespace std;

//========================================
// ModelManagerの取得
//========================================
ModelManager& ModelManager::GetInstance()
{
	static ModelManager instance;
	return instance;
}

//========================================
// FBXモデルの読み込み
//========================================
shared_ptr<StaticFbxModel> ModelManager::Load(const string& filePath)
{
	if (filePath.empty())
	{
		return nullptr;
	}

	//========================================
	// 読み込み済みか確認
	//========================================
	auto it = m_models.find(filePath);

	if (it != m_models.end())
	{
		shared_ptr<StaticFbxModel> pExisting =
			it->second.lock();

		if (pExisting)
		{
			return pExisting;
		}
	}

	//========================================
	// 新しいモデルを作成
	//========================================
	shared_ptr<StaticFbxModel> pModel(new StaticFbxModel());

	//========================================
	// FBXを読み込む
	//========================================
	if (!pModel->Load(filePath.c_str()))
	{
		return nullptr;
	}

	//========================================
	// 読み込み済みモデルとして記録
	//========================================
	m_models[filePath] = pModel;

	return pModel;
}







