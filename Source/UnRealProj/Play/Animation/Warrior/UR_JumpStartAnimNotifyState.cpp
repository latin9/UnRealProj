// Fill out your copyright notice in the Description page of Project Settings.


#include "Play/Animation/Warrior/UR_JumpStartAnimNotifyState.h"
#include "../../Actor/Player/WarriorCharacter.h"

void UUR_JumpStartAnimNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	switch (m_PlayerType)
	{
	case UPLAYER_TYPE::Wizard:
	{
		/*DefaultAnimation Anim = DefaultAnimation::Idle;
		ChnageAnimMontage<APlayCharacter, DefaultAnimation>(MeshComp, Anim);*/
	}
	break;
	case UPLAYER_TYPE::Warrior:
	{
		AWarriorCharacter* Character = MeshComp->GetOwner<AWarriorCharacter>();

		if (!Character || !Character->IsValidLowLevel())
		{
			return;
		}

		Character->GetAnimationInstance()->ChangeAnimMontage(WarriorJumpAnimation::JumpLoop);
	}
	break;
	}
}


