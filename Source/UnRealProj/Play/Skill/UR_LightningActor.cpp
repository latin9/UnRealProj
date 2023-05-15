// Fill out your copyright notice in the Description page of Project Settings.


#include "Play/Skill/UR_LightningActor.h"
#include "Components/SphereComponent.h"
#include "GameFramework/PlayerController.h"
#include "../Actor/Player/WarriorCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Global/URStructs.h"
#include "Global/URBlueprintFunctionLibrary.h"
#include "Global/URGameInstance.h"

// Sets default values
AUR_LightningActor::AUR_LightningActor()	:
	m_DestroyTime(0.7f)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	m_SphereComponent = CreateDefaultSubobject<USphereComponent>(FName(TEXT("LightningCollision")));
	m_SphereComponent->SetRelativeScale3D(FVector(3.f, 3.f, 3.f));
	m_SphereComponent->SetCollisionProfileName(FName(TEXT("MonsterAttack")));
	RootComponent = m_SphereComponent;
	// �� ����� ����ϸ� ���� ��밡 ���� �浹�ؼ� ������ ���ÿ� ������ ����� ��Ȳ�� �߻����� �ʴ´�.
	// AddDynamic�̶� �������� ���ؼ� �ؾ����� �����ϰ� �ۼ��� �� ����.
	m_SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &AUR_LightningActor::OnCollision);

	m_FXComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("LightningParticle"));
	static ConstructorHelpers::FObjectFinder<UParticleSystem> ParticleSystem(TEXT("ParticleSystem'/Game/Resource/Play/Effect/FXVarietyPack/Particles/P_ky_lightning3.P_ky_lightning3'"));

	if (ParticleSystem.Succeeded())
	{
		m_FXComponent->SetTemplate(ParticleSystem.Object);
	}
	m_FXComponent->SetupAttachment(m_SphereComponent);

	{
		static ConstructorHelpers::FObjectFinder<USoundBase> SpawnSound(TEXT("SoundWave'/Game/Resource/Play/Sound/SKill/Sparrow/LightningSound.LightningSound'"));

		if (SpawnSound.Succeeded())
			m_LightningSound = SpawnSound.Object;
	}
}

// Called when the game starts or when spawned
void AUR_LightningActor::BeginPlay()
{
	Super::BeginPlay();

	m_Data = UURBlueprintFunctionLibrary::GetURInst()->GetMonsterData(FName(TEXT("Sparrow")));

	if (!m_Data)
	{
		UE_LOG(LogTemp, Error, TEXT("SparrowData Empty"));
	}


	AWarriorCharacter* Player = Cast<AWarriorCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	if (!Player)
	{
		return;
	}

	Player->CameraShake(CameraShake_Type::BigShake);

	// ��ġ��� ����
	if (m_LightningSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), m_LightningSound, RootComponent->GetComponentLocation(), 0.5f, 1.f);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("SP Lightning Hit Sound Null!"));
	}

}

// Called every frame
void AUR_LightningActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	m_DestroyTime -= DeltaTime;

	if (m_DestroyTime <= 0.f)
	{
		Destroy();
	}
}

void AUR_LightningActor::OnCollision(UPrimitiveComponent* _Component, AActor* _DestActor, UPrimitiveComponent* _DestComponent, int32 _OtherBodyIndex, bool _FromSweep, const FHitResult& _Result)
{
	// �浹������� ����� ������Ʈ�� ���� �� �ִ�.
	AURCharacter* Chracter = Cast<AURCharacter>(_DestActor);

	if (nullptr == Chracter)
	{
		return;
	}

	double RandDamage = GetWorld()->GetGameInstance<UURGameInstance>()->GetRandomStream().RandRange(m_Data->MinAttack, m_Data->MaxAttack);

	Chracter->CallDamage(RandDamage, this);

	m_SphereComponent->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
}

