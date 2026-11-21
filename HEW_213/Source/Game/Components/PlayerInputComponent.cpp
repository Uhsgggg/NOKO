#include "PlayerInputComponent.h"
#include "Engine/Scene/GameObject.h"

#ifdef _DEBUG
#include "imgui.h"
#endif

PlayerInputComponent::PlayerInputComponent(GameObject* pOwner) : Component(pOwner)
{
}

//マウス情報を受け取り、ドラッグ状態だけを記録する
void PlayerInputComponent::Sample(const PointerFrame& frame, bool canAim)
{
	m_aim.released = false;
	if (!frame.valid || !canAim)
	{
		Cancel();
		return;
	}

	if (frame.pressed)
	{
		Cancel();
		//ゲーム画面のどこからでもドラッグ開始
		m_aim.dragging = true;
		m_aim.anchor   = frame.point;
		m_aim.point = frame.point;
	}

	if (!m_aim.dragging) return;

	m_aim.point = frame.point;
	if (frame.released)
	{
		m_aim.dragging = false;
		m_aim.released = true;
	}
	else if (!frame.held)
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
	auto pCopy = std::make_unique<PlayerInputComponent>(pNewOwner);
	pCopy->SetEnabled(IsEnabled());
	return pCopy;
}

#ifdef _DEBUG
void PlayerInputComponent::DrawDebugInspector()
{
	ImGui::TextUnformatted("NO saved settings");
}
#endif