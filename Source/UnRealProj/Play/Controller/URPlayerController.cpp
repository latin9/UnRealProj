// Fill out your copyright notice in the Description page of Project Settings.


#include "URPlayerController.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "../Actor/Player/WarriorCharacter.h"
#include "GameFramework/PawnMovementComponent.h"

AURPlayerController::AURPlayerController()	:
	m_PlayerToDestDist(120.f)
{
	// ���ӳ����� ���콺 Ŀ���� ���̵��� ����.
	//bShowMouseCursor = true;
}

void AURPlayerController::InputClickPressed()
{
	//m_ClickMouse = true;
	//m_PlayCharacter->SetIsMoving(true);

	//m_PlayCharacter->GetAnimationInstance()->ChangeAnimMontage(DefaultAnimation::Forward);
}

void AURPlayerController::InputClickReleased()
{
	//m_ClickMouse = false;
	//m_PlayCharacter->SetIsMoving(false);
}

void AURPlayerController::SetNewDestination(const FVector& DestLocation)
{
	if (m_PlayCharacter)
	{
		float Dist = FVector::Dist(DestLocation, m_PlayCharacter->GetActorLocation());

		if (Dist > m_PlayerToDestDist)
		{
			// �� �Լ������� �켱 ��Ʈ�ѷ��� �����ϰ� �ִ� ���� �����ͼ� ���� ������ ������ �Ÿ��� �����ؼ�,
			// �� �Ÿ��� 120 �𸮾� ���ֺ��� ũ�� ���� �������� �̵���Ų��.
			UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, DestLocation);

			m_PlayCharacter->GetAnimationInstance()->ChangeAnimMontage(DefaultAnimation::Forward);
		}
	}
}

void AURPlayerController::MoveToMouseCursor()
{
	// MoveToMouseCursor() �Լ��� GetHitResultUnderCursor() �Լ��� ���� ���콺 Ŀ�� �Ʒ��� ���� Ʈ���̽��� ���� 
	// �� ��ġ�� SetNewDestination() �Լ��� �����ϴ� ������ �Ѵ�.
	FHitResult HitResult;
	GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, false, HitResult);

	if (HitResult.bBlockingHit)
	{
		SetNewDestination(HitResult.ImpactPoint);
		m_HitPos = HitResult.ImpactPoint;
	}
}


void AURPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	//InputComponent->BindAction(FName(TEXT("MoveClick")), EInputEvent::IE_Pressed, this, &AURPlayerController::InputClickPressed);
	//InputComponent->BindAction(FName(TEXT("MoveClick")), EInputEvent::IE_Released, this, &AURPlayerController::InputClickReleased);

}

void AURPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	/*if (m_ClickMouse && !m_PlayCharacter->IsAttack())
	{
		MoveToMouseCursor();
	}

	FVector Pos = m_PlayCharacter->GetActorLocation();
	float X = abs(Pos.X - m_HitPos.X);
	float Y = abs(Pos.Y - m_HitPos.Y);
	float Z = abs(Pos.Z - m_HitPos.Z);

	if (X + Y + Z <= 150.f)
	{
		m_HitPos = FVector(0.f, 0.f, 0.f);
		m_PlayCharacter->GetAnimationInstance()->ChangeAnimMontage(DefaultAnimation::Idle);
	}*/
}

void AURPlayerController::BeginPlay()
{
	Super::BeginPlay();

	//m_PlayCharacter = Cast<APlayCharacter>(GetPawn());
}
