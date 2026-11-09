#pragma once
#include <DirectXMath.h>

class Enemy
{
public:
	virtual ~Enemy() = default;

	virtual void Update(float deltaTime) = 0;

	virtual void Draw(
		const DirectX::XMFLOAT4X4& view,
		const DirectX::XMFLOAT4X4& projection
	) = 0;

	void SetPosition(float x, float y, float z)
	{
		m_position = { x,y,z };
	}

	void SetScale(float scale)
	{
		m_scale = scale;
	}

	DirectX::XMFLOAT3 GetPosition() const
	{
		return m_position;
	}

	float GetScale() const
	{
		return m_scale;
	}

protected:
	DirectX::XMFLOAT3 m_position = { 0,0,0 };

	float m_rotationY = 0.0f;
	float m_scale = 1.0;

	int m_hp = 100;
};