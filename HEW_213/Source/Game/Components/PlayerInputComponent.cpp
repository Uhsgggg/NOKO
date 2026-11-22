#include "PlayerInputComponent.h"
#include "Engine/Scene/GameObject.h"

#ifdef _DEBUG
#include "imgui.h"
#endif

using namespace std;

PlayerInputComponent::PlayerInputComponent(GameObject* pOwner) : Component(pOwner)
{
}

//マウス情報を受け取り、ドラッグ状態だけを記録する
void PlayerInputComponent::Sample(const PointerFrame& pointerFrame, bool canAim)
{
	m_aim.released = false;
	if (!pointerFrame.valid || !canAim)
	{
		Cancel();
		return;
	}

	if (pointerFrame.pressed)
	{
		Cancel();
		//ゲーム画面のどこからでもドラッグ開始
		m_aim.dragging = true;
		m_aim.anchor   = pointerFrame.point;
		m_aim.point    = pointerFrame.point;
	}

	if (!m_aim.dragging) return;

	m_aim.point = pointerFrame.point;
	if (pointerFrame.released)
	{
		m_aim.dragging = false;
		m_aim.released = true;
	}
	else if (!pointerFrame.held)
	{
		Cancel();
	}
}

AimState PlayerInputComponent::GetAim() const
{
	return m_aim;
}

void PlayerInputComponent::Cancel()
{
	m_aim = {};
}

std::unique_ptr<Component> PlayerInputComponent::Clone(GameObject* pNewOwner) const
{
	auto pCopy = make_unique<PlayerInputComponent>(pNewOwner);
	pCopy->SetEnabled(IsEnabled());
	return pCopy;
}

#ifdef _DEBUG
void PlayerInputComponent::DrawDebugInspector()
{
	ImGui::TextUnformatted("設定する項目はありません");
}
#endif