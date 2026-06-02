#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "BehaviorTree/BTTaskNode.h"
#include "GameplayTagContainer.h"
#include "SoulsBTTask_Attack.generated.h"

struct FSoulsBTTask_AttackMemory
{
    /** GetWorld()->TimeSeconds at ExecuteTask. Used for timeout. */
    float TaskStartTime = 0.0f;

    bool bActionStarted = false;
};

UCLASS()
class SOULLIKE_API USoulsBTTask_Attack : public UBTTaskNode
{
    GENERATED_BODY()

public:
    USoulsBTTask_Attack();

protected:
    //~ UBTTaskNode
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
    virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

    /** Per-instance memory bytes. Engine uses this to size the NodeMemory slot. */
    virtual uint16 GetInstanceMemorySize() const override;

    /** Placement-new our struct into the engine-allocated slot. */
    virtual void InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryInit::Type InitType) const override;

    /** Destruct the struct. (Trivial in our case but required for symmetry.) */
    virtual void CleanupMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryClear::Type CleanupType) const override;

    /** Description shown in BT editor under NodeName. */
    virtual FString GetStaticDescription() const override;
    //~ End UBTTaskNode

    /** Action GameplayTag to start. Defaults to Action.LightAttack. */
    UPROPERTY(EditAnywhere, Category = "Souls|AI", meta = (Categories = "Action"))
    FGameplayTag ActionToStart;

    UPROPERTY(EditAnywhere, Category = "Souls|AI", meta = (Categories = "Status"))
    FGameplayTag MonitorTag;

    UPROPERTY(EditAnywhere, Category = "Souls|AI", meta = (ClampMin = "0.5"))
    float TimeoutSeconds = 5.0f;

    UPROPERTY(EditAnywhere, Category = "Souls|AI|Blackboard")
    bool bUseRequiredBoolKey = false;

    /** Bool key that must be true when bUseRequiredBoolKey is enabled. */
    UPROPERTY(EditAnywhere, Category = "Souls|AI|Blackboard", meta = (EditCondition = "bUseRequiredBoolKey"))
    FBlackboardKeySelector RequiredBoolKey;

private:
    /** Helper: cast NodeMemory back to typed struct ptr. */
    static FSoulsBTTask_AttackMemory* GetMemory(uint8* NodeMemory)
    {
        return reinterpret_cast<FSoulsBTTask_AttackMemory*>(NodeMemory);
    }

    /* Helper: cast NodeMemory back to typed struct const ptr. */
    static const FSoulsBTTask_AttackMemory* GetMemory(const uint8* NodeMemory)
    {
        return reinterpret_cast<const FSoulsBTTask_AttackMemory*>(NodeMemory);
    }
};
