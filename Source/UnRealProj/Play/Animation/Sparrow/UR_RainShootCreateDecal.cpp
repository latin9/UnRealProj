// Fill out your copyright notice in the Description page of Project Settings.


#include "Play/Animation/Sparrow/UR_RainShootCreateDecal.h"
#include "../../Actor/Boss/UR_SparrowSubBoss.h"
#include "../../Actor/Player/WarriorCharacter.h"
#include "../../Actor/Boss/BossObj/Decal/UR_RainPosDecal.h"
#include "../../Actor/Boss/BossObj/UR_RainShootHoleActor.h"
#include "../../Actor/Boss/BossObj/UR_SparrowMeteor.h"

void UUR_RainShootCreateDecal::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	AUR_SparrowSubBoss* Boss = MeshComp->GetOwner<AUR_SparrowSubBoss>();

	if (!Boss || !Boss->IsValidLowLevel())
	{
		return;
	}

	AWarriorCharacter* Player = Boss->GetWorld()->GetFirstPlayerController()->GetPawn<AWarriorCharacter>();

	if (!Player || !Player->IsValidLowLevel())
	{
		return;
	}

	switch (Boss->GetRandAttackNumb())
	{
	case SparrowAttack_Type::RainBowShoot:
	{
		FVector DecalPos;

		FCollisionQueryParams CollisionParams;
		CollisionParams.AddIgnoredActor(Player);
		CollisionParams.AddIgnoredActor(Boss);

		FVector StartPos = Player->GetActorLocation();
		FVector EndPos = StartPos;
		EndPos.Z -= 500.f;

		FHitResult HitResult;

		// �ٴ����� Lay�� �� ��Į�� ��ġ�� ��Ȯ�ϰ� �ٴ��̶� ������ �ϱ� ���ؼ� �Ʒ�ó�� ����
		if (Player->GetWorld()->LineTraceSingleByChannel(HitResult, StartPos, EndPos, ECollisionChannel::ECC_Visibility, CollisionParams))
		{
			// ��Į�� ��ġ�� �浹�� ��ġ�� ����
			DecalPos = HitResult.Location;

			FActorSpawnParameters SpawnParams;
			FTransform SpawnTransform = FTransform(DecalPos);

			AUR_RainPosDecal* Decal = Player->GetWorld()->SpawnActor<AUR_RainPosDecal>(AUR_RainPosDecal::StaticClass(), SpawnTransform, SpawnParams);
			Decal->SetAttackType(Boss->GetRandAttackNumb());
			Decal->SetData(Boss->GetSparrowData());
		}
	}
		break;
	case SparrowAttack_Type::RainBowBurstShoot:
	{
		FVector DecalPos;

		FCollisionQueryParams CollisionParams;
		CollisionParams.AddIgnoredActor(Player);
		CollisionParams.AddIgnoredActor(Boss);

		FVector StartPos = Player->GetActorLocation();
		FVector EndPos = StartPos;
		EndPos.Z -= 500.f;

		FHitResult HitResult;

		// �ٴ����� Lay�� �� ��Į�� ��ġ�� ��Ȯ�ϰ� �ٴ��̶� ������ �ϱ� ���ؼ� �Ʒ�ó�� ����
		if (Player->GetWorld()->LineTraceSingleByChannel(HitResult, StartPos, EndPos, ECollisionChannel::ECC_Visibility, CollisionParams))
		{
			// ��Į�� ��ġ�� �浹�� ��ġ�� ����
			DecalPos = HitResult.Location;

			FActorSpawnParameters SpawnParams;
			FTransform SpawnTransform = FTransform(DecalPos);

			AUR_RainShootHoleActor* Decal = Player->GetWorld()->SpawnActor<AUR_RainShootHoleActor>(AUR_RainShootHoleActor::StaticClass(), SpawnTransform, SpawnParams);

			DecalPos.Z += 400.f;
			FRotator Rot = FRotator(0.0, 0.0, -90.0);
			SpawnTransform = FTransform(Rot, DecalPos);
			AUR_SparrowMeteor* Meteor = Player->GetWorld()->SpawnActorDeferred<AUR_SparrowMeteor>(AUR_SparrowMeteor::StaticClass(), SpawnTransform);
			Meteor->SetInfo(FName(TEXT("MonsterAttack")), 300.f, 3.f);
			//Meteor->SetActorRotation(Rot);
			Meteor->FinishSpawning(SpawnTransform);
			//Decal->SetAttackType(Boss->GetRandAttackNumb());
			//Decal->SetData(Boss->GetSparrowData());
		}
	}
		break;
	}
}
