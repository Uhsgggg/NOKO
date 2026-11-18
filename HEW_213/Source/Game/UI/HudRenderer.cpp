//========================================//
// 
// HudRenderer.cpp
// 
// HudPresenterが返す表示データを
// Spriteで描く処理
// 
//========================================//
#include "HudRenderer.h"
#include "Engine/Graphics/Sprite.h"
#include <algorithm>

using namespace DirectX;

void HudRenderer::DrawImage(
	Texture* pTexture, float x, float y, float width, float height)
{
	if (!pTexture) return;
	if (!pTexture->GetResource()) return;
	if (width <= 0.0f || height <= 0.0f) return;

	// 使用する画像
	Sprite::SetTexture(pTexture);

	// 描画位置
	Sprite::SetOffset({
		x + width / 2.0f,
		y + height / 2.0f
		});

	// 画像サイズ
	Sprite::SetSize({ width, height });

	// 画像全体を表示する
	Sprite::SetUVPos({ 0.0f, 0.0f });
	Sprite::SetUVScale({ 1.0f, 1.0f });

	Sprite::Draw();

	// 既存のRect・Text用の画像に戻す
	Sprite::SetTexture(&m_white);
}