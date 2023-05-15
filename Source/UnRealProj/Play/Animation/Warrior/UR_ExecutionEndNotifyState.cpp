// Fill out your copyright notice in the Description page of Project Settings.


#include "Play/Animation/Warrior/UR_ExecutionEndNotifyState.h"
#include "../../Actor/Player/WarriorCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "../../Actor/NPC/UR_NPCFirst.h"

void UUR_ExecutionEndNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, 
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	AWarriorCharacter* Character = MeshComp->GetOwner<AWarriorCharacter>();

	if (!Character || !Character->IsValidLowLevel())
	{
		return;
	}

	Character->GetController();
	Character->EnableInput(Character->GetController<APlayerController>());
	Character->GetCameraSpringArmComponent()->bInheritYaw = false;


	FVector vecSpawnPos = Character->GetActorLocation() + Character->GetActorForwardVector() * 500.f;

	FRotator Rot = (-Character->GetActorForwardVector()).Rotation();
	FTransform SpawnTransform = FTransform(Rot, vecSpawnPos);

	AActor* NewActor = Character->GetWorld()->SpawnActorDeferred<AActor>(m_SpawnActorClass, SpawnTransform);
	NewActor->FinishSpawning(SpawnTransform);
}
