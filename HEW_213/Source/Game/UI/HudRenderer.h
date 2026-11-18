//========================================//
// 
// HudRenderer.h
// 
// HudPresenterが返す表示データを
// Spriteで描く処理
// 
//========================================//
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
	bool m_ready = false;
	void Rect(
		float x, float y,
		float w, float h,
		HudColor color);
	void Text(
		float x, float y,
		float pixel,
		const std::string& text,
		HudColor color);
	void DrawImage(
		Texture* pTexture,
		float x, float y,
		float width, float height);
};