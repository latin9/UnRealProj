// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PlayerAnimInstance.generated.h"

UENUM(BlueprintType)
enum class PlayerAnimation : uint8
{
	// 1100 
	Idle UMETA(DisplayName = "���ִ� ���"),
	Left UMETA(DisplayName = "����"),      // 0001
	Right  UMETA(DisplayName = "������"),  // 0010
	Forward UMETA(DisplayName = "��"), // 0100
	BackWard UMETA(DisplayName = "��"),  // 1000
};

/**
 * 
 */
UCLASS()
class UNREALPROJ_API UPlayerAnimInstance : public UAnimInstance
{
	GENERATED_BODY()


	
};
