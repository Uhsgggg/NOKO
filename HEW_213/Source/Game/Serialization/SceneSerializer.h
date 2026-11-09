#pragma once

#include <vector>
#include <memory>
#include <string>

class GameObject;
class CameraPlay;

class SceneSerializer
{
public:
	// ステージをJSONファイルに保存
	static bool Save(
		const std::string& filePath,
		const std::vector<std::unique_ptr<GameObject>>& objects,
		const CameraPlay& playCamera,
		std::string& error
	);

	// JSONファイルからステージを復元
	static bool Load(
		const std::string& filePath,
		std::vector<std::unique_ptr<GameObject>>& objects,
		CameraPlay& playCamera,
		std::string& error
	);
};

