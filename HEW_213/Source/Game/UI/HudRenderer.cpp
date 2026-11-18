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

using namespace DirectX;
using namespace std;

// 画像を一度だけ読み込む
bool HudRenderer::Initialize()
{
	if (m_ready) return true;

	// 既に読み込んだ画像を再作成しない
	if (!m_atlas.GetResource())
	{
		if (FAILED(m_atlas.Create("Assets/UI/hud_atlas.png")))
			return false;
	}

	// hud_atlasのサイズ
	if (m_atlas.GetWidth() != 1024 || m_atlas.GetHeight() != 512)
		return false;

	m_ready = true;
	return true;
}

// HUD全体を描画する
void HudRenderer::Draw(
	const HudView& view, 
	bool hasBoss, int result)
{
	if (!m_ready || !GetRenderWidth() || !GetRenderHeight()) return;

	float w = static_cast<float>(GetRenderWidth());
	float h = static_cast<float>(GetRenderHeight());

	// 基準解像度1280x720。縦横比を崩さず拡大縮小する
	// s … 画面サイズに応じた倍率
	float s = w / 1280.0f;
	if (h / 720.0f < 2)
	{
		s = h / 720.0f;
	}

	// DirectX描画準備
	ID3D11DepthStencilState* pDepth = nullptr;
	ID3D11RasterizerState* pRaster = nullptr;
	ID3D11BlendState* pBlend = nullptr;
	UINT stencil = 0;
	UINT sampleMask = 0;
	float blendFactor[4] = {};
	GetContext()->OMGetDepthStencilState(&pDepth, &stencil);
	GetContext()->RSGetState(&pRaster);
	GetContext()->OMGetBlendState(&pBlend, blendFactor, &sampleMask);
	SetDepthTest(false);
	SetCullingMode(D3D11_CULL_NONE);
	SetBlendMode(BLEND_ALPHA);

	XMFLOAT4X4 identity, projection;
	XMStoreFloat4x4(&identity, XMMatrixIdentity());
	XMStoreFloat4x4(&projection,
		XMMatrixTranspose(XMMatrixOrthographicOffCenterLH(0, w, h, 0, 0, 1)));
	Sprite::SetWorld(identity);
	Sprite::SetView(identity);
	Sprite::SetProjection(projection);
	Sprite::SetVertexShader(nullptr);
	Sprite::SetPixelShader(nullptr);

	// ここからUIの配置
	// ボスHP：真上
	if (hasBoss)
	{
		float x = (w - 448 * s) / 2.0f;
		float y = 24 * s;
		DrawRegion(&m_atlas, x, y, 448 * s, 96 * s,
			0, 224, 448, 96);

		float ratio = view.bossRatio;
		if (!(ratio > 0)) ratio = 0;
		if (ratio > 1) ratio = 1;

		// 幅とUBの両方を減らすと、模様潰さず切り取れる
		DrawRegion(&m_atlas, x + 24 * s, y + 54 * s,
			400 * s * ratio, 24 * s,
			0, 336, 400 * ratio, 24);
	}

	// 次の敵数：右上
	float right = w - 248 * s;
	DrawRegion(&m_atlas, right, 24 * s, 224 * s, 112 * s,
		256, 96, 224, 112);
	DrawNumber(view.nextEnemiesText, right + 20 * s, 76 * s, s);

	// 残り手数：右下
	float bottom = h - 136 * s;
	DrawRegion(&m_atlas, right, bottom, 224 * s, 112 * s,
		0, 96, 224, 112);
	DrawNumber(view.shotsText, right + 20 * s, bottom + 52 * s, s);

	// リトライ：左下
	DrawRegion(&m_atlas, 24 * s, h - 72 * s, 192 * s, 48 * s,
		0, 384, 192, 48);

	// 勝敗：中央（0はプレイ中、1はクリア）
	if (result != 0)
	{
		float srcY = 224;
		if (result == 1)
			srcY = 96;
		DrawRegion(&m_atlas, (w - 384 * s) / 2, (h - 112 * s) / 2,
			384 * s, 112 * s, 512, srcY, 384, 112);
	}

	// DirectX描画設定に戻す
	Sprite::SetTexture(nullptr);
	Sprite::SetUVPos(XMFLOAT2(0, 0));
	Sprite::SetUVScale(XMFLOAT2(1, 1));
	Sprite::SetColor(XMFLOAT4(1, 1, 1, 1));
	GetContext()->OMSetDepthStencilState(pDepth, stencil);
	GetContext()->RSSetState(pRaster);
	GetContext()->OMSetBlendState(pBlend, blendFactor, sampleMask);
	if (pDepth) pDepth->Release();
	if (pRaster) pRaster->Release();
	if (pBlend) pBlend->Release();
}

// 画像の一部分を描画する
void HudRenderer::DrawRegion(Texture* pTexture, 
	float x, float y, float w, float h, 
	float srcX, float srcY, float srcW, float srcH)
{
	if (!pTexture || !pTexture->GetResource()) return;
	if (w <= 0 || h <= 0 || srcW <= 0 || srcH <= 0) return;

	float texW = static_cast<float>(pTexture->GetWidth());
	float texH = static_cast<float>(pTexture->GetHeight());

	if (texW <= 0 || texH <= 0) return;
	if (srcX < 0 || srcY < 0 ||
		srcX + srcW > texW || srcY + srcH > texH) return;

	Sprite::SetTexture(pTexture);

	Sprite::SetOffset(XMFLOAT2(x + w / 2, y + h / 2));

	// 左上原点の画面座標に合わせ、上下反転を防ぐ
	Sprite::SetSize(XMFLOAT2(w, -h));

	// ピクセル座標を0～1のUV座標に変換
	Sprite::SetUVPos(XMFLOAT2(srcX / texW, srcY / texH));
	Sprite::SetUVScale(XMFLOAT2(srcW / texW, srcH / texH));
	Sprite::SetColor(XMFLOAT4(1, 1, 1, 1));
	Sprite::Draw();
}

// PNGやJPGを丸ごと描画する
void HudRenderer::DrawImage(Texture* pTexture, 
	float x, float y, float w, float h)
{
	if (!pTexture) return;
	
	DrawRegion(pTexture, x, y, w, h, 0, 0,
		static_cast<float>(pTexture->GetWidth()),
		static_cast<float>(pTexture->GetHeight()));
}

// 数字を1文字ずつ切り出して描画する
void HudRenderer::DrawNumber(const string& text,
	float x, float y, float scale)
{
	float digitW = 32 * scale;
	float digitH = 40 * scale;

	// 桁数が多くてもパネルの中に収める
	float availableW = 184 * scale;
	if (!text.empty() && digitW * text.size() > availableW)
	{
		digitW = availableW / static_cast<float>(text.size());
		digitH = digitW * 80 / 64;
	}

	for (char ch : text)
	{
		if (ch < '0' || ch > '9') continue;

		int number = ch - '0';
		float srcX = 64.0f * number;
		DrawRegion(&m_atlas, x, y, digitW, digitH,
			srcX, 0, 64, 80);
		x += digitW;
	}
}