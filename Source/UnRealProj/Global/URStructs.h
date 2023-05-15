// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/DataTable.h"
#include "../Play/Actor/URCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "URStructs.generated.h"

/**
 * 
 */
UCLASS()
class UNREALPROJ_API UURStructs : public UObject
{
	GENERATED_BODY()
	
};


// �����͸� ����ȭ �ؼ� �޸𸮸� �Ƴ���� �ǵ�
// �����͸� ��õ������ ����Ҷ� �ӷ��� ������ ���ݴϴ�.
USTRUCT(Atomic, BlueprintType)
struct FURMonsterDataInfo : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

	// double�� ������� ǥ�������ϱ⶧���� ������ int�� ��� �� �ϰ� double�ڷ����� ����Ѵ�.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double HP;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double Def;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double MinAttack;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double MaxAttack;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double Speed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double FindRange;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double AttRange;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double SkillRange;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<DefaultAnimation, class UAnimMontage*> Animations;
};

USTRUCT(Atomic, BlueprintType)
struct FURPlayerDataInfo : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

	// double�� ������� ǥ�������ϱ⶧���� ������ int�� ��� �� �ϰ� double�ڷ����� ����Ѵ�.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double HP;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double MaxHP;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double MP;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double MaxMP;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Stamina;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxStamina;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Def;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Speed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double MinAttack;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double MaxAttack;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AttackRange;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float QSkillCollTime;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ESkillCollTime;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RSkillCollTime;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<DefaultAnimation, class UAnimMontage*> Animations;
};

USTRUCT(Atomic, BlueprintType)
struct FURObjectTable : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

		// double�� ������� ǥ�������ϱ⶧���� ������ int�� ��� �� �ϰ� double�ڷ����� ����Ѵ�.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AActor> ObjectSubClass;
};

USTRUCT(Atomic, BlueprintType)
struct FURItemData : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName DisplayName;
	// �������� ������Ÿ�� ����
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ContentsItemType	ItemType;

	// �κ��丮 �����ܿ� ���� �ؽ�ó
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* IconTexture;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh* DropMesh;

	// ü��
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int HP;

	// ����
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int MP;

	FURItemData() :
		DisplayName(""),
		ItemType(ContentsItemType::Default),
		IconTexture(nullptr),
		DropMesh(nullptr),
		HP(0),
		MP(0)
	{

	}
};

USTRUCT()
struct FURWarriorCommand
{
	GENERATED_USTRUCT_BODY()

	bool IsWClicked;
	bool IsWDoubleClicked;
	float WResetTime;
	float WDoubleResetTime;
};

USTRUCT()
struct FURWarriorSound
{
	GENERATED_USTRUCT_BODY()

	class USoundBase* NoMPVoice;
	class USoundBase* CoolTimeVoice;
	class USoundBase* UltimateDash;
};