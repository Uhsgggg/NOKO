#include "StopAllSystem.h"

#include "Engine/Scene/GameObject.h"
#include "Game/Components/MoveComponent.h"

void StopAllSystem::Stop(
	const std::vector<std::unique_ptr<GameObject>>& objects)
{
	// 全オブジェクトを調べる
	for (const auto& pObject : objects)
	{
		// なかったら無視
		if (!pObject) continue;

		// MoveComponentを取得
		MoveComponent* pMove =
			pObject->GetComponent<MoveComponent>();

		// MoveComponentがなかったら無視
		if (!pMove) continue;

		// 無効化されている場合無視
		if (!pMove->IsEnabled()) continue;

		// 移動速度を0にする
		pMove->SetVelocity(
			0.0f,
			0.0f,
			0.0f
		);
	}
}
