#pragma once

#include "Game/Systems/GameTypes.h"
#include "Engine/Graphics/DirectX/Texture.h"

class HudRenderer
{
public:
	bool Initialize();
	void Draw(const HudView& view, bool hasBoss, int result);

private:
	Texture m_white;
	void Rect(
		float x, float y,
		float w, float h,
		HudColor color);
	void Text(
		float x, float y,
		float pixel,
		const std::string& text,
		HudColor color);
};