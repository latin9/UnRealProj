// Fill out your copyright notice in the Description page of Project Settings.


#include "Play/AI/Khaimera/UR_KhaimeraBerserkTaskNode.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "../../Controller/URAIController.h"
#include "../../Actor/Boss/UR_KhaimeraBoss.h"
#include "Global/URStructs.h"

UUR_KhaimeraBerserkTaskNode::UUR_KhaimeraBerserkTaskNode()
{
	// TickTask함수를 동작시킬지 결정해주 변수
	bNotifyTick = true;
}

EBTNodeResult::Type UUR_KhaimeraBerserkTaskNode::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	if (!m_Controller)
		m_Controller = Cast<AURAIController>(OwnerComp.GetAIOwner());

	if (!m_Boss)
		m_Boss = m_Controller->GetPawn<AUR_KhaimeraBoss>();

	const FURMonsterDataInfo* MonsterInfo = m_Boss->GetKhaimeraData();

	if (m_Boss->GetHPPercent() > 0.5f)
	{
		return EBTNodeResult::Failed;
	}

	if (m_Boss->GetIsBerserk() && !m_Boss->GetAnimationInstance()->IsAnimMontage(KhaimeraBossAnimation::BerserkStart))
	{
		m_Boss->SetIsInvincibility(false);
		return EBTNodeResult::Failed;
	}

	if (!m_Boss->GetIsBerserk())
	{
		m_Boss->SetIsBerserk(true);
		m_Boss->SetIsInvincibility(true);
		m_Boss->SetBerserkMesh();
		m_Boss->GetAnimationInstance()->ChangeAnimMontage(KhaimeraBossAnimation::BerserkStart);
	}
	

    return EBTNodeResult::InProgress;
}

void UUR_KhaimeraBerserkTaskNode::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	FinishLatentTask(OwnerComp, ExecuteTask(OwnerComp, NodeMemory));
}


bool UUR_KhaimeraBerserkTaskNode::AnimMontageJudge()
{
	if (m_Boss->GetAnimationInstance()->IsAnimMontage(KhaimeraBossAnimation::FowardAttack1) ||
		m_Boss->GetAnimationInstance()->IsAnimMontage(KhaimeraBossAnimation::FowardAttack2) ||
		m_Boss->GetAnimationInstance()->IsAnimMontage(KhaimeraBossAnimation::FowardAttack3) ||
		m_Boss->GetAnimationInstance()->IsAnimMontage(KhaimeraBossAnimation::Attack1Recovery) ||
		m_Boss->GetAnimationInstance()->IsAnimMontage(KhaimeraBossAnimation::Attack2Recovery) ||
		m_Boss->GetAnimationInstance()->IsAnimMontage(KhaimeraBossAnimation::Attack3Recovery) ||
		m_Boss->GetAnimationInstance()->IsAnimMontage(KhaimeraBossAnimation::FowardFastComboAttack) ||
		m_Boss->GetAnimationInstance()->IsAnimMontage(KhaimeraBossAnimation::DashStart) ||
		m_Boss->GetAnimationInstance()->IsAnimMontage(KhaimeraBossAnimation::DashAttack) ||
		m_Boss->GetAnimationInstance()->IsAnimMontage(KhaimeraBossAnimation::BerserkStart) ||
		m_Boss->GetAnimationInstance()->IsAnimMontage(DefaultAnimation::ForwardHit) ||
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
