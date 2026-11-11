#pragma once

#include <string>

// エディタから発光する命令
enum class EditorAction
{
	None,

	// 床・ゴールオブジェクト
	CreateGround,
	CreateGoal,

	// 基本図形オブジェクト
	CreateBox,
	CreateCylinder,
	CreateSphere,

	// FBXオブジェクト
	CreateFbx,

	// 複製・削除
	Duplicate,
	Delete,

	// ステージ保存
	SaveScene,
	LoadScene,
};

// 1回の編集操作を表すデータ
struct EditorCommand
{
	EditorAction action = EditorAction::None;
	std::string modelPath;

	// DuplicateやDeleteの対象番号
	int objectIndex = -1;
};