// Fill out your copyright notice in the Description page of Project Settings.


#include "Play/AI/Sparrow/UR_SparrowRainAttack.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "../../Actor/Player/WarriorCharacter.h"
#include "../../Controller/URAIController.h"
#include "../../Actor/Boss/UR_SparrowSubBoss.h"

UUR_SparrowRainAttack::UUR_SparrowRainAttack()
{
	bNotifyTick = true;
}

EBTNodeResult::Type UUR_SparrowRainAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	if (!m_Controller)
		m_Controller = Cast<AURAIController>(OwnerComp.GetAIOwner());

	if (!m_Boss)
		m_Boss = m_Controller->GetPawn<AUR_SparrowSubBoss>();

	if (!m_Player)
		m_Player = m_Boss->GetWorld()->GetFirstPlayerController()->GetPawn<AWarriorCharacter>();


	const FURMonsterDataInfo* KhaimeraInfo = m_Boss->GetSparrowData();

	// ���� ���° ���������� �Ǵ����ش�.
	int AttackNumb = OwnerComp.GetBlackboardComponent()->GetValueAsInt(FName("RandAttackNumb"));

	/*
	* 5�� : Rain Charging Shoot = 1�� �� ��Ƽ� �������� ȭ���� ������ (��Į�� ��ġ�� ��Ÿ���� �ش� ��ġ�� ���߿��� ȭ���� �������� ����)
	* 6�� : Rain Burst Shoot = 3�� ���޾Ƽ� ���� ������ �� (��Į�� ��ġ�� ��Ÿ���� �ش� ��ġ�� ���߿��� ȭ���� �������� ����)
	*/

	// 4�����ݺ��� ���� �����̶�°��� ���� �ڼ����� �Ѿ���Ѵٴ� �ǹ��̴�.
	if (AttackNumb < 5 || AttackNumb > 6)
	{
		return EBTNodeResult::Succeeded;
	}

	if (AnimMontageJudge())
	{
		return EBTNodeResult::Succeeded;
	}

	UObject* Target = OwnerComp.GetBlackboardComponent()->GetValueAsObject(FName("TargetActor"));

	if (nullptr == Target)
	{
		return EBTNodeResult::Failed;
	}

	AActor* TargetActor = Cast<AActor>(Target);

	FVector ForwardDir = m_Boss->GetActorForwardVector();    // forward ����
	FVector TargetDir = (TargetActor->GetActorLocation() - m_Boss->GetActorLocation());                   // Target ����
	TargetDir = TargetDir.GetSafeNormal();
	FVector UpDir = ForwardDir.Cross(m_Boss->GetActorRightVector());	// Up ����


	float Dot = FVector::DotProduct(ForwardDir, TargetDir);
	float Angle = FMath::RadiansToDegrees(FMath::Acos(Dot));
	FVector CrossPrdt = FVector::CrossProduct(ForwardDir, TargetDir);

	float Delta = GetWorld()->DeltaTimeSeconds;
	float YawDelta = 0;
	if (CrossPrdt.Z < 0.f)
	{
		YawDelta = Angle * Delta * -1;
	}
	else if (CrossPrdt.Z > 0.08f)
	{
		YawDelta = Angle * Delta;
	}

	YawDelta *= 10.f;
	FRotator DeltaRotation = FRotator(0, YawDelta, 0);   //Yaw

	// ������ Ŭ ���� ������ ȸ���� �ϰԵȴ�.
	m_Boss->AddActorWorldRotation(DeltaRotation);

	if (!m_Enable)
	{
		m_Enable = true;
		m_Boss->AttackOn();
		m_Boss->GetAnimationInstance()->ChangeAnimMontage(SparrowBossAnimation::RainShootStart);
	}

	if (m_Boss->GetAnimationInstance()->IsAnimMontage(SparrowBossAnimation::RainShootAndEnd) ||
		m_Boss->GetAnimationInstance()->IsAnimMontage(SparrowBossAnimation::RainShootBurst) ||
		m_Boss->GetAnimationInstance()->IsAnimMontage(SparrowBossAnimation::RainShootHold) ||
		m_Boss->GetAnimationInstance()->IsAnimMontage(SparrowBossAnimation::RainShootStart))
	{
		return EBTNodeResult::InProgress;
	}

	m_Enable = false;
	m_Boss->AttackOff();

	return EBTNodeResult::Succeeded;
}

void UUR_SparrowRainAttack::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	FinishLatentTask(OwnerComp, ExecuteTask(OwnerComp, NodeMemory));
}

bool UUR_SparrowRainAttack::AnimMontageJudge()
{
	if (m_Boss->GetAnimationInstance()->IsAnimMontage(DefaultAnimation::ForwardHit) ||
		m_Boss->GetAnimationInstance()->IsAnimMontage(DefaultAnimation::BackwardHit) ||
		m_Boss->GetAnimationInstance()->IsAnimMontage(DefaultAnimation::LeftHit) ||
		m_Boss->GetAnimationInstance()->IsAnimMontage(DefaultAnimation::RightHit) ||
		m_Boss->GetAnimationInstance()->IsAnimMontage(DefaultAnimation::HitAirLoop) ||
		m_Boss->GetAnimationInstance()->IsAnimMontage(DefaultAnimation::KnockDown) ||
		m_Boss->GetAnimationInstance()->IsAnimMontage(DefaultAnimation::GetUp))
	{
		return true;
	}

	return false;
}
