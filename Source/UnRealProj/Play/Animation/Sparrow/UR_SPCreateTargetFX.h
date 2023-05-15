// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "UR_SPCreateTargetFX.generated.h"

/**
 * 
 */
UCLASS()
class UNREALPROJ_API UUR_SPCreateTargetFX : public UAnimNotifyState
{
	GENERATED_BODY()
		
private:
	class AUR_SparrowSubBoss* m_Boss;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UserContents", meta = (AllowPrivateAccess = "true"))
		FName m_SocketName;

	// z���� ��ġ�� �ٸ��� ���� z���� Add�� �� �ִ� ������ ����
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UserContents", meta = (AllowPrivateAccess = "true"))
		float m_AddLocationZ;

protected:
	void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
