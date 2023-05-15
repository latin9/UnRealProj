// Fill out your copyright notice in the Description page of Project Settings.


#include "Play/AI/Sparrow/UR_SparrowMoveTaskNode.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "../../Controller/URAIController.h"
#include "../../Actor/Boss/UR_SparrowSubBoss.h"
#include "Global/URStructs.h"

UUR_SparrowMoveTaskNode::UUR_SparrowMoveTaskNode()
{
	// TickTask�Լ��� ���۽�ų�� �������� ����
	bNotifyTick = true;
}

EBTNodeResult::Type UUR_SparrowMoveTaskNode::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	if (!m_Controller)
		m_Controller = Cast<AURAIController>(OwnerComp.GetAIOwner());

	if (!m_Boss)
		m_Boss = m_Controller->GetPawn<AUR_SparrowSubBoss>();

	if (AnimMontageJudge())
	{
		return EBTNodeResult::Failed;
	}

	const FURMonsterDataInfo* MonsterInfo = m_Boss->GetSparrowData();

	int AttackNumb = OwnerComp.GetBlackboardComponent()->GetValueAsInt(FName("RandAttackNumb"));

	// 3������ ���� ���ݵ��� ���� �ָ� �־ ���۵Ǵ� ��ų���̱� ������
	// �ش� ��ȣ�� ���̶�� MoveTaskNode�� �����ϵ��� �ؾ���
	if (AttackNumb > 4)
	{
		UObject* Target = OwnerComp.GetBlackboardComponent()->GetValueAsObject("TargetActor");

		if (!Target)
		{
			return EBTNodeResult::Succeeded;
		}

		AActor* TargetActor = Cast<AActor>(Target);

		FVector Dir = TargetActor->GetActorLocation() - m_Boss->GetActorLocation();

		if (m_Boss->GetTargetDir(TargetActor).Size() < MonsterInfo->SkillRange)
		{
			//Monster->GetAnimationInstance()->ChangeAnimMontage(DefaultAnimation::Attack);
			return EBTNodeResult::Failed;
		}


		FVector ForwardDir = m_Boss->GetActorForwardVector();    // forward ����
		FVector TargetDir = (TargetActor->GetActorLocation() - m_Boss->GetActorLocation());                   // Target ����
		TargetDir = TargetDir.GetSafeNormal();
		FVector UpDir = ForwardDir.Cross(m_Boss->GetActorRightVector());	// Up ����
		float dot = FVector::DotProduct(ForwardDir, TargetDir);
		float angle = FMath::RadiansToDegrees(FMath::Acos(dot));
		FVector crossPrdt = FVector::CrossProduct(ForwardDir, TargetDir);

		float delta = GetWorld()->DeltaTimeSeconds;
		float yawDelta = 0;
		if (crossPrdt.Z < -0.08f)
		{
			yawDelta = angle * delta * -1;
		}
		else if (crossPrdt.Z > 0.08f)
		{
			yawDelta = angle * delta;
		}
		yawDelta *= 10.f;
		FRotator deltaRotation = FRotator(0, yawDelta, 0);   //Yaw

		// ������ Ŭ ���� ������ ȸ���� �ϰԵȴ�.
		m_Boss->AddActorWorldRotation(deltaRotation);

		m_Boss->SetTargetMovementInput(TargetActor);
		m_Boss->GetAnimationInstance()->ChangeAnimMontage(DefaultAnimation::Forward);


		return EBTNodeResult::Type::InProgress;
	}

	UObject* Target = OwnerComp.GetBlackboardComponent()->GetValueAsObject("TargetActor");

	if (!Target)
	{
		return EBTNodeResult::Succeeded;
	}

	AActor* TargetActor = Cast<AActor>(Target);

	FVector Dir = TargetActor->GetActorLocation() - m_Boss->GetActorLocation();

	if (m_Boss->GetTargetDir(TargetActor).Size() < MonsterInfo->AttRange)
	{
		//Monster->GetAnimationInstance()->ChangeAnimMontage(DefaultAnimation::Attack);
		return EBTNodeResult::Failed;
	}

	FVector ForwardDir = m_Boss->GetActorForwardVector();    // forward ����
	FVector TargetDir = (TargetActor->GetActorLocation() - m_Boss->GetActorLocation());                   // Target ����
	TargetDir = TargetDir.GetSafeNormal();
	FVector UpDir = ForwardDir.Cross(m_Boss->GetActorRightVector());	// Up ����
	float dot = FVector::DotProduct(ForwardDir, TargetDir);
	float angle = FMath::RadiansToDegrees(FMath::Acos(dot));
	FVector crossPrdt = FVector::CrossProduct(ForwardDir, TargetDir);

	float delta = GetWorld()->DeltaTimeSeconds;
	float yawDelta = 0;
	if (crossPrdt.Z < -0.08f)
	{
		yawDelta = angle * delta * -1;
	}
	else if (crossPrdt.Z > 0.08f)
	{
		yawDelta = angle * delta;
	}
	yawDelta *= 10.f;
	FRotator deltaRotation = FRotator(0, yawDelta, 0);   //Yaw

	// ������ Ŭ ���� ������ ȸ���� �ϰԵȴ�.
	m_Boss->AddActorWorldRotation(deltaRotation);

	//m_Boss->SetTargetLook(TargetActor);
	m_Boss->SetTargetMovementInput(TargetActor);
	m_Boss->GetAnimationInstance()->ChangeAnimMontage(DefaultAnimation::Forward);


	return EBTNodeResult::Type::InProgress;
}

void UUR_SparrowMoveTaskNode::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	FinishLatentTask(OwnerComp, ExecuteTask(OwnerComp, NodeMemory));
}

bool UUR_SparrowMoveTaskNode::AnimMontageJudge()
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
