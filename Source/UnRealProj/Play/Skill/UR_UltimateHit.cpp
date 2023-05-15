// Fill out your copyright notice in the Description page of Project Settings.


#include "Play/Skill/UR_UltimateHit.h"
#include "Components/SphereComponent.h"
#include "../Actor/URCharacter.h"
#include "../Actor/Monster/Monster.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AUR_UltimateHit::AUR_UltimateHit()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	m_SphereComponent = CreateDefaultSubobject<USphereComponent>(FName(TEXT("UltimateCollision")));
	m_SphereComponent->SetRelativeScale3D(FVector(2.f, 2.f, 2.f));
	m_SphereComponent->SetCollisionProfileName(FName(TEXT("PlayerAttack")));
	RootComponent = m_SphereComponent;
	// �� ����� ����ϸ� ���� ��밡 ���� �浹�ؼ� ������ ���ÿ� ������ ����� ��Ȳ�� �߻����� �ʴ´�.
	// AddDynamic�̶� �������� ���ؼ� �ؾ����� �����ϰ� �ۼ��� �� ����.
	m_SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &AUR_UltimateHit::OnCollision);

}

// Called when the game starts or when spawned
void AUR_UltimateHit::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AUR_UltimateHit::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AUR_UltimateHit::OnCollision(UPrimitiveComponent* _Component, AActor* _DestActor, UPrimitiveComponent* _DestComponent,
	int32 _OtherBodyIndex, bool _FromSweep, const FHitResult& _Result)
{
	// �浹������� ����� ������Ʈ�� ���� �� �ִ�.
	AURCharacter* Character = Cast<AURCharacter>(_DestActor);

	AURCharacter* Player = GetWorld()->GetFirstPlayerController()->GetPawn<AURCharacter>();

	if (nullptr == Character)
	{
		return;
	}

	m_Dir = Character->GetActorLocation() - Player->GetActorLocation();
	m_Dir.GetSafeNormal();

	Character->CallDamage(m_Damage);

	AMonster* Monster = Cast<AMonster>(Character);
	Monster->SetUltimateHitDir(m_Dir);
	Monster->SetUltimateHitEnable(true);
	Monster->GetCharacterMovement()->MaxAcceleration = 3000.f;	// 300
	Monster->GetCharacterMovement()->MaxWalkSpeed = 4000.f;	//400

	m_SphereComponent->DestroyComponent();
}

