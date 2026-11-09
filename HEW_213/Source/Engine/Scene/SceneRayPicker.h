#pragma once

#ifdef _DEBUG
#include <vector>
#include <memory>

class GameObject;
class Camera;

// 3D空間のオブジェクト選択を担当
class SceneRayPicker
{
public:
	// マウス位置からGameObjectを選択
	// 戻り値：
	// 0以上 = 選択したオブジェクト番号
	// -1    = 何にも当たらなかった
	static int Pick(
		const std::vector<std::unique_ptr<GameObject>>& objects,
		Camera& camera,
		float mouseX,
		float mouseY,
		float viewportWidth,
		float viewportHeight
	);
};
#endif

