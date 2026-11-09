#include "Geometory.h"
#include <DirectXMath.h>

#include <vector>
#include <cmath>
#include <Windows.h>

using namespace DirectX;

void Geometory::MakeBox()
{
	//--- 頂点の作成
	Vertex vtx[]
	{	// -Z面
		{ {-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f} }, // 0
		{ { 0.5f,  0.5f, -0.5f}, {1.0f, 0.0f} }, // 1
		{ {-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f} }, // 2
		{ { 0.5f, -0.5f, -0.5f}, {1.0f, 1.0f} }, // 3
		//  0.5f面
		{ { 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f} }, // 4
		{ { 0.5f,  0.5f,  0.5f}, {1.0f, 0.0f} }, // 5
		{ { 0.5f, -0.5f, -0.5f}, {0.0f, 1.0f} }, // 6
		{ { 0.5f, -0.5f,  0.5f}, {1.0f, 1.0f} }, // 7
		//  0.5f面
		{ { 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f} }, // 8
		{ {-0.5f,  0.5f,  0.5f}, {1.0f, 0.0f} }, // 9
		{ { 0.5f, -0.5f,  0.5f}, {0.0f, 1.0f} }, // 10
		{ {-0.5f, -0.5f,  0.5f}, {1.0f, 1.0f} }, // 11
		// -0.5f面
		{ {-0.5f,  0.5f,  0.5f}, {0.0f, 0.0f} }, // 12
		{ {-0.5f,  0.5f, -0.5f}, {1.0f, 0.0f} }, // 13
		{ {-0.5f, -0.5f,  0.5f}, {0.0f, 1.0f} }, // 14
		{ {-0.5f, -0.5f, -0.5f}, {1.0f, 1.0f} }, // 15
		// 0.5f面
		{ {-0.5f,  0.5f,  0.5f}, {0.0f, 0.0f} }, // 16
		{ { 0.5f,  0.5f,  0.5f}, {1.0f, 0.0f} }, // 17
		{ {-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f} }, // 18
		{ { 0.5f,  0.5f, -0.5f}, {1.0f, 1.0f} }, // 19
		// -0.5f面
		{ { 0.5f, -0.5f, -0.5f}, {0.0f, 0.0f} }, // 20
		{ {-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f} }, // 21
		{ { 0.5f, -0.5f,  0.5f}, {0.0f, 1.0f} }, // 22
		{ {-0.5f, -0.5f,  0.5f}, {1.0f, 1.0f} }, // 23
	};

	//--- インデックスの作成
	int idx[] =
	{
		 0,  1,  2,  1,  3,  2, // -Z面
		 4,  5,  6,  5,  7,  6, //  X面
		 8,  9, 10,  9, 11, 10, //  Z面
		12, 13, 14, 13, 15, 14, // -X面
		16, 17, 18, 17, 19, 18, //  Y面
		21, 22, 23, 20, 22, 21  // -Y面
	};

	MeshBuffer::Description desc = {};
	desc.pVtx = vtx;
	desc.vtxCount = _countof(vtx);
	desc.vtxSize = sizeof(Vertex);
	desc.pIdx = idx;
	desc.idxCount = _countof(idx);
	desc.idxSize = sizeof(int);
	desc.topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	// バッファの作成
	m_pBox = new MeshBuffer();
	m_pBox->Create(desc);
}

// 円柱
void Geometory::MakeCylinder()
{
	const float PI = 3.14159265f;
	const int DIV = 36; // 分割数
	const float R = 0.5f; // 半径
	const float HH = 0.5f; // 高さ

	// 頂点とインデックス
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;

	//--- 側面の頂点の作成
	for (int i = 0; i <= DIV; ++i)
	{
		float rad = 2.0f * PI * i / DIV;

		float x = cosf(rad) * R;
		float z = sinf(rad) * R;

		float u = static_cast<float>(i) / DIV;

		// 下側の頂点
		vertices.push_back({
			{x, -HH, z},
			{u, 1.0f} 
			});

		// 上側の頂点
		vertices.push_back({
			{x, HH, z},
			{u, 0.0f} 
			});
	}

	// 天面
	unsigned int topCenter =
		static_cast<unsigned int>(vertices.size());

	vertices.push_back({
		{0.0f, HH, 0.0f},
		{0.5f, 0.5f} 
		});

	unsigned int topRing =
		static_cast<unsigned int>(vertices.size());

	for(int i = 0; i <= DIV; ++i)
	{
		float rad = 2.0f * PI * i / DIV;

		float x = cosf(rad) * R;
		float z = sinf(rad) * R;

		float u = 0.5f + cosf(rad) * 0.5f;
		float v = 0.5f + sinf(rad) * 0.5f;
		vertices.push_back({
			{x, HH, z},
			{u, v} 
			});
	}

	// 底面
	unsigned int bottomCenter =
		static_cast<unsigned int>(vertices.size());

	vertices.push_back({
		{0.0f, -HH, 0.0f},
		{0.5f, 0.5f} 
		});

	unsigned int bottomRing =
		static_cast<unsigned int>(vertices.size());

	for(int i = 0; i <= DIV; ++i)
	{
		float rad = 2.0f * PI * i / DIV;

		float x = cosf(rad) * R;
		float z = sinf(rad) * R;

		float u = 0.5f + cosf(rad) * 0.5f;
		float v = 0.5f + sinf(rad) * 0.5f;

		vertices.push_back({
			{x, -HH, z},
			{u, v} 
			});
	}

	//--- インデックスの作成
	for(int i = 0; i < DIV; ++i)
	{
		// 側面の4頂点
		unsigned int bottomLeft = i * 2;
		unsigned int topLeft = bottomLeft + 1;
		unsigned int bottomRight = (i + 1) * 2;
		unsigned int topRight = bottomRight + 1;

		// 側面：三角形2枚
		indices.push_back(bottomLeft);
		indices.push_back(topLeft);
		indices.push_back(bottomRight);

		indices.push_back(topLeft);
		indices.push_back(topRight);
		indices.push_back(bottomRight);

		// 天面
		indices.push_back(topCenter);
		indices.push_back(topRing + i + 1);
		indices.push_back(topRing + i);

		// 底面
		indices.push_back(bottomCenter);
		indices.push_back(bottomRing + i);
		indices.push_back(bottomRing + i + 1);
	}

	//--- バッファの作成
	MeshBuffer::Description desc = {};

	desc.pVtx = vertices.data();
	desc.vtxCount = static_cast<UINT>(vertices.size());
	desc.vtxSize = sizeof(Vertex);
	desc.pIdx = indices.data();
	desc.idxCount = static_cast<UINT>(indices.size());
	desc.idxSize = sizeof(unsigned int);

	desc.topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	m_pCylinder = new MeshBuffer();

	HRESULT hr = m_pCylinder->Create(desc);	

	if (FAILED(hr))
	{
		SAFE_DELETE(m_pCylinder);

		MessageBoxA(
			nullptr,
			"Failed to create Cylinder mesh.",
			"Geometory Error",
			MB_OK | MB_ICONERROR
		);
	}
}

void Geometory::MakeSphere()
{
	const float PI = 3.14159265f;
	const float R = 0.5f; // 半径
	const int LONGITUDE = 36; // 経度の分割数
	const int LATITUDE = 18; // 緯度の分割数

	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;

	//--- 頂点の作成
	for(int lat = 0; lat <= LATITUDE; ++lat)
	{
		float theta = PI * lat / LATITUDE;
		float sinTheta = sinf(theta);
		float cosTheta = cosf(theta);
		for(int lon = 0; lon <= LONGITUDE; ++lon)
		{
			float phi = 2.0f * PI * lon / LONGITUDE;
			float sinPhi = sinf(phi);
			float cosPhi = cosf(phi);
			float x = R * sinTheta * cosPhi;
			float y = R * cosTheta;
			float z = R * sinTheta * sinPhi;
			float u = static_cast<float>(lon) / LONGITUDE;
			float v = static_cast<float>(lat) / LATITUDE;
			vertices.push_back({
				{x, y, z},
				{u, v} 
			});
		}
	}

	//--- インデックスの作成
	for(int lat = 0; lat < LATITUDE; ++lat)
	{
		for(int lon = 0; lon < LONGITUDE; ++lon)
		{
			unsigned int first = lat * (LONGITUDE + 1) + lon;
			unsigned int second = first + LONGITUDE + 1;

			indices.push_back(first);
			indices.push_back(first + 1);
			indices.push_back(second);
			
			indices.push_back(second);
			indices.push_back(first + 1);
			indices.push_back(second + 1);
		}
	}

	// バッファの作成
	MeshBuffer::Description desc = {};

	desc.pVtx = vertices.data();
	desc.vtxCount = static_cast<UINT>(vertices.size());
	desc.vtxSize = sizeof(Vertex);

	desc.pIdx = indices.data();
	desc.idxCount = static_cast<UINT>(indices.size());
	desc.idxSize = sizeof(unsigned int);

	desc.topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	m_pSphere = new MeshBuffer();

	HRESULT hr = m_pSphere->Create(desc);

	if (FAILED(hr))
	{
		SAFE_DELETE(m_pSphere);

		MessageBoxA(
			nullptr,
			"Failed to create Sphere mesh.",
			"Geometory Error",
			MB_OK | MB_ICONERROR
		);
	}
}