// Fill out your copyright notice in the Description page of Project Settings.


#include "Play/AI/Pirate/UR_BlockingMoveTaskNode.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "../../Controller/URAIController.h"
#include "../../Actor/Monster/Monster.h"
#include "../../Actor/Player/WarriorCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

UUR_BlockingMoveTaskNode::UUR_BlockingMoveTaskNode()
{
	bNotifyTick = true;
}

EBTNodeResult::Type UUR_BlockingMoveTaskNode::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	AURAIController* Controller = Cast<AURAIController>(OwnerComp.GetAIOwner());

	AMonster* Monster = Controller->GetPawn<AMonster>();
	
	AURCharacter* Player = Monster->GetWorld()->GetFirstPlayerController()->GetPawn<AURCharacter>();

	if (Monster->IsDeath())
	{
		return EBTNodeResult::Failed;
	}

	// ����ο��� HP�� 0���� ũ�ٸ� �н��Ѵ�.
	if (Monster->GetHPPercent() > 0.2f)
	{
		return EBTNodeResult::Failed;
	}

	Monster->SetIsBlocking(true);

	if (Monster->GetAnimationInstance()->IsAnimMontage(PirateAnimation::BlockHit))
	{
		return EBTNodeResult::Succeeded;
	}

	if (Monster->GetCharacterMovement()->MaxWalkSpeed != 300)
	{
		Monster->GetCharacterMovement()->MaxWalkSpeed = 300.f;
	}

	FVector MonsterPos = Monster->GetActorLocation();
	FVector PlayerPos = Player->GetActorLocation();
	FVector MonsterDir = MonsterPos - PlayerPos;
	MonsterDir = MonsterDir.GetSafeNormal();

	// ������ ���� �ڻ��μ�Ÿ ���� ���Ѵ���
	double BValue = FVector::DotProduct(MonsterDir, -Player->GetActorForwardVector());
	double LValue = FVector::DotProduct(MonsterDir, -Player->GetActorRightVector());
	double RValue = FVector::DotProduct(MonsterDir, Player->GetActorRightVector());

	// ��ũ�ڻ��ΰ� ���� �� ��׸��� �̿��ؼ� ��׸��� ������ ��ȯ�Ͽ�
	// �����ġ�� 
	float BConvert = acosf(static_cast<float>(BValue));
	BConvert = FMath::RadiansToDegrees(BConvert);
	float LConvert = acosf(static_cast<float>(LValue));
	LConvert = FMath::RadiansToDegrees(LConvert);
	float RConvert = acosf(static_cast<float>(RValue));
	RConvert = FMath::RadiansToDegrees(RConvert);

	FVector LeftPos = Player->GetActorLocation() - Player->GetActorRightVector() * 300.f;
	FVector RightPos = Player->GetActorLocation() + Player->GetActorRightVector() * 300.f;
	FVector BackwardPos = Player->GetActorLocation() - Player->GetActorForwardVector() * 300.f;

	if (nullptr != Monster->GetPath())
	{
		if (false == Monster->NoAnimPathMove())
		{
			Monster->ResetPath();
			return EBTNodeResult::InProgress;
		}

		if (100.0f <= Monster->GetLastPathPointToTargetDis(Player->GetActorLocation()))
		{
			Monster->ResetPath();
			return EBTNodeResult::InProgress;
		}
	}

	double Dist = INT_MAX;
	double PrevDist = 0.0;

	// ���� ����� �Ÿ��� Ž���Ѵ�.
	Dist = std::min(Dist, (LeftPos - MonsterPos).Length());

	// ���� �ٸ��ٸ� �������ġ�� ���ŵȰ�
	if (static_cast<int>(PrevDist) != static_cast<int>(Dist))
	{
		m_Type = PathPos_Type::Left;
	}
	PrevDist = Dist;

	Dist = std::min(Dist, (RightPos - MonsterPos).Length());

	if (static_cast<int>(PrevDist) != static_cast<int>(Dist))
	{
		m_Type = PathPos_Type::Right;
	}
	PrevDist = Dist;
	Dist = std::min(Dist, (BackwardPos - MonsterPos).Length());

	if (static_cast<int>(PrevDist) != static_cast<int>(Dist))
	{
		m_Type = PathPos_Type::Backward;
	}

	UNavigationPath* FindPath = nullptr;

	double Value = 0.0;

	FVector MonsterForward = Monster->GetActorForwardVector();
	FVector MonsterUp = FVector(0.0, 0.0, 1.0);
	FVector AVec;

	// ���� ����� ��ġ�� �־��־� Path�� �̾Ƴ���.
	switch (m_Type)
	{
	case PathPos_Type::Left:
	{
		FindPath = Monster->PathFind(LeftPos);
		AVec = (LeftPos - MonsterPos).GetSafeNormal();
		Value = MonsterUp.Dot(FVector::CrossProduct(AVec, MonsterForward));
	}
		break;
	case PathPos_Type::Right:
	{
		FindPath = Monster->PathFind(RightPos);
		AVec = (RightPos - MonsterPos).GetSafeNormal();
		Value = MonsterUp.Dot(FVector::CrossProduct(AVec, MonsterForward));
	}
		break;
	case PathPos_Type::Backward:
	{
		FindPath = Monster->PathFind(BackwardPos);
		AVec = (BackwardPos - MonsterPos).GetSafeNormal();
		Value = MonsterUp.Dot(FVector::CrossProduct(AVec, MonsterForward));
	}
		break;
	}

	if (BConvert < 45.f)
	{
		if (Monster->GetTargetDir(BackwardPos).Size() <= 30.f)
		{
			Monster->ResetPath();
			return EBTNodeResult::Failed;
		}
	}
	else if (RConvert < 45.f)
	{
		if (Monster->GetTargetDir(RightPos).Size() <= 30.f)
		{
			Monster->ResetPath();

			if (Monster->GetIsBlocking())
				Monster->SetIsBlocking(false);

			return EBTNodeResult::Failed;
		}
	}
	else if (LConvert < 45.f)
	{
		if (Monster->GetTargetDir(LeftPos).Size() <= 30.f)
		{
			Monster->ResetPath();

			if (Monster->GetIsBlocking())
				Monster->SetIsBlocking(false);

			return EBTNodeResult::Failed;
		}
	}

	if (nullptr != FindPath && false == FindPath->PathPoints.IsEmpty())
	{
		Monster->SetPath(FindPath, false);
		Monster->NoAnimPathMove();
		Monster->SetTargetLook(Player);

		// ���Ͱ� ���ŷ���� �ƴ϶�� ���ŷ������ �����ߴ� �˷��־����.
		if (!Monster->GetIsBlocking())
			Monster->SetIsBlocking(true);

		if (Value > 0.0)
		{
			Monster->GetAnimationInstance()->ChangeAnimMontage(PirateAnimation::BlockWalkRight);
		}
		else
		{
			Monster->GetAnimationInstance()->ChangeAnimMontage(PirateAnimation::BlockWalkLeft);
		}

		return EBTNodeResult::InProgress;
	}

	

	return EBTNodeResult::InProgress;
}

void UUR_BlockingMoveTaskNode::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	FinishLatentTask(OwnerComp, ExecuteTask(OwnerComp, NodeMemory));
}
