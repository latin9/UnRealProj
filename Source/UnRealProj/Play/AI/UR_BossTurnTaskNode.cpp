// Fill out your copyright notice in the Description page of Project Settings.


#include "Play/AI/UR_BossTurnTaskNode.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "../Actor/Boss/UR_BossMonster.h"
#include "Play/Controller/URAIController.h"
#include "Global/URStructs.h"
#include "Global/URBlueprintFunctionLibrary.h"
#include "Kismet/KismetMathLibrary.h"

UUR_BossTurnTaskNode::UUR_BossTurnTaskNode()
{
	bNotifyTick = true;
}

EBTNodeResult::Type UUR_BossTurnTaskNode::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	if (!m_Controller)
		m_Controller = Cast<AURAIController>(OwnerComp.GetAIOwner());

	if (!m_LichBoss)
		m_LichBoss = m_Controller->GetPawn<AUR_BossMonster>();

	if (m_LichBoss)
	{
		UAnimMontage* FindMontage = m_LichBoss->GetAnimationInstance()->GetAnimation(BossAnimation::Spawn);
		if (m_LichBoss->GetAnimationInstance()->Montage_IsPlaying(FindMontage))
		{
			return EBTNodeResult::Failed;
		}

		const FURMonsterDataInfo* MonsterInfo = m_LichBoss->GetMonsterData();

		UObject* Target = OwnerComp.GetBlackboardComponent()->GetValueAsObject("TargetActor");

		if (!Target)
		{
			return EBTNodeResult::Succeeded;
		}

		const BossAnimation Anim = BossAnimation::Skill1;

		if (!m_LichBoss->FindBossAnimMontage(Anim))
		{
			return EBTNodeResult::Succeeded;
		}

		AActor* TargetActor = Cast<AActor>(Target);

		m_LichBoss->SetTargetLook(TargetActor);
		m_LichBoss->SetTargetMovementInput(TargetActor);

		auto forwardVect = m_LichBoss->GetActorForwardVector();    // forward บคลอ
		FVector A = (TargetActor->GetActorLocation() - m_LichBoss->GetActorLocation());                   // A บคลอ
		A.Normalize();

		float dot = FVector::DotProduct(forwardVect, A);
		float angle = FMath::RadiansToDegrees(FMath::Acos(dot));

		if (angle > -5 || angle < 5)
		{
			return EBTNodeResult::Succeeded;
		}
	}

	return EBTNodeResult::Type::InProgress;
}

void UUR_BossTurnTaskNode::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	FinishLatentTask(OwnerComp, ExecuteTask(OwnerComp, NodeMemory));
}
