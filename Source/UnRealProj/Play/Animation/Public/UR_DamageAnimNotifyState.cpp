// Fill out your copyright notice in the Description page of Project Settings.


#include "UR_DamageAnimNotifyState.h"
#include "../../Actor/URCharacter.h"
#include "../../Actor/Player/WarriorCharacter.h"

UUR_DamageAnimNotifyState::UUR_DamageAnimNotifyState()	:
	m_IsKnockBack(true)
{
}

void UUR_DamageAnimNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	AURCharacter* Actor = MeshComp->GetOwner<AURCharacter>();

	if (!Actor || !Actor->IsValidLowLevel())
	{
		return;
	}

	Actor->SetKnockDown(m_IsKnockDown);
	Actor->DamageOn(m_IsKnockBack);

	if (!Actor->GetAnimationInstance()->IsAnimMontage(WarriorAnimation::SkillELoop))
		Actor->CameraShake(CameraShake_Type::HitShake);

	m_OneFrame = false;
}

void UUR_DamageAnimNotifyState::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	AURCharacter* Actor = MeshComp->GetOwner<AURCharacter>();

	if (!Actor || !Actor->IsValidLowLevel())
	{
		return;
	}

	// begin�̶� tick�� 1�������� �����ؾ��ϴµ� �����Ƿ�
	// 1�������� �׳� �������� �ڵ尡 ����ǵ��� ��Ų��.
	if (!m_OneFrame)
	{
		m_OneFrame = true;
		return;
	}

	Actor->DamageOff();
}

void UUR_DamageAnimNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
}
