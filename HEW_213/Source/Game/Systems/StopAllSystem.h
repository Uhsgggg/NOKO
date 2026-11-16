#pragma once

#include <memory>
#include <vector>

class GameObject;

// Componentとは違い、(Systemという基底クラスがあるとして)
// [ class StopAllSystem : public System ]
// みたいに継承しない
class StopAllSystem
{
public:
	// すべてのオブジェクトを調べ、
	// MoveComponentを持つものを停止する
	void Stop(const std::vector<std::unique_ptr<GameObject>>& objects);

	// 必要であれば、Clone()、GetTypeId()、DrawDebugInspector()も追加する
};