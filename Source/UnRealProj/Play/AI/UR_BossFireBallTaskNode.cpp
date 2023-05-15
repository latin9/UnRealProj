// Fill out your copyright notice in the Description page of Project Settings.


#include "Play/AI/UR_BossFireBallTaskNode.h"
#include "../Play/Actor/Boss/UR_BossMonster.h"
#include "Play/Controller/URAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackBoardComponent.h"
#include "Global/URStructs.h"
#include "Global/URBlueprintFunctionLibrary.h"

UUR_BossFireBallTaskNode::UUR_BossFireBallTaskNode()
{
	// TickTask�Լ��� ���۽�ų�� �������� ����
	bNotifyTick = true;
}

EBTNodeResult::Type UUR_BossFireBallTaskNode::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	AURAIController* Controller = Cast<AURAIController>(OwnerComp.GetAIOwner());
	AUR_BossMonster* Boss = Controller->GetPawn<AUR_BossMonster>();
	const FURMonsterDataInfo* MonsterInfo = Boss->GetMonsterData();

	UAnimMontage* FindMontage = Boss->GetAnimationInstance()->GetAnimation(BossAnimation::Spawn);
	if (Boss->GetAnimationInstance()->Montage_IsPlaying(FindMontage))
	{
		return EBTNodeResult::Failed;
	}

	UObject* Target = OwnerComp.GetBlackboardComponent()->GetValueAsObject(FName("TargetActor"));

	if (nullptr == Target)
	{
		return EBTNodeResult::Failed;
	}

	AActor* TargetActor = Cast<AActor>(Target);

	// ���� �������� �ۿ� �ִٸ� Idle�� �����Ѵ�.
	if (!Boss->GetIsRangeInTarget(TargetActor, MonsterInfo->SkillRange))
	{
		return EBTNodeResult::Failed;
	}

	// �ش� ��ų�� 0���̿������� ����� �� ����.
	if (Boss->GetRandNumb() == 0)
	{
		return EBTNodeResult::Failed;
	}

	UAnimMontage* Montage = Boss->GetAnimationInstance()->GetCurrentMontage();

	/*if (Montage->GetName() == FString(TEXT("UR_Lich_Attack_Right_Montage")))
	{
		return EBTNodeResult::Failed;
	}*/

	if (!Boss->GetIsRangeInTarget(TargetActor, MonsterInfo->SkillRange))
	{
		return EBTNodeResult::Failed;
	}

	m_WaitTime = Montage->GetPlayLength();

	Boss->GetAnimationInstance()->ChangeAnimMontage(BossAnimation::Skill1);

	return EBTNodeResult::InProgress;
}

void UUR_BossFireBallTaskNode::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	// Waittime���� �� ��Ÿ���� ���̰� ���ְ�
	m_WaitTime -= DeltaSeconds;

	if (0.0f < m_WaitTime)
	{
		return;
	}

	// �׸�ŭ ��ٷȴٴ� �̾߱Ⱑ �˴ϴ�.
	m_WaitTime = 0.0f;

	FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
}
