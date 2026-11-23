#pragma once

#include "Engine/Scene/Component.h"
#include "Game/Systems/GameTypes.h"

class ReflectionCounterComponent : public Component {
public:
    explicit ReflectionCounterComponent(GameObject *pOwner);

    const char *GetTypeId() const override
    { 
        return "ReflectionCounter";
    }

    const char *GetTypeName() const override
    { 
        return "反射回数カウンター";
    }

    std::unique_ptr<Component> Clone(
        GameObject *pNewOwner) const override;

#ifdef _DEBUG
    void DrawDebugInspector() override;
#endif

    void Reset();
    void Increment();
    int GetCount() const;

    // 反射を1回記録して、攻撃強化値も加算
    void RecordReflection(int attackLevelGain);
    // 現在の攻撃強化値を取得
    int GetAttackLevel() const;

private:
    // 実際に何回反射したか
    int m_reflectionCount = 0;
    // 攻撃力を加算するための強化値
    int m_attackLevel = 0;
};
