// Fill out your copyright notice in the Description page of Project Settings.


#include "Play/Animation/AnimNotify/UR_FootStepSoundNotify.h"
#include "../../Actor/URCharacter.h"
#include "Components/AudioComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AISense_Hearing.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/CapsuleComponent.h"

UUR_FootStepSoundNotify::UUR_FootStepSoundNotify()
{
	static ConstructorHelpers::FObjectFinder<USoundBase> Dirt(TEXT("SoundCue'/Game/Resource/Play/Sound/Step/SC_Dirt.SC_Dirt'"));

	if (Dirt.Succeeded())
		m_Dirty = Dirt.Object;


	static ConstructorHelpers::FObjectFinder<USoundBase> Modern(TEXT("SoundCue'/Game/Resource/Play/Sound/Step/SC_Modern.SC_Modern'"));

	if (Modern.Succeeded())
		m_Modern = Modern.Object;


	static ConstructorHelpers::FObjectFinder<USoundBase> Snow(TEXT("SoundCue'/Game/Resource/Play/Sound/Step/SC_Snow.SC_Snow'"));

	if (Snow.Succeeded())
		m_Snow = Snow.Object;


	static ConstructorHelpers::FObjectFinder<USoundBase> Stone(TEXT("SoundCue'/Game/Resource/Play/Sound/Step/SC_Stone.SC_Stone'"));

	if (Stone.Succeeded())
		m_Stone = Stone.Object;
}

void UUR_FootStepSoundNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	
	AURCharacter* Character = MeshComp->GetOwner<AURCharacter>();

	if (!Character || !Character->IsValidLowLevel())
	{
		return;
	}

	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(Character);
	CollisionParams.AddIgnoredComponent(Character->GetCapsuleComponent());
	CollisionParams.bReturnPhysicalMaterial = true;
	
	FVector StartPos = MeshComp->GetOwner()->GetActorLocation();
	FVector EndPos = StartPos;
	EndPos.Z -= 300.f;

	double Length = Character->GetVelocity().Length();

	// �޸��� or �ȱ� �ӵ��� ���� �߼Ҹ� ������ ����
	double OutValue = UKismetMathLibrary::MapRangeClamped(Length, 0.0, 1000.0, 0.0, 1.0);

	// �����ͺ��� ����
	double StepSoundM = 0.5;

	OutValue *= StepSoundM;

	FHitResult HitResult;
	
	// �ٴ����� Lay�� �� ������ ���׸����� �Ǵ��� �ش� Step Sound�� ����ϵ��� ����
	if (Character->GetWorld()->LineTraceSingleByChannel(HitResult, StartPos, EndPos, 
		ECollisionChannel::ECC_Visibility, CollisionParams))
	{
		USoundBase* Sound = nullptr;

		m_StepSurface = UGameplayStatics::GetSurfaceType(HitResult);

		if(!m_StepSurface)
		{
			UE_LOG(LogTemp, Error, TEXT("FootStep Surface Null!"));
		}

		SelectSoundBase(&Sound);

		if (!Sound)
		{
			UE_LOG(LogTemp, Error, TEXT("FootStep Sound Null!"));
		}
		
		UAudioComponent* AudioComp = UGameplayStatics::SpawnSoundAttached(Sound, MeshComp, 
			FName(TEXT("root")), FVector(), EAttachLocation::SnapToTarget,
			false, OutValue, OutValue);

		/*UAISense_Hearing::ReportNoiseEvent(Character->GetWorld(), HitResult.Location, 
			StepSoundM, Character, 0.f, FName(TEXT("Detected")));*/
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("FootStep LineTrace Fail!"));
		
		return;
	}

}

void UUR_FootStepSoundNotify::SelectSoundBase(USoundBase** _SoundBase)
{
	switch (m_StepSurface) 
	{
	case SurfaceType_Default:
		break;
	case SurfaceType1:
		*_SoundBase = m_Dirty;
		break;
	case SurfaceType2:
		*_SoundBase = m_Modern;
		break;
	case SurfaceType3:
		break;
	case SurfaceType4:
		break;
	case SurfaceType5:
		*_SoundBase = m_Snow;
		break;
	case SurfaceType6:
		*_SoundBase = m_Stone;
		break;
	}
}
