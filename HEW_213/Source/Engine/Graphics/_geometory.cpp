#include "Geometory.h"
#include <DirectXMath.h>

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

void Geometory::MakeCylinder()
{
	const float PI = 3.14159265f;
	const int DIV = 36; // 分割数
	const float R = 0.5f; // 半径
	const float HH = 0.5f; // 高さ

	//--- 頂点の作成
	Vertex vtx[(DIV + 1) * 2] = {};

	for (int i = 0; i < DIV; i++)
	{
		float rad = (PI * 2) * i / DIV;

		float x = cosf(rad) * R;
		float z = sinf(rad) * R;
	}

	// 天面、底面


	// 側面

	//--- インデックスの作成


	// 天面、底面

	// 側面



	//--- バッファの作成
}

void Geometory::MakeSphere()
{
	//--- 頂点の作成

	//--- インデックスの作成

	// バッファの作成
}