#pragma once

// エディタから発光する命令
enum class EditorAction
{
	None,

	// オブジェクト編集
	CreateGround,
	CreateGoal,
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

	// DuplicateやDeleteの対象番号
	int objectIndex = -1;
};