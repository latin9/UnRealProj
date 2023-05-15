// Fill out your copyright notice in the Description page of Project Settings.


#include "URCharacter.h"
#include "Components/SphereComponent.h"
#include "Global/URGameInstance.h"
#include "Global/URStructs.h"
#include "Item/URItemActor.h"
#include "Sound/SoundBase.h"
#include "Kismet/GamePlayStatics.h"
#include "Components/CapsuleComponent.h"

// Sets default values
AURCharacter::AURCharacter()	:
	m_QuestProgress(QuestProgress::Default),
	m_HitType(EHitType::Default),
	m_HitDir(EHitDir::Default),
	m_IsBlocking(false),
	m_IsInvincibility(false),
	m_IsAttack(false),
	m_KnockBackHitPower(2500.f),
	m_KnockDownHitPower(3000.f),
	m_IsQuestCompletion(false),
	m_IsQuesting(false)
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Tags.Add(FName("URCharacter"));


	GetMesh()->bHiddenInSceneCapture = true;

	static ConstructorHelpers::FObjectFinder<USoundBase> HitSound(TEXT("SoundCue'/Game/Resource/Play/Sound/SKill/Hit/NewSoundCue.NewSoundCue'"));

	if (HitSound.Succeeded())
		m_HitSound = HitSound.Object;

}

AActor* AURCharacter::TargetSearch(FName _Name, float _Range)
{
	TArray<AActor*> Actors;

	UGameplayStatics::GetAllActorsWithTag(GetWorld(), _Name, Actors);

	if (Actors.IsEmpty())
	{
		return nullptr;
	}

	if (0.f >= _Range)
	{
		return Actors[0];
	}

	FVector Location = GetActorLocation();

	size_t Size = Actors.Num();

	for (size_t i = 0; i < Size; ++i)
	{
		// �÷��̾�� ���͵鰣�� ��ġ���� �̿��ؼ� �Ÿ��� ���Ѵ�.
		float Length = (Location - Actors[i]->GetActorLocation()).Size();

		if (_Range > Length)
		{
			return Actors[i];
		}
	}

	return nullptr;
}

void AURCharacter::ReSetHPPercent()
{
	m_HPPercent = GetHP() / m_PlayerInfo->MaxHP;
}

double AURCharacter::GetMaxHP()
{
	return m_PlayerInfo->MaxHP;
}

double AURCharacter::GetStamina()
{
	return m_Stamina;
}

double AURCharacter::GetMaxStamina()
{
	return m_PlayerInfo->MaxStamina;
}

bool AURCharacter::GetIsRangeInTarget(AActor* _Target, float _Length)
{
	return GetTargetDir(_Target).Size() <= _Length;
}

FVector AURCharacter::GetTargetDir(AActor* _Target)
{
	return _Target->GetActorLocation() - GetActorLocation();
}

FVector AURCharacter::GetTargetDir(const FVector& _TargetPos)
{
	return _TargetPos - GetActorLocation();
}

FVector AURCharacter::GetTargetDirNormalize(AActor* _Target) 
{
	return GetTargetDir(_Target).GetSafeNormal();
}

FVector AURCharacter::GetTargetDirNormalize(const FVector& _TargetPos)
{
	return GetTargetDir(_TargetPos).GetSafeNormal();
}

void AURCharacter::SetTargetMovementInput(AActor* _Target, float Value)
{
	SetDirMovementInput(GetTargetDirNormalize(_Target), 1.f);
}

void AURCharacter::SetDirMovementInput(const FVector& _Dir, float Value)
{
	AddMovementInput(_Dir, 1.f);
}

void AURCharacter::SetTargetLook(AActor* _Target)
{
	FRotator Rot = GetTargetDirNormalize(_Target).Rotation();
	Rot.Pitch = 0.0;
	SetActorRotation(Rot);
}

void AURCharacter::SetDirLook(const FVector& _Dir)
{
	FVector Forward = GetActorForwardVector() * 10.f;
	Forward.Z = 0.f;

	FVector Dir = _Dir;
	Dir = Dir.GetSafeNormal();

	Dir += Forward;

	SetActorRotation(Dir.GetSafeNormal().Rotation());
}

TArray<AActor*> AURCharacter::TargetsSearch(FName _Name, float _Range)
{
	TArray<AActor*> Actors;
	TArray<AActor*> SelectActors;

	UGameplayStatics::GetAllActorsWithTag(GetWorld(), _Name, Actors);

	if (Actors.IsEmpty())
	{
		return Actors;
	}

	if (0 >= _Range)
	{
		return Actors;
	}

	FVector Location = GetActorLocation();

	for (size_t i = 0; i < Actors.Num(); i++)
	{
		float Len = (Location - Actors[i]->GetActorLocation()).Size();

		if (_Range > Len)
		{
			SelectActors.Add(Actors[i]);
		}
	}

	return SelectActors;
}

void AURCharacter::CallDamage(double _Damage, AActor* _Actor, bool _IsKnockBack, bool _IsCameraShake)
{
	HitDirJudge(_Actor);
	if (!m_IsBlocking)
		m_HP -= _Damage;
	else
	{
		// ���鿡�� ���ݹ�����쿡�� �������� ���� �ʰ� �˹鸸 ����.
		switch (m_HitDir)
		{
		case EHitDir::Forward:
			break;
		case EHitDir::Backward:
		case EHitDir::Left:
		case EHitDir::Right:
			m_HP -= _Damage;
			break;
		}
	}

	m_HPPercent = m_HP / m_MaxHP;

	if (_IsKnockBack)
	{
		FVector Dir = GetActorLocation() - _Actor->GetActorLocation();

		Dir = Dir.GetSafeNormal();

		Dir.Z = 0.0;
		switch (m_HitType)
		{
		case EHitType::NormalHit:
			LaunchCharacter(Dir * m_KnockBackHitPower, true, false);
			break;
		case EHitType::KnockDownHit:
			LaunchCharacter(Dir * m_KnockDownHitPower, true, false);
			break;
		}
	}

}

TArray<FHitResult> AURCharacter::CollisionCheck(const FVector& Start, const FVector& End, const FQuat& Rot, FName ProfileName, const FCollisionShape& CollisionShape)
{
	FCollisionQueryParams CollisionCheck(FName(TEXT("Collision Check")), false, this);

	TArray<FHitResult> Result = TArray<FHitResult>();

	GetWorld()->SweepMultiByProfile(Result, Start, End, Rot, ProfileName, CollisionShape, CollisionCheck);

	return Result;
}

TArray<UActorComponent*> AURCharacter::GetDamageCollision()
{
	return GetComponentsByTag(USphereComponent::StaticClass(), FName(TEXT("Damage")));
}

void AURCharacter::ItemDrop(const TArray<const struct FURItemData*>& _Array)
{
	UURGameInstance* Instance = GetWorld()->GetGameInstance<UURGameInstance>();

	if (!Instance)
	{
		return;
	}

	TSubclassOf<UObject> Object = Instance->GetGetObjectData(FName("Item"));

	for (size_t i = 0; i < _Array.Num(); ++i)
	{
		FHitResult HitResult;

		FVector EndPos = GetActorLocation() + FVector(0.0, 0.0, -1.0) * 1000.f;

		FCollisionQueryParams CollisionParams;
		CollisionParams.AddIgnoredActor(this);
		CollisionParams.AddIgnoredComponent(GetCapsuleComponent());
		CollisionParams.bReturnPhysicalMaterial = true;

		if (GetWorld()->LineTraceSingleByChannel(HitResult, GetActorLocation(), EndPos, ECollisionChannel::ECC_Visibility, CollisionParams))
		{
			AActor* This = this;
			FActorSpawnParameters SpawnParams;
			// ������ ��ü�� �θ��� �浹 ���� ������ �о���ϴ�.
			// ���� ���Ŀ� �ٷ� �浹�� �ǰԵǸ� �������� �з� ���ŵ� ���� SetItem�� ����� ���� ����.
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
			FTransform SpawnTransform(HitResult.TraceEnd + FVector(0.0, 0.0, 50.0));
			AURItemActor* NewActor = GetWorld()->SpawnActor<AURItemActor>(Object, SpawnTransform, SpawnParams);
			NewActor->SetItem(_Array[i]);
		}

	}
}

float AURCharacter::GetLastPathPointToTargetDis(const FVector& _TargetPos)
{
	FVector LastPoint = m_Path->PathPoints.Last();
	FVector StartPoint = _TargetPos;

	LastPoint.Z = 0.f;
	StartPoint.Z = 0.f;

	return (LastPoint - StartPoint).Size();
}

UNavigationPath* AURCharacter::PathFind(AActor* _Actor)
{
	if (nullptr == _Actor)
	{
		return nullptr;
	}

	return PathFind(_Actor->GetActorLocation());
}

UNavigationPath* AURCharacter::PathFind(const FVector& _TargetPosition)
{
	return UNavigationSystemV1::FindPathToLocationSynchronously(GetWorld(), GetActorLocation(), _TargetPosition);
}

bool AURCharacter::PathMove()
{
	if (nullptr == m_Path)
	{
		return false;
	}

	if (0 == m_Path->PathPoints.Num())
	{
		return false;
	}

	if (m_Path->PathPoints.IsEmpty())
	{
		return false;
	}

	FVector TargetPos = m_Path->PathPoints[0];
	FVector ActorPos = GetActorLocation();

	TargetPos.Z = 0.f;
	ActorPos.Z = 0.f;

	FVector Dir = TargetPos - ActorPos;

	if (50.f >= Dir.Size())
	{
		// ù��° �ε��� ����
		m_Path->PathPoints.RemoveAt(0);
		
		if (0 == m_Path->PathPoints.Num())
		{
			return false;
		}

		TargetPos = m_Path->PathPoints[0];
		TargetPos.Z = 0.f;
		Dir = TargetPos - ActorPos;
	}

	SetDirLook(Dir);
	SetDirMovementInput(Dir);
	GetAnimationInstance()->ChangeAnimMontage(DefaultAnimation::Forward);

	return true;
}

bool AURCharacter::NoAnimPathMove()
{
	if (nullptr == m_Path)
	{
		return false;
	}

	if (0 == m_Path->PathPoints.Num())
	{
		return false;
	}

	if (m_Path->PathPoints.IsEmpty())
	{
		return false;
	}

	FVector TargetPos = m_Path->PathPoints[0];
	FVector ActorPos = GetActorLocation();

	TargetPos.Z = 0.f;
	ActorPos.Z = 0.f;

	FVector Dir = TargetPos - ActorPos;

	if (50.f >= Dir.Size())
	{
		// ù��° �ε��� ����
		m_Path->PathPoints.RemoveAt(0);

		if (0 == m_Path->PathPoints.Num()) {
			return false;
		}

		TargetPos = m_Path->PathPoints[0];
		TargetPos.Z = 0.f;
		Dir = TargetPos - ActorPos;
	}

	//SetDirLook(Dir);
	SetDirMovementInput(Dir);

	return true;
}

void AURCharacter::SetPath(UNavigationPath* _Path, bool _Debug)
{
	m_Path = _Path;

	if (false == _Debug)
	{
		return;
	}

	FColor Color = FColor::Black;
	//                   0�� �ȳ����� �ϱ� ���ؼ�
	// zero ������� �������ؼ�
	int PathColor = 255 / m_Path->PathPoints.Num() + 1;

	for (size_t i = 0; i < m_Path->PathPoints.Num() - 1; i++)
	{
		FVector Start = m_Path->PathPoints[i];
		FVector End = m_Path->PathPoints[i + 1];

		Color.R += PathColor;

		//DrawDebugLine(GetWorld(), Start, End, Color, false, 1000.0f, 0, 10.0f);
	}
}

void AURCharacter::ResetPath()
{
	SetPath(nullptr, false);
}

// Called when the game starts or when spawned
void AURCharacter::BeginPlay()
{
	Super::BeginPlay();

	m_AnimationInstance = Cast<UURAnimInstance>(GetMesh()->GetAnimInstance());

	if (nullptr == m_AnimationInstance || false == m_AnimationInstance->IsValidLowLevel())
	{
		return;
	}

	if (m_Animations.IsEmpty())
	{
		return;
	}

	for (auto& Anim : m_Animations)
	{
		GetAnimationInstance()->AddAnimMontage(static_cast<int>(Anim.Key), Anim.Value);
	}
	
	// �⺻������ Idle���·� ����
	GetAnimationInstance()->SetDefaultMontage(DefaultAnimation::Idle);
	m_AnimationInstance->ChangeAnimMontage(DefaultAnimation::Idle);
}

// Called every frame
void AURCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AURCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AURCharacter::HitDirJudge(AActor* _Actor)
{
	FVector MonsterPos = _Actor->GetActorLocation();
	FVector PlayerPos = GetActorLocation();
	FVector MonsterDir = MonsterPos - PlayerPos;
	MonsterDir = MonsterDir.GetSafeNormal();

	// ������ ���� �ڻ��μ�Ÿ ���� ���Ѵ���
	double FValue = FVector::DotProduct(MonsterDir, GetActorForwardVector());
	double BValue = FVector::DotProduct(MonsterDir, -GetActorForwardVector());
	double LValue = FVector::DotProduct(MonsterDir, -GetActorRightVector());
	double RValue = FVector::DotProduct(MonsterDir, GetActorRightVector());

	// ��ũ�ڻ��ΰ� ���� �� ��׸��� �̿��ؼ� ��׸��� ������ ��ȯ�Ͽ�
	// �����ġ���� �����ߴ����� �Ǵ��Ѵ�.
	float FConvert = acosf(static_cast<float>(FValue));
	FConvert = FMath::RadiansToDegrees(FConvert);
	float BConvert = acosf(static_cast<float>(BValue));
	BConvert = FMath::RadiansToDegrees(BConvert);
	float LConvert = acosf(static_cast<float>(LValue));
	LConvert = FMath::RadiansToDegrees(LConvert);
	float RConvert = acosf(static_cast<float>(RValue));
	RConvert = FMath::RadiansToDegrees(RConvert);

	if (FConvert < 45.f)
	{
		m_HitDir = EHitDir::Forward;
	}
	else if (BConvert < 45.f)
	{
		m_HitDir = EHitDir::Backward;
	}
	else if (RConvert < 45.f)
	{
		m_HitDir = EHitDir::Right;
	}
	else if (LConvert < 45.f)
	{
		m_HitDir = EHitDir::Left;
	}
	else
	{
		m_HitDir = EHitDir::Default;
	}
}

void AURCharacter::CharacterSoundPlay(USoundBase* _Sound, float VolumeMultiplier, float PitchMultiplier)
{
	if (GetWorld() != nullptr)
		UGameplayStatics::PlaySound2D(GetWorld(), _Sound, VolumeMultiplier, PitchMultiplier);

}

void AURCharacter::CharacterSoundPlay(USoundBase* _Sound, USceneComponent* _AttachComponent, FName _AttachPointName, 
	float VolumeMultiplier, float PitchMultiplier, FVector Location, EAttachLocation::Type LocationType, bool bStopWhenAttachedToDestroyed)
{
	if (_Sound)
	{
		UGameplayStatics::SpawnSoundAttached(_Sound, _AttachComponent, _AttachPointName, Location, EAttachLocation::SnapToTarget, false, VolumeMultiplier, PitchMultiplier);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("CharacterSoundPlay Func _Sound Null!"));
	}
}

