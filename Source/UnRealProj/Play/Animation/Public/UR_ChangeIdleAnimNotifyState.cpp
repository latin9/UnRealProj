// Fill out your copyright notice in the Description page of Project Settings.


#include "Play/Animation/Public/UR_ChangeIdleAnimNotifyState.h"
#include "../../Actor/Player/WarriorCharacter.h"

void UUR_ChangeIdleAnimNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	switch (m_MonsterType)
	{
	case UMONSTER_TYPE::Pirate:
	{
		AURCharacter* Character = MeshComp->GetOwner<AURCharacter>();

		if (!Character || !Character->IsValidLowLevel())
		{
			return;
		}

		Character->GetAnimationInstance()->ChangeAnimMontage(DefaultAnimation::Idle);
	}
		break;
	case UMONSTER_TYPE::Khaimera:
	{
		AURCharacter* Character = MeshComp->GetOwner<AURCharacter>();

		if (!Character || !Character->IsValidLowLevel())
		{
			return;
		}

		Character->GetAnimationInstance()->ChangeAnimMontage(DefaultAnimation::Idle);
	}
	break;
	case UMONSTER_TYPE::Sparrow:
	{
		AURCharacter* Character = MeshComp->GetOwner<AURCharacter>();

		if (!Character || !Character->IsValidLowLevel())
		{
			return;
		}

		Character->GetAnimationInstance()->ChangeAnimMontage(DefaultAnimation::Idle);
		Character->AttackOff();
	}
	}

	switch (m_PlayerType)
	{
	case UPLAYER_TYPE::Wizard:
	{
	}
	break;
	case UPLAYER_TYPE::Warrior:
	{
		AWarriorCharacter* Character = MeshComp->GetOwner<AWarriorCharacter>();

		if (!Character || !Character->IsValidLowLevel())
		{
			return;
		}

		if (!Character->GetIsCombating())
		{
			Character->GetAnimationInstance()->ChangeAnimMontage(DefaultAnimation::Idle);
		}
		else
		{
			Character->GetAnimationInstance()->ChangeAnimMontage(WarriorCombatAnimation::CombatIdle);
		}
	}
	break;
	}
}
