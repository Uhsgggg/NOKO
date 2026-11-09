#pragma once
#include "Enemy.h"
#include "Source/Engine/Graphics/StaticFbxModel.h"

class KingSlime : public Enemy
{
public:
	explicit KingSlime(StaticFbxModel& model);

	void Update(float deltaTime) override;

	void Draw(
		const DirectX::XMFLOAT4X4& view,
		const DirectX::XMFLOAT4X4& projection
	);

private:
	StaticFbxModel* m_pModel = nullptr;
};

