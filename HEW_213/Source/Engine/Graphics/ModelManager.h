#pragma once

#include <memory>
#include <string>
#include <unordered_map>

class StaticFbxModel;

class ModelManager
{
public:
	// 共通のModelManagerを取得
	static ModelManager& GetInstance();

	// FBXを読み込む。読み込み済みなら再利用する
	std::shared_ptr<StaticFbxModel> Load(
		const std::string& filePath
	);

private:
	ModelManager() = default;

	// コピー禁止
	ModelManager(const ModelManager&) = delete;
	ModelManager& operator=(const ModelManager&) = delete;

	// 読み込み済みモデルの管理
	std::unordered_map<
		std::string,
		std::weak_ptr<StaticFbxModel>
	> m_models;
};

