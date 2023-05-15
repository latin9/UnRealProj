// Fill out your copyright notice in the Description page of Project Settings.


#include "Play/AI/Sparrow/UR_SparrowDeathTaskNode.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "../../Controller/URAIController.h"
#include "../../Actor/Boss/UR_SparrowSubBoss.h"
#include "Global/URStructs.h"

UUR_SparrowDeathTaskNode::UUR_SparrowDeathTaskNode()
{
	bNotifyTick = true;
}

EBTNodeResult::Type UUR_SparrowDeathTaskNode::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	if (!m_Controller)
		m_Controller = Cast<AURAIController>(OwnerComp.GetAIOwner());

	if (!m_Boss)
		m_Boss = m_Controller->GetPawn<AUR_SparrowSubBoss>();

	// ����ο��� HP�� 0���� ũ�ٸ� �н��Ѵ�.
	if (m_Boss->GetHP() > 0.0)
	{
		return EBTNodeResult::Failed;
	}

	if (m_Boss->GetAnimationInstance()->IsAnimMontage(DefaultAnimation::ExecutionTarget))
	{
		return EBTNodeResult::InProgress;
		m_Boss->SetIsInvincibility(true);
	}
	// ���⿡ �Դٸ� �������̱� ������ �ش� ������ �������ְ� InProgress�� ���������ش�.
	if (!m_Boss->GetAnimationInstance()->IsAnimMontage(DefaultAnimation::DeathStart))
	{
		m_Boss->GetAnimationInstance()->ChangeAnimMontage(DefaultAnimation::DeathLoop);
		m_Boss->SetIsInvincibility(true);
	}

	return EBTNodeResult::InProgress;
}

void UUR_SparrowDeathTaskNode::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	FinishLatentTask(OwnerComp, ExecuteTask(OwnerComp, NodeMemory));
}
