// Fill out your copyright notice in the Description page of Project Settings.


#include "Play/AI/UR_DeathTaskNode.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Play/Actor/Monster/Monster.h"
#include "../Controller/URAIController.h"

UUR_DeathTaskNode::UUR_DeathTaskNode()
{
	// TickTask�Լ��� ���۽�ų�� �������� ����
	bNotifyTick = true;
}

EBTNodeResult::Type UUR_DeathTaskNode::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	AURAIController* Controller = Cast<AURAIController>(OwnerComp.GetAIOwner());

	AMonster* Monster = Controller->GetPawn<AMonster>();

	if (Monster->IsDeath())
	{
		/*if (!Monster->GetAnimationInstance()->IsAnimMontage(DefaultAnimation::DeathStart) &&
			!Monster->GetAnimationInstance()->IsAnimMontage(DefaultAnimation::DeathLoop) &&
			!Monster->GetAnimationInstance()->IsAnimMontage(DefaultAnimation::HitAirToFloorEnd))
		{
			Monster->GetAnimationInstance()->ChangeAnimMontage(DefaultAnimation::DeathStart);
		}
		
		if (Monster->GetAnimationInstance()->IsAnimMontage(DefaultAnimation::DeathLoop) ||
			Monster->GetAnimationInstance()->IsAnimMontage(DefaultAnimation::HitAirToFloorEnd))
		{
			Monster->GetAnimationInstance()->ChangeAnimMontage(DefaultAnimation::DeathLoop);
		}*/

		if (!Monster->GetAnimationInstance()->IsAnimMontage(DefaultAnimation::DeathStart) &&
			!Monster->GetAnimationInstance()->IsAnimMontage(DefaultAnimation::DeathLoop))
			Monster->GetAnimationInstance()->ChangeAnimMontage(DefaultAnimation::DeathStart);

		else if (Monster->GetAnimationInstance()->IsAnimMontage(DefaultAnimation::DeathLoop))
			Monster->GetAnimationInstance()->ChangeAnimMontage(DefaultAnimation::DeathLoop);

		Monster->SetIsInvincibility(true);
		return EBTNodeResult::InProgress;
	}

	return EBTNodeResult::Succeeded;
}

void UUR_DeathTaskNode::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	FinishLatentTask(OwnerComp, ExecuteTask(OwnerComp, NodeMemory));
}
