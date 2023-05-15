// Fill out your copyright notice in the Description page of Project Settings.


#include "Play/AI/UR_BossAttackTaskNode.h"
#include "../Play/Actor/Boss/UR_BossMonster.h"
#include "Play/Controller/URAIController.h"
#include "../Actor/Player/WarriorCharacter.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackBoardComponent.h"
#include "Global/URStructs.h"
#include "Global/URBlueprintFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"

UUR_BossAttackTaskNode::UUR_BossAttackTaskNode()
{
	// TickTask�Լ��� ���۽�ų�� �������� ����
	bNotifyTick = true;
}

EBTNodeResult::Type UUR_BossAttackTaskNode::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	if (!m_Controller)
		m_Controller = Cast<AURAIController>(OwnerComp.GetAIOwner());

	if(!m_Boss)
		m_Boss = m_Controller->GetPawn<AUR_BossMonster>();

	UAnimMontage* FindMontage = m_Boss->GetAnimationInstance()->GetAnimation(BossAnimation::Spawn);
	if (m_Boss->GetAnimationInstance()->Montage_IsPlaying(FindMontage))
	{
		return EBTNodeResult::Failed;
	}

	const FURMonsterDataInfo* MonsterInfo = m_Boss->GetMonsterData();

	UObject* Target = OwnerComp.GetBlackboardComponent()->GetValueAsObject(FName("TargetActor"));

	if (nullptr == Target)
	{
		return EBTNodeResult::Failed;
	}

	AActor* TargetActor = Cast<AActor>(Target);

	// ���� �������� �ۿ� �ִٸ� Idle�� �����Ѵ�.
	if (!m_Boss->GetIsRangeInTarget(TargetActor, MonsterInfo->AttRange))
	{
		return EBTNodeResult::Failed;
	}

	UAnimMontage* Montage = m_Boss->GetAnimationInstance()->GetAnimation(DefaultAnimation::Attack);

	if (!Montage)
	{
		m_Boss->GetAnimationInstance()->ChangeAnimMontage(DefaultAnimation::Idle);
		return EBTNodeResult::Failed;
	}

	m_WaitTime = Montage->GetPlayLength();

	m_Boss->GetAnimationInstance()->ChangeAnimMontage(DefaultAnimation::Attack);

	return EBTNodeResult::Type();
}

void UUR_BossAttackTaskNode::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	if (!m_Controller)
		m_Controller = Cast<AURAIController>(OwnerComp.GetAIOwner());

	if (!m_Boss)
		m_Boss = m_Controller->GetPawn<AUR_BossMonster>();

	AWarriorCharacter* Player = Cast<AWarriorCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

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
