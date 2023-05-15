// Fill out your copyright notice in the Description page of Project Settings.


#include "Play/Animation/Public/UR_CreateObjAnimNotifyState.h"
#include "Global/URBlueprintFunctionLibrary.h"
#include "../../Actor/Boss/UR_KhaimeraBoss.h"
#include "../../Actor/Boss/UR_SparrowSubBoss.h"
#include "../../Actor/Boss/BossObj/UR_SPMuzzleFlash.h"
#include "../../Skill/URProjectile.h"
#include "../../Actor/Monster/Monster.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

void UUR_CreateObjAnimNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	// ������ �÷��̿��尡 NULL�̶�� ����
	if (!UURBlueprintFunctionLibrary::GetCurrentPlayWorld())
	{
		return;
	}
	
	if (!m_SpawnActorClass && !m_SpawnActorClass->IsValidLowLevel())
	{
		return;
	}
	
	switch (m_ActorType)
	{
	case ActorType::Normal:
	{
		TArray<AMonster*> ArrayActors;
		AURCharacter* Player = MeshComp->GetOwner<AURCharacter>();
		
		UWorld* World = Player->GetWorld();
		TActorIterator<AMonster> iter(World);

		for (; iter; ++iter)
		{
			AActor* Actor = *iter;
			double Dist = Player->GetTargetDir(Actor).Size();

			if (Dist < 1000.0)
			{
				ArrayActors.Push((AMonster*)Actor);
			}
		}

		for (auto iter1 : ArrayActors)
		{
			FVector Pos = iter1->GetActorLocation();

			FTransform SpawnTransform = FTransform(Pos);

			// ���ο� ���͸� ������ش�.
			AActor* NewActor = MeshComp->GetWorld()->SpawnActor<AActor>(m_SpawnActorClass, SpawnTransform);
		}


	}
		break;
	case ActorType::Projectile:
	{
		switch (m_MonsterType)
		{
			// �÷��̾�
		case UMONSTER_TYPE::Default:
		{
			// ������ �̸������ͼ� �ش� ������ ��ġ���� ���´�.
			FVector Pos = MeshComp->GetSocketLocation(m_SocketName);
			Pos.Z += m_AddLocationZ;

			FTransform SpawnTransform = FTransform(Pos);

			SpawnTransform.SetRotation(MeshComp->GetOwner()->GetTransform().GetRotation());

			// ���ο� ���͸� ������ش�.
			AActor* NewActor = MeshComp->GetWorld()->SpawnActor<AActor>(m_SpawnActorClass, SpawnTransform);

			AURProjectile* Projectile = Cast<AURProjectile>(NewActor);

			if (!Projectile)
				return;

			// ���⼭ �̻����� �ɼ��� �����ϴ� ������ Ư�� ��Ÿ�ָ��� �̻����� ������ �ٸ��� �� �� �־�̴�.
			Projectile->SetInfo(m_CollisionProfileName, m_Speed, m_LifeTime);
		}
			break;
		case UMONSTER_TYPE::Khaimera:
		{
			AUR_KhaimeraBoss* Boss = MeshComp->GetOwner<AUR_KhaimeraBoss>();

			if (!Boss || !Boss->IsValidLowLevel())
			{
				return;
			}

			if (Boss->GetIsBerserk())
			{
				AURCharacter* Player = Boss->GetWorld()->GetFirstPlayerController()->GetPawn<AURCharacter>();
				// ������ �̸������ͼ� �ش� ������ ��ġ���� ���´�.
				FVector Pos = MeshComp->GetSocketLocation(m_SocketName);
				Pos.Z += m_AddLocationZ;

				FTransform SpawnTransform = FTransform(Pos);

				SpawnTransform.SetRotation(MeshComp->GetOwner()->GetTransform().GetRotation());

				// ���ο� ���͸� ������ش�.
				AActor* NewActor = MeshComp->GetWorld()->SpawnActor<AActor>(m_SpawnActorClass, SpawnTransform);

				AURProjectile* Projectile = Cast<AURProjectile>(NewActor);

				FVector Dir;

				if (!Projectile)
					return;

				if (Player)
				{
					Dir = Player->GetActorLocation() - Projectile->GetActorLocation();

					Dir = Dir.GetSafeNormal();

					// ���⼭ �̻����� �ɼ��� �����ϴ� ������ Ư�� ��Ÿ�ָ��� �̻����� ������ �ٸ��� �� �� �־�̴�.
					Projectile->SetInfo(m_CollisionProfileName, Dir, m_Speed, m_LifeTime);
				}
				else
				{
					// ���⼭ �̻����� �ɼ��� �����ϴ� ������ Ư�� ��Ÿ�ָ��� �̻����� ������ �ٸ��� �� �� �־�̴�.
					Projectile->SetInfo(m_CollisionProfileName, Projectile->GetActorForwardVector(), m_Speed, m_LifeTime);
				}

			}
			
		}
			break;
		case UMONSTER_TYPE::Sparrow:
		{
			AUR_SparrowSubBoss* Boss = MeshComp->GetOwner<AUR_SparrowSubBoss>();

			if (!Boss || !Boss->IsValidLowLevel())
			{
				return;
			}

			switch (Boss->GetRandAttackNumb())
			{
			case SparrowAttack_Type::Attack1:
				break;
			case SparrowAttack_Type::Attack2:
				CreateSparrowSP4Shoot(MeshComp, Boss);
				break;
			case SparrowAttack_Type::Attack3:
				CreateSparrowSP5Shoot(MeshComp, Boss);
				break;
			case SparrowAttack_Type::Attack4:
				CreateSparrowBurstShoot(MeshComp, Boss);
				break;
			case SparrowAttack_Type::RainBowShoot:
				break;
			case SparrowAttack_Type::RainBowBurstShoot:
				break;
			}
		}
			break;
		}
		
	}
		break;
	}
	
}

void UUR_CreateObjAnimNotifyState::CreateSparrowSP4Shoot(USkeletalMeshComponent* _MeshComp, AUR_SparrowSubBoss* _Boss)
{
	AURCharacter* Player = _Boss->GetWorld()->GetFirstPlayerController()->GetPawn<AURCharacter>();
	// ������ �̸������ͼ� �ش� ������ ��ġ���� ���´�.
	FVector Pos = _MeshComp->GetSocketLocation(m_SocketName);
	Pos.Z += m_AddLocationZ;

	//FRotator Rot = FRotator(90, 0.0, -90.0 + i * 20.0);

	FTransform SpawnTransform = FTransform(Pos);

	AActor* NewActor = _MeshComp->GetWorld()->SpawnActorDeferred<AActor>(m_SpawnActorClass, SpawnTransform);

	NewActor->FinishSpawning(SpawnTransform);

	AURProjectile* Projectile = Cast<AURProjectile>(NewActor);

	if (!Projectile)
		return;

	FVector Dir = (Player->GetActorLocation() - Projectile->GetActorLocation()).GetSafeNormal();

	// ���⼭ �̻����� �ɼ��� �����ϴ� ������ Ư�� ��Ÿ�ָ��� �̻����� ������ �ٸ��� �� �� �־�̴�.
	Projectile->SetInfo(m_CollisionProfileName, Dir, m_Speed, m_LifeTime);
}

void UUR_CreateObjAnimNotifyState::CreateSparrowSP5Shoot(USkeletalMeshComponent* _MeshComp, AUR_SparrowSubBoss* _Boss)
{
	AURCharacter* Player = _Boss->GetWorld()->GetFirstPlayerController()->GetPawn<AURCharacter>();
	// ������ �̸������ͼ� �ش� ������ ��ġ���� ���´�.
	FVector Pos = _MeshComp->GetSocketLocation(m_SocketName);
	Pos.Z += m_AddLocationZ;

	FTransform SpawnTransform = FTransform(Pos);

	AActor* NewActor = _MeshComp->GetWorld()->SpawnActorDeferred<AActor>(m_SpawnActorClass, SpawnTransform);
	AUR_SPMuzzleFlash* MuzzleActor = _MeshComp->GetWorld()->SpawnActorDeferred<AUR_SPMuzzleFlash>(AUR_SPMuzzleFlash::StaticClass(), SpawnTransform);
	MuzzleActor->SetMuzzleType(SPMuzzle_Type::EvadeShoot);

	NewActor->FinishSpawning(SpawnTransform);
	MuzzleActor->FinishSpawning(SpawnTransform);

	AURProjectile* Projectile = Cast<AURProjectile>(NewActor);

	if (!Projectile)
		return;

	FVector Dir = (Player->GetActorLocation() - Projectile->GetActorLocation()).GetSafeNormal();

	// ���⼭ �̻����� �ɼ��� �����ϴ� ������ Ư�� ��Ÿ�ָ��� �̻����� ������ �ٸ��� �� �� �־�̴�.
	Projectile->SetInfo(m_CollisionProfileName, Dir, m_Speed, m_LifeTime);
}

void UUR_CreateObjAnimNotifyState::CreateSparrowBurstShoot(USkeletalMeshComponent* _MeshComp, AUR_SparrowSubBoss* _Boss)
{
	AURCharacter* Player = _Boss->GetWorld()->GetFirstPlayerController()->GetPawn<AURCharacter>();
	// ������ �̸������ͼ� �ش� ������ ��ġ���� ���´�.
	FVector Pos = _MeshComp->GetSocketLocation(m_SocketName);
	Pos.Z += m_AddLocationZ;

	m_ArrowArray.Empty();

	//SpawnTransform.SetRotation(_MeshComp->GetOwner()->GetTransform().GetRotation());

	for (int i = 0; i < 5; ++i)
	{
		FRotator Rot = FRotator(90, 0.0, -90.0 + i * 20.0);

		FTransform SpawnTransform = FTransform(Rot, Pos);

		AActor* NewActor = _MeshComp->GetWorld()->SpawnActorDeferred<AActor>(m_SpawnActorClass, SpawnTransform);

		m_ArrowArray.Add({ NewActor, SpawnTransform });
	}
	for (int i = 0; i < m_ArrowArray.Num(); ++i)
	{
		m_ArrowArray[i].first->FinishSpawning(m_ArrowArray[i].second);

		AURProjectile* Projectile = Cast<AURProjectile>(m_ArrowArray[i].first);

		FVector Dir;

		if (!Projectile)
			return;

		// ���⼭ �̻����� �ɼ��� �����ϴ� ������ Ư�� ��Ÿ�ָ��� �̻����� ������ �ٸ��� �� �� �־�̴�.
		Projectile->SetInfo(m_CollisionProfileName, Projectile->GetActorForwardVector(), m_Speed, m_LifeTime);
	}

	
}
