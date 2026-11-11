#pragma once

#include <DirectXMath.h>
#include <memory>
#include <vector>

#include "Engine/Graphics/DirectX/MeshBuffer.h"
#include "Engine/Graphics/DirectX/Texture.h"

class StaticFbxModel
{
public:
	// FBXファイルを読み込む関数
	bool Load(const char* pFilePath);

	void Draw
	(
		const DirectX::XMFLOAT4X4& world,
		const DirectX::XMFLOAT4X4& view,
		const DirectX::XMFLOAT4X4& projection,
		const DirectX::XMFLOAT4& tint = 
			DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

private:
	// メッシュ
	std::vector<std::unique_ptr<MeshBuffer>> m_meshes;

	// マテリアルの画像テクスチャ
	std::vector<std::unique_ptr<Texture>> m_materialTextures;

	// マテリアルの拡散色
	std::vector<DirectX::XMFLOAT4> m_materialDiffuseColors;

	// マテリアルの環境光成分
	std::vector<DirectX::XMFLOAT4> m_materialAmbientColors;

	// マテリアルの鏡面反射色・光沢
	std::vector<DirectX::XMFLOAT4> m_materialSpecularColors;

	// メッシュごとのマテリアル番号
	std::vector<unsigned int> m_meshMaterialIndices;
};

