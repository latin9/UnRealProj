// Fill out your copyright notice in the Description page of Project Settings.


#include "Play/AI/Khaimera/UR_KhaimeraAttackTaskNode.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "../../Actor/Player/WarriorCharacter.h"
#include "../../Controller/URAIController.h"
#include "../../Actor/Boss/UR_KhaimeraBoss.h"
#include "Global/URStructs.h"

UUR_KhaimeraAttackTaskNode::UUR_KhaimeraAttackTaskNode()	:
	m_Enable(false)
{
	bNotifyTick = true;
}

EBTNodeResult::Type UUR_KhaimeraAttackTaskNode::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	if (!m_Controller)
		m_Controller = Cast<AURAIController>(OwnerComp.GetAIOwner());

	if (!m_Boss)
		m_Boss = m_Controller->GetPawn<AUR_KhaimeraBoss>();

	if (!m_Player)
		m_Player = m_Boss->GetWorld()->GetFirstPlayerController()->GetPawn<AWarriorCharacter>();


	const FURMonsterDataInfo* KhaimeraInfo = m_Boss->GetKhaimeraData();

	// ���� ���° ���������� �Ǵ����ش�.
	int AttackNumb = OwnerComp.GetBlackboardComponent()->GetValueAsInt(FName("RandAttackNumb"));

	// 3�����ݺ��� ���� �����̶�°��� ���� �ڼ����� �Ѿ���Ѵٴ� �ǹ��̴�.
	if (AttackNumb > 4)
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
		m_Boss->AttackOff();
		return EBTNodeResult::Failed;
	}

	AActor* TargetActor = Cast<AActor>(Target);

	if (!m_Enable)
	{
		if (!m_Boss->GetAnimationInstance()->IsAnimMontage(KhaimeraBossAnimation::FowardAttack1) &&
			!m_Boss->GetAnimationInstance()->IsAnimMontage(KhaimeraBossAnimation::FowardAttack2) &&
			!m_Boss->GetAnimationInstance()->IsAnimMontage(KhaimeraBossAnimation::FowardAttack3) &&
			!m_Boss->GetAnimationInstance()->IsAnimMontage(KhaimeraBossAnimation::FowardCombo1))
		{
			// ���� �������� �ۿ� �ִٸ� Idle�� �����Ѵ�.
			// �÷��̾ �˴ٿ� ���ϰԵǸ� ���ݹ������� ����� ������ �ش� �κ��� ����ó��
			if (!m_Player->GetAnimationInstance()->IsAnimMontage(WarriorHitAnimation::CombatHitLargeToFallDown) &&
				!m_Player->GetAnimationInstance()->IsAnimMontage(WarriorHitAnimation::HitLargeToFallDown))
			{
				if (!m_Boss->GetIsRangeInTarget(TargetActor, KhaimeraInfo->AttRange))
				{
					m_Boss->AttackOff();
					return EBTNodeResult::Failed;
				}
			}
		}
	}

	switch (m_AttackType)
	{
	case KhaimeraAttack::Attack1:
		if (!m_Enable)
		{
			m_Enable = true;
			m_Boss->GetAnimationInstance()->ChangeAnimMontage(KhaimeraBossAnimation::FowardAttack1);
			m_Boss->AttackOn();
		}
		break;
	case KhaimeraAttack::Attack2:
		if (!m_Enable)
		{
			m_Enable = true;
			m_Boss->GetAnimationInstance()->ChangeAnimMontage(KhaimeraBossAnimation::FowardAttack2);
			m_Boss->AttackOn();
		}
		break;
	case KhaimeraAttack::Attack3:
		if (!m_Enable)
		{
			m_Enable = true;
			m_Boss->GetAnimationInstance()->ChangeAnimMontage(KhaimeraBossAnimation::FowardAttack3);
			m_Boss->AttackOn();
		}
		break;
	case KhaimeraAttack::Combo1:
		if (!m_Enable)
		{
			m_Enable = true;
			m_Boss->GetAnimationInstance()->ChangeAnimMontage(KhaimeraBossAnimation::FowardCombo1);
			m_Boss->AttackOn();
		}
		break;
	}


	if (m_Boss->GetAnimationInstance()->IsAnimMontage(KhaimeraBossAnimation::FowardAttack1) ||
		m_Boss->GetAnimationInstance()->IsAnimMontage(KhaimeraBossAnimation::FowardAttack2) ||
		m_Boss->GetAnimationInstance()->IsAnimMontage(KhaimeraBossAnimation::FowardAttack3) ||
		m_Boss->GetAnimationInstance()->IsAnimMontage(KhaimeraBossAnimation::Attack1Recovery) ||
		m_Boss->GetAnimationInstance()->IsAnimMontage(KhaimeraBossAnimation::Attack2Recovery) ||
		m_Boss->GetAnimationInstance()->IsAnimMontage(KhaimeraBossAnimation::Attack3Recovery))
	{
		return EBTNodeResult::InProgress;
	}

	m_Enable = false;
	m_Boss->AttackOff();

	return EBTNodeResult::Succeeded;
}

void UUR_KhaimeraAttackTaskNode::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	FinishLatentTask(OwnerComp, ExecuteTask(OwnerComp, NodeMemory));
}

bool UUR_KhaimeraAttackTaskNode::AnimMontageJudge()
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
