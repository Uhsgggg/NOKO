#pragma once

#ifdef _DEBUG
#include <vector>
#include <memory>
#include <string>

#include "Source/Game/Editor/EditorCommand.h"

class GameObject;
class CameraPlay;

// 編集命令を実行した結果
// 
// 実行した結果、「何が変わったか」
// 「失敗したか」という報告用構造体
struct SceneEditResult
{
	// Hierarchyの選択番号を更新するか
	bool selectionChanged = false;

	// 新しく選択するオブジェクト番号
	int selectedIndex = -1;

	// エラーがなければ空文字
	std::string error;
};

// エディタの操作を実行するクラス
class SceneEditActions
{
public:
	static SceneEditResult Execute(
		const EditorCommand& command,
		std::vector<std::unique_ptr<GameObject>>& objects,
		CameraPlay& playCamera,
		const std::string& stageFilePath
	);
};
#endif