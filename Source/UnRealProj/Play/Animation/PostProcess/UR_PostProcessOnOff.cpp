// Fill out your copyright notice in the Description page of Project Settings.


#include "Play/Animation/PostProcess/UR_PostProcessOnOff.h"
#include "../../Actor/Player/WarriorCharacter.h"

void UUR_PostProcessOnOff::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	m_Player = MeshComp->GetOwner<AWarriorCharacter>();

	if (!m_Player || !m_Player->IsValidLowLevel())
	{
		return;
	}

	// true�� ȿ���� Ų��.(�߰�)
	if (m_IsOnOff)
	{
		m_Player->AddPostProcessDrunkMat();
	}
	// false �� ȿ���� ����.(����)
	else
	{
		m_Player->DeletePostProcessDrunkMat();
	}
}
