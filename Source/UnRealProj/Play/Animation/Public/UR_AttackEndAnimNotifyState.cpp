// Fill out your copyright notice in the Description page of Project Settings.


#include "Play/Animation/Public/UR_AttackEndAnimNotifyState.h"
#include "../../Actor/Player/WarriorCharacter.h"

void UUR_AttackEndAnimNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	switch (m_PlayerType)
	{
	case UPLAYER_TYPE::Wizard:
	{
	}
		break;
	case UPLAYER_TYPE::Warrior:
	{
		WarriorCombatAnimation Anim = WarriorCombatAnimation::CombatIdle;
		ChnageAnimMontage<AWarriorCharacter, WarriorCombatAnimation>(MeshComp, Anim);
	}
		break;
	}
}
