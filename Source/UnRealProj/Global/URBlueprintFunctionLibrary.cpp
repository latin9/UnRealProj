// Fill out your copyright notice in the Description page of Project Settings.


#include "URBlueprintFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"

void UURBlueprintFunctionLibrary::OpenLevel(FName OpenLevelName)
{
	// ���� �������� ���带 ���´�
	UWorld* CurrentWorld = GetCurrentPlayWorld();

	// ���ٸ� ����
	if (!CurrentWorld)
		return;

	// ���� ����� �̸��� �־ ������
	UGameplayStatics::OpenLevel(CurrentWorld, OpenLevelName);
}

UWorld* UURBlueprintFunctionLibrary::GetCurrentPlayWorld()
{
	UWorld* PIE = nullptr;
	UWorld* GamePreview = nullptr;
	UWorld* Game = nullptr;

	for (FWorldContext Context : GEngine->GetWorldContexts())
	{
		switch (Context.WorldType)
		{
		case EWorldType::PIE:
			PIE = Context.World();
			break;
		case EWorldType::GamePreview:
			GamePreview = Context.World();
			break;
		case EWorldType::Game:
			Game = Context.World();
			break;
		default:
			break;
		}
	}

	UWorld* CurrentWorld = nullptr;

	if (nullptr != PIE)
	{
		CurrentWorld = PIE;
	}
	else if (nullptr != GamePreview)
	{
		CurrentWorld = GamePreview;
	}
	else if (nullptr != Game)
	{
		CurrentWorld = Game;
	}

	return CurrentWorld;
}

UURGameInstance* UURBlueprintFunctionLibrary::GetURInst()
{
	UURGameInstance* Inst = GetCurrentPlayWorld()->GetGameInstance<UURGameInstance>();

	if (!Inst)
	{
		UE_LOG(LogTemp, Error, TEXT("URGameInst Null!"));
		return nullptr;
	}

	return Inst;
}

void UURBlueprintFunctionLibrary::DebugSwitch()
{
	// �÷������� ���尡 ���ٸ� ����
	if (nullptr == GetCurrentPlayWorld())
	{
		return;
	}
	UURGameInstance* Inst = GetCurrentPlayWorld()->GetGameInstance<UURGameInstance>();

	if (!Inst)
	{
		UE_LOG(LogTemp, Error, TEXT("URGameInst Null!"));
		return;
	}

	Inst->DebugSwitch();
}

bool UURBlueprintFunctionLibrary::IsDebug()
{
	UURGameInstance* Inst = GetCurrentPlayWorld()->GetGameInstance<UURGameInstance>();

	if (!Inst)
	{
		UE_LOG(LogTemp, Error, TEXT("URGameInst Null!"));
		return false;
	}

	return Inst->IsDebug();
}