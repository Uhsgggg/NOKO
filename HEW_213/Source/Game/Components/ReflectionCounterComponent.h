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
        return "ReflectionCounter";
    }

    std::unique_ptr<Component> Clone(
        GameObject *pNewOwner) const override;

#ifdef _DEBUG
    void DrawDebugInspector() override;
#endif

    void Reset();
    void Increment();
    int GetCount() const;

private:
    int m_count = 0;
};
