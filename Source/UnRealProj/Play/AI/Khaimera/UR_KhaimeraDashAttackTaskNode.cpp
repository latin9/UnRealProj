// Fill out your copyright notice in the Description page of Project Settings.


#include "Play/AI/Khaimera/UR_KhaimeraDashAttackTaskNode.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "../../Controller/URAIController.h"
#include "../../Actor/Boss/UR_KhaimeraBoss.h"
#include "Global/URStructs.h"

UUR_KhaimeraDashAttackTaskNode::UUR_KhaimeraDashAttackTaskNode()	:
	m_Enable(false)
{
	bNotifyTick = true;
}

EBTNodeResult::Type UUR_KhaimeraDashAttackTaskNode::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	if (!m_Controller)
		m_Controller = Cast<AURAIController>(OwnerComp.GetAIOwner());

	if (!m_Boss)
		m_Boss = m_Controller->GetPawn<AUR_KhaimeraBoss>();

	const FURMonsterDataInfo* KhaimeraInfo = m_Boss->GetKhaimeraData();
	
	// ���� ���° ���������� �Ǵ����ش�.
	int AttackNumb = OwnerComp.GetBlackboardComponent()->GetValueAsInt(FName("RandAttackNumb"));

	// 5�� �ƴ϶�� ���� ������ �ƴϴ�.
	if (AttackNumb != 6)
	{
		return EBTNodeResult::Succeeded;
	}

	if (AnimMontageJudge())
	{
		return EBTNodeResult::Succeeded;
	}
	m_AttackType = static_cast<KhaimeraAttack>(AttackNumb);

	UObject* Target = OwnerComp.GetBlackboardComponent()->GetValueAsObject(FName("TargetActor"));

	if (nullptr == Target)
	{
		return EBTNodeResult::Failed;
	}

	AActor* TargetActor = Cast<AActor>(Target);

	if (!m_Enable)
	{
		// ���� �������� �ۿ� �ִٸ� Idle�� �����Ѵ�.
		if (!m_Boss->GetIsRangeInTarget(TargetActor, KhaimeraInfo->SkillRange))
		{
			return EBTNodeResult::Failed;
		}
	}

	switch (m_AttackType)
	{
	case KhaimeraAttack::DashAttack:
		if (!m_Enable)
		{
			m_Enable = true;
			m_Boss->GetAnimationInstance()->ChangeAnimMontage(KhaimeraBossAnimation::DashStart);
		}
		break;
	}


	if (m_Boss->GetAnimationInstance()->IsAnimMontage(KhaimeraBossAnimation::DashStart) ||
		m_Boss->GetAnimationInstance()->IsAnimMontage(KhaimeraBossAnimation::DashAttack))
	{
		return EBTNodeResult::InProgress;
	}

	m_Enable = false;

	return EBTNodeResult::Failed;
}

void UUR_KhaimeraDashAttackTaskNode::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	FinishLatentTask(OwnerComp, ExecuteTask(OwnerComp, NodeMemory));
}

bool UUR_KhaimeraDashAttackTaskNode::AnimMontageJudge()
{
	if (m_Boss->GetAnimationInstance()->IsAnimMontage(DefaultAnimation::ForwardHit) ||
		m_Boss->GetAnimationInstance()->IsAnimMontage(DefaultAnimation::BackwardHit) ||
		m_Boss->GetAnimationInstance()->IsAnimMontage(DefaultAnimation::LeftHit) ||
		m_Boss->GetAnimationInstance()->IsAnimMontage(DefaultAnimation::RightHit) ||
		m_Boss->GetAnimationInstance()->IsAnimMontage(DefaultAnimation::HitAirLoop) ||
		m_Boss->GetAnimationInstance()->IsAnimMontage(DefaultAnimation::KnockDown) ||
		m_Boss->GetAnimationInstance()->IsAnimMontage(DefaultAnimation::GetUp))
	{
		return true;
	}

	return false;
}
