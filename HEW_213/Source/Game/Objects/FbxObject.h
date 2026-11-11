#pragma once

#include "Engine/Scene/GameObject.h"
#include "Engine/Graphics/StaticFbxModel.h"

#include <DirectXMath.h>
#include <memory>
#include <string>

class FbxObject : public GameObject
{
public:
	FbxObject();
	~FbxObject() override = default;

	// 描画処理
	void Draw() override;

	// 複製処理
	std::unique_ptr<GameObject> Clone() const override;

	// 使用するFBXを変更する
	bool SetModelPath(const std::string& filePath);

	// 使用中のFBXパスを取得
	const std::string& GetModelPath() const;

	// SceneGameからカメラ行列を受け取る
	static void SetCameraMatrices(
		const DirectX::XMFLOAT4X4& view,
		const DirectX::XMFLOAT4X4& projection
	);

private:
	// モデルファイルの場所
	std::string m_modelPath;

	// ModelManagerから取得した共有モデル
	std::shared_ptr<StaticFbxModel> m_model;

	// 同じシーン内で共有のカメラ行列
	static DirectX::XMFLOAT4X4 s_view;
	static DirectX::XMFLOAT4X4 s_projection;
};