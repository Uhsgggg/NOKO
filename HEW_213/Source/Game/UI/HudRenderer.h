//========================================//
// 
// HudRenderer.h
// 
// HudPresenter‚ª•Ô‚·•\¦ƒf[ƒ^‚ğ
// Sprite‚Å•`‚­ˆ—
// 
//========================================//
#pragma once

#include "Game/Systems/GameTypes.h"
#include "Engine/Graphics/DirectX/Texture.h"

class HudRenderer
{
public:
	// ‰æ‘œ‚ğ“Ç‚İ‚Ş
	bool Initialize();

	// HUD‘S‘Ì‚ğ•`‰æ‚·‚é
	void Draw(const HudView& view, bool hasBoss, int result);

private:
	// ‰æ‘œ‚Ìˆê•”•ª‚ğ•`‰æ‚·‚é
	void DrawRegion(Texture* pTexture,
		float x, float y, float w, float h,
		float srcX, float srcY, float srcW, float srcH);

	// ‰æ‘œ‘S‘Ì‚ğ•`‰æ‚·‚é
	void DrawImage(Texture* pTexture,
		float x, float y, float w, float h);

	// ”š‰æ‘œ‚ğ‘g‚İ‡‚í‚¹‚Ä•`‰æ‚·‚é
	void DrawNumber(const std::string& text,
		float x, float y, float scale);

private:
	Texture m_atlas;
	bool m_ready = false;
};