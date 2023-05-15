// Fill out your copyright notice in the Description page of Project Settings.


#include "../BossObj/UR_SPMuzzleFlash.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
AUR_SPMuzzleFlash::AUR_SPMuzzleFlash()	:
	m_MuzzleType(SPMuzzle_Type::EvadeShoot),
	m_DestroyTime(1.f)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UParticleSystem> MuzzleParticleSystem(TEXT("ParticleSystem'/Game/ParagonWraith/FX/Particles/Abilities/ScopedShot/FX/P_Wraith_Sniper_MuzzleFlash.P_Wraith_Sniper_MuzzleFlash'"));

	m_MuzzleFlashFX = MuzzleParticleSystem.Object;

	// Parameters can be set like this (see documentation for further info) - the names and type must match the user exposed parameter in the Niagara System
	m_MuzzleComponent = CreateDefaultSubobject<UParticleSystemComponent>(FName(TEXT("MuzzleFlashComponent")));
	m_MuzzleComponent->SetupAttachment(RootComponent);

	// ã�Ҵ��� ��ã�Ҵ��� �α� ����ش�.
	// BeginPlay���� Template�������ְ� �ֱ⶧���� ���ִ°� ����.
	// ���� ���� �����Ҷ��� �α� ����ֱ���.
	if (!MuzzleParticleSystem.Succeeded())
	{
		UE_LOG(LogTemp, Error, TEXT("MuzzleParticleSystem Find Error!"));
	}
	
	m_MuzzleComponent->SetRelativeScale3D(FVector(10.0, 10.0, 10.0));

}

void AUR_SPMuzzleFlash::BeginPlay()
{
	Super::BeginPlay();
	
	// �������͵��۵�� �������Ѵ��� �ǴϽ������� �ϱ� ���� �ش� Ÿ���� �������ָ�
	// ���� ��µǱ�����(BeginPlay �����ϱ� ��) Ÿ���� �����Ǳ� ������ BeginPlay����
	// Ÿ�Կ����� Template�� �������ش�.
	switch (m_MuzzleType)
	{
	case SPMuzzle_Type::EvadeShoot:
		m_MuzzleComponent->SetTemplate(m_MuzzleFlashFX);
		break;
	}
}

// Called every frame
void AUR_SPMuzzleFlash::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	m_DestroyTime -= DeltaTime;

	if (m_DestroyTime <= 0.f)
	{
		Destroy();
	}
}

