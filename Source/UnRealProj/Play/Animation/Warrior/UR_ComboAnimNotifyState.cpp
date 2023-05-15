// Fill out your copyright notice in the Description page of Project Settings.


#include "Play/Animation/Warrior/UR_ComboAnimNotifyState.h"
#include "Kismet/GameplayStatics.h"

UUR_ComboAnimNotifyState::UUR_ComboAnimNotifyState()	:
	m_ComboChangeAnimation(WarriorAnimation::Default),
	m_Player(nullptr),
	m_IsComboAttack(false)
{
}

void UUR_ComboAnimNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	m_Player = MeshComp->GetOwner<AWarriorCharacter>();

	if (!m_Player || !m_Player->IsValidLowLevel())
	{
		return;
	}

	if (!m_Player->GetIsQSkill())
	{
		m_Player->AttackOn();
	}

	m_IsComboAttack = false;
	m_Player->SetIsCombating(true);
	m_Player->SetIsComboAttack(false);
	// ���� ������ ����� ���Ž����־����
	m_Player->SetMotionWarpingActor(m_Player->GetTargetActor());
}

void UUR_ComboAnimNotifyState::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	if (m_Player)
	{
		if (!m_Player->GetIsQSkill())
			m_Player->TurnFunc();
		// NotifyTick�� ���ư��µ��� ��Ŭ���� �� ���̶� �� ���ȴ��� �Ǵ��� �ϱ� ���ؼ��̴�
		if (m_Player->GetIsComboAttack())
		{
			m_IsComboAttack = true;
		}
	}
}

void UUR_ComboAnimNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (m_Player)
	{
		// �߰��� �޺������� �Ϸ��� ������ �����ٴ� �ǹ��̴�.
		// �׷��� NotifyEnd�ϋ� ���� �޺� ������ �����ָ� ��.
		if (m_IsComboAttack && m_ComboChangeAnimation != WarriorAnimation::Default)
		{
			m_Player->GetAnimationInstance()->ChangeAnimMontage(m_ComboChangeAnimation);
		}
	}
}
