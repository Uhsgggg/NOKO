#include "StaticFbxModel.h"

#include "Source/Engine/Graphics/DirectX/ShaderList.h"
#include "Source/Engine/Graphics/DirectX/Texture.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/material.h>

#include <Windows.h>
#include <utility>
#include <string>

using namespace DirectX;

namespace
{
	// FBXから読み込んだ頂点情報を保存するための構造体
	struct FbxVertex
	{// メモ:１頂点 = 48 byte
		XMFLOAT3 pos;		// 頂点座標
		XMFLOAT3 normal;	// 頂点法線
		XMFLOAT2 uv;		// テクスチャ座標
		XMFLOAT4 color;		// 頂点カラー
	};

	void ShowFbxError(const std::string& message)
	{
		MessageBoxA
		(
			nullptr,
			message.c_str(),
			"FBX Load Error",
			MB_OK | MB_ICONERROR
		);
	}
}

bool StaticFbxModel::Load(const char* pFilePath)
{
	m_meshes.clear();
	m_materialTextures.clear();
	m_materialDiffuseColors.clear();
	m_materialAmbientColors.clear();
	m_materialSpecularColors.clear();
	m_meshMaterialIndices.clear();

	Assimp::Importer importer;

	const unsigned int flags =
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_GenNormals |
		aiProcess_PreTransformVertices |
		aiProcess_ConvertToLeftHanded;

	// パスが指定されていない場合
	if (!pFilePath || pFilePath[0] == '\0')
	{
		ShowFbxError("FBX file path is empty.");
		return false;
	}

	// FBXの読み込み
	const aiScene* pScene =
		importer.ReadFile(pFilePath, flags);

	// 読み込み失敗
	if (!pScene)
	{
		std::string message = "FBX Load Failed.\n\n";

		message += pFilePath;
		message += "\n\n";
		message += importer.GetErrorString();

		ShowFbxError(message);
		return false;
	}

	// メッシュがない場合
	if (!pScene->HasMeshes())
	{
		ShowFbxError("FBX contains no meshes.");
		return false;
	}
	

	//========================================
	// マテリアルから画像を取得する
	//========================================
	m_materialTextures.resize(pScene->mNumMaterials);

	for (unsigned int i = 0;
		i < pScene->mNumMaterials;
		++i)
	{
		const aiMaterial* pMaterial =
			pScene->mMaterials[i];

		aiString texturePath;

		// 拡散色テクスチャの参照を取得
		aiReturn result = pMaterial->GetTexture(
			aiTextureType_DIFFUSE,
			0,
			&texturePath
		);

		// PBRマテリアルの場合も確認
		if (result != AI_SUCCESS)
		{
			result = pMaterial->GetTexture(
				aiTextureType_BASE_COLOR,
				0,
				&texturePath
			);
		}

		if (result != AI_SUCCESS)
			continue;

		std::unique_ptr<Texture> pTexture(new Texture());

		// FBX内に埋め込まれた画像を探す
		const aiTexture* pEmbedded =
			pScene->GetEmbeddedTexture(
				texturePath.C_Str()
			);

		HRESULT hr = E_FAIL;

		if (pEmbedded != nullptr)
		{
			// JPEG/PNGなどの圧縮形式を読み込む
			if (pEmbedded->mHeight == 0)
			{
				hr = pTexture->CreateFromMemory(
					pEmbedded->pcData,
					pEmbedded->mWidth
				);
			}
		}
		else
		{
			std::string fileName = texturePath.C_Str();

			// FBXファイルが保存されているフォルダー
			std::string fbxPath = pFilePath;

			size_t folderPos = fbxPath.find_last_of("/\\");

			std::string folder =
				folderPos == std::string::npos
				? ""
				: fbxPath.substr(0, folderPos + 1);

			// まずFBXと同じフォルダーから探す
			std::string path = folder + fileName;

			hr = pTexture->Create(path.c_str());

			if (FAILED(hr))
			{
				path = "Assets/Model/" + fileName;

				hr = pTexture->Create(path.c_str());
			}
		}

		m_materialTextures[i] = std::move(pTexture);
	}

	//========================================
	// マテリアル情報を読み込む
	//========================================
	// 拡散色
	m_materialDiffuseColors.resize(
		pScene->mNumMaterials,
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f)
	);

	// 環境光成分
	m_materialAmbientColors.resize(
		pScene->mNumMaterials,
		XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f)
	);

	// 鏡面反射色・光沢
	m_materialSpecularColors.resize(
		pScene->mNumMaterials,
		XMFLOAT4(1.0f, 1.0f, 1.0f, 16.0f)
	);

	for (unsigned int i = 0; i < pScene->mNumMaterials; ++i)
	{
		const aiMaterial* pMaterial = pScene->mMaterials[i];

		aiColor3D color;

		// 拡散色
		if (pMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS)
		{
			m_materialDiffuseColors[i] = XMFLOAT4(
				color.r, color.g, color.b, 1.0f
			);
		}

		// 環境光成分
		if (pMaterial->Get(AI_MATKEY_COLOR_AMBIENT, color) == AI_SUCCESS)
		{
			m_materialAmbientColors[i] = XMFLOAT4(
				color.r, color.g, color.b, 1.0f
			);
		}

		// 鏡面反射色
		if (pMaterial->Get(AI_MATKEY_COLOR_SPECULAR, color) == AI_SUCCESS)
		{
			m_materialSpecularColors[i].x = color.r;
			m_materialSpecularColors[i].y = color.g;
			m_materialSpecularColors[i].z = color.b;
		}

		// 光沢の鋭さ
		float shininess = 16.0f;
		if (pMaterial->Get(AI_MATKEY_SHININESS, shininess) == AI_SUCCESS)
		{
			if (shininess > 0.0f)
			{
				m_materialSpecularColors[i].w = shininess;
			}
		}

		// 鏡面反射の強さ
		float specularStrength = 1.0f;
		if (pMaterial->Get(AI_MATKEY_SHININESS_STRENGTH, specularStrength) == AI_SUCCESS)
		{
			if (specularStrength >= 0.0f)
			{
				m_materialSpecularColors[i].x *= specularStrength;
				m_materialSpecularColors[i].y *= specularStrength;
				m_materialSpecularColors[i].z *= specularStrength;
			}
		}
	}

	//========================================
	// メッシュ変換処理
	//========================================
	for (unsigned int meshIndex = 0;
		meshIndex < pScene->mNumMeshes;
		++meshIndex)
	{
		const aiMesh* pMesh =
			pScene->mMeshes[meshIndex];

		if (!pMesh || !pMesh->HasPositions()) continue;

		std::vector<FbxVertex> vertices(pMesh->mNumVertices);

		// メッシュをひとつずつ取り出す処理
		for (unsigned int i = 0; i < pMesh->mNumVertices; ++i)
		{
			// 座標
			const aiVector3D& pos = pMesh->mVertices[i];
			FbxVertex& vertex = vertices[i];

			vertex.pos = XMFLOAT3(pos.x, pos.y, pos.z);

			// 法線
			if (pMesh->HasNormals())
			{
				const aiVector3D& normal = pMesh->mNormals[i];
				vertex.normal = XMFLOAT3(normal.x, normal.y, normal.z);
			}
			else
			{
				vertex.normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
			}

			// UV座標
			if (pMesh->HasTextureCoords(0))
			{
				const aiVector3D& uv = pMesh->mTextureCoords[0][i];
				vertex.uv = XMFLOAT2(uv.x, uv.y);
			}
			else
			{
				vertex.uv = XMFLOAT2(0.0f, 0.0f);
			}

			// 頂点カラー
			vertex.color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		}

		// インデックス取得処理を追加
		std::vector<unsigned int> indices;

		for (unsigned int i = 0; i < pMesh->mNumFaces; ++i)
		{
			const aiFace& face = pMesh->mFaces[i];

			if (face.mNumIndices != 3) continue;

			indices.push_back(face.mIndices[0]);
			indices.push_back(face.mIndices[1]);
			indices.push_back(face.mIndices[2]);
		}
		if (indices.empty()) continue;

		//========================================
		// MeshBufferに頂点を渡す
		//========================================
		// MeshBufferの設定
		MeshBuffer::Description desc = {};

		desc.pVtx = vertices.data(); // 頂点配列の先頭アドレス
		desc.vtxSize = sizeof(FbxVertex);
		desc.vtxCount = static_cast<UINT>(vertices.size());
		desc.isWrite = false; // 作成後に頂点を書き換えるか

		desc.pIdx = indices.data(); // インデックス配列の先頭アドレス
		desc.idxSize = sizeof(unsigned int);
		desc.idxCount = static_cast<UINT>(indices.size());

		desc.topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST; // 描画形式

		// GPUバッファを生成する
		std::unique_ptr<MeshBuffer> pBuffer(new MeshBuffer());

		HRESULT hr = pBuffer->Create(desc);
		if (FAILED(hr))
		{
			ShowFbxError("MeshBuffer Create Failed\n");
			m_meshes.clear();
			return false;
		}

		m_meshes.push_back(std::move(pBuffer));
		m_meshMaterialIndices.push_back(pMesh->mMaterialIndex);
	}

	// 1つ以上の描画可能なメッシュを作成できたら trueを返す
	return !m_meshes.empty();
}

//========================================
// 描画関数
//========================================
void StaticFbxModel::Draw(
	const DirectX::XMFLOAT4X4& world,
	const DirectX::XMFLOAT4X4& view,
	const DirectX::XMFLOAT4X4& projection)
{
	if (m_meshes.empty()) return;

	//========================================
	// World, View, Projection行列
	//========================================
	XMFLOAT4X4 matrices[3] =
	{
		world,
		view,
		projection
	};

	ShaderList::SetWVP(matrices);

	//========================================
	// カメラ位置を取得
	//========================================
	// View行列は転置済みなので元へ戻す
	XMMATRIX viewMatrix = XMMatrixTranspose(XMLoadFloat4x4(&view));

	// View行列の逆行例
	XMMATRIX inverseView = XMMatrixInverse(nullptr, viewMatrix);

	// カメラのワールド座標
	XMFLOAT3 cameraPos;
	XMStoreFloat3(&cameraPos, inverseView.r[3]);

	ShaderList::SetCameraPos(cameraPos);

	//========================================
	// シェーダーを設定
	//========================================
	VertexShader* pVS = ShaderList::GetVS(ShaderList::VS_WORLD);
	PixelShader* pPS = ShaderList::GetPS(ShaderList::PS_SPECULAR);

	pVS->Bind();
	pPS->Bind();

	//========================================
	// メッシュごとに描画
	//========================================
	for (size_t i = 0; i < m_meshes.size(); ++i)
	{
		Model::Material material = {};

		// マテリアル初期値
		material.diffuse  = XMFLOAT4(1.0f, 1.0f, 1.0f,  1.0f);
		material.ambient  = XMFLOAT4(0.3f, 0.3f, 0.3f,  1.0f);
		material.specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 16.0f);
		material.pTexture = nullptr;

		// メッシュのマテリアル番号
		unsigned int materialIndex = m_meshMaterialIndices[i];

		// Mayaの拡散色
		if (materialIndex < m_materialDiffuseColors.size())
		{
			material.diffuse =
				m_materialDiffuseColors[materialIndex];
		}

		// Mayaの環境光成分
		if (materialIndex < m_materialAmbientColors.size())
		{
			material.ambient =
				m_materialAmbientColors[materialIndex];
		}

		// Mayaの鏡面反射色・光沢
		if (materialIndex < m_materialSpecularColors.size())
		{
			material.specular =
				m_materialSpecularColors[materialIndex];
		}

		// Mayaの画像テクスチャ
		if (materialIndex < m_materialTextures.size())
		{
			material.pTexture =
				m_materialTextures[materialIndex].get();
		}

		// シェーダーへマテリアルを渡す
		ShaderList::SetMaterial(material);

		// メッシュを描画
		m_meshes[i]->Draw();
	}
}
