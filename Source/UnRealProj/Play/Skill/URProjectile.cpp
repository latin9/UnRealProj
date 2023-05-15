// Fill out your copyright notice in the Description page of Project Settings.


#include "URProjectile.h"
#include "Components/SphereComponent.h"
#include "Components/SceneComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "../Actor/URCharacter.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AURProjectile::AURProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(FName(TEXT("DefaultSceneRoot")));

	m_SphereComponent = CreateDefaultSubobject<USphereComponent>(FName(TEXT("DamageCollision")));

	// �� ����� ����ϸ� ���� ��밡 ���� �浹�ؼ� ������ ���ÿ� ������ ����� ��Ȳ�� �߻����� �ʴ´�.
	// AddDynamic�̶� �������� ���ؼ� �ؾ����� �����ϰ� �ۼ��� �� ����.
	m_SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &AURProjectile::OnCollision);

	RootComponent = m_SphereComponent;

	m_ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(FName(TEXT("ProjectileMovementComponent")));

	// �̷��� �߷��� �ް� �������� �ʴ´�. �̻����� ���� 1�ڷ� ������ ���ư��� ����
	m_ProjectileMovementComponent->ProjectileGravityScale = 0.f;
	m_ProjectileMovementComponent->bRotationFollowsVelocity = true;

	m_Damage = 5.f;
}

// Called when the game starts or when spawned
void AURProjectile::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void AURProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	m_ProjectileMovementComponent->Velocity = m_SkillDir * m_Speed;

	if (0.0f >= (m_LifeTime -= DeltaTime))
	{
		Destroy();
	}
}

void AURProjectile::SetInfo(FName _CollisionProfileName, float _Speed, float _LifeTime)
{
	m_Speed = _Speed;

	m_LifeTime = _LifeTime;

	m_SphereComponent->SetCollisionProfileName(_CollisionProfileName);
}

void AURProjectile::SetInfo(FName _CollisionProfileName, FVector _Dir, float _Speed, float _LifeTime)
{
	m_Speed = _Speed;

	m_LifeTime = _LifeTime;

	m_SkillDir = _Dir;

	m_SphereComponent->SetCollisionProfileName(_CollisionProfileName);
}

void AURProjectile::OnCollision(UPrimitiveComponent* _Component, AActor* _DestActor, UPrimitiveComponent* _DestComponent, int32 _OtherBodyIndex, bool _FromSweep, const FHitResult& _Result)
{
	// �浹������� ����� ������Ʈ�� ���� �� �ִ�.
	AURCharacter* Chracter = Cast<AURCharacter>(_DestActor);

	if (nullptr == Chracter)
	{
		return;
	}

	Chracter->CallDamage(m_Damage, this);

	// ��ġ��� ����
	if (m_HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), m_HitSound, Chracter->GetActorLocation(), 0.3f, 1.f);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Projectile Hit Sound Null!"));
	}
}

