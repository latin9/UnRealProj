// Fill out your copyright notice in the Description page of Project Settings.


#include "Play/Animation/Warrior/UR_UltimateAttackAnimNotifyState.h"
#include "../../Actor/Player/WarriorCharacter.h"
#include "Kismet/KismetSystemLibrary.h"

void UUR_UltimateAttackAnimNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	m_Player = MeshComp->GetOwner<AWarriorCharacter>();

	if (!m_Player || !m_Player->IsValidLowLevel())
	{
		return;
	}

	m_Player->UltimateAttack();
	
}

void UUR_UltimateAttackAnimNotifyState::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	if (!m_Player || !m_Player->IsValidLowLevel())
	{
		return;
	}
}

void UUR_UltimateAttackAnimNotifyState::Finished()
{
}
