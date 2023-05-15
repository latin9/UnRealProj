// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "Global/UREnum.h"
#include "UR_AttackTaskNode.generated.h"

UCLASS()
class UNREALPROJ_API UUR_AttackTaskNode : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UUR_AttackTaskNode();

private:
	class AURAIController* m_Controller;

	class AMonster* m_Monster;

	float m_WaitTime;

	PirateAttack_Type m_AttackType;

	
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	bool AnimMontageJudge(class AMonster* _Monster);
};
