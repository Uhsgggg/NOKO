#pragma once

#include <memory>
#include <string>
#include <unordered_map>

class StaticFbxModel;

class ModelManager
{
public:
	// FBXを読み込む。読み込み済みなら再利用する
	std::shared_ptr<StaticFbxModel> Load(
		const std::string& filePath
	);

private:
	// ファイルパスとモデルデータを関連付ける
	std::unordered_map<
		std::string,
		std::shared_ptr<StaticFbxModel>
	> model;
};

