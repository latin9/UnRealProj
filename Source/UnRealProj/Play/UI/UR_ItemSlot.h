// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Global/URStructs.h"
#include "UR_ItemSlot.generated.h"

// �κ��丮 ���Կ��� �������� ���������� �����ϱ����� Ŭ�����̴�
UCLASS()
class UNREALPROJ_API UUR_ItemSlot : public UUserWidget
{
	GENERATED_BODY()

protected:
	// ���������ǰ�� �����ڴ�� �̰��� �̿��Ѵ�.
	void NativeConstruct() override;

private:
	const FURItemData* m_Data;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	class UImage* m_Image;

	class UTextBlock* m_ItemCountText;

	FString m_BlockText;
	
	// �����Ϳ��� ���ε��� ���� c++�̰� �����Ͱ� ���� �������� �� ����.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	FString m_ItemCountString;


public:
	// URealUn_ItemSlot();

	FORCEINLINE const FURItemData* GetItemData()
	{ 
		return m_Data;
	}

	void SetItemData(const FURItemData* _Data);

	UFUNCTION(BlueprintCallable, Category = "Contents", meta = (AllowPrivateAccess = "true"))
	class UImage* GetItemSlotImage()	const
	{
		return m_Image;
	}

	UFUNCTION(BlueprintCallable, Category = "Contents", meta = (AllowPrivateAccess = "true"))
	FName GetIconDisPlayName()	const
	{
		return m_Data->DisplayName;
	}

	UFUNCTION(BlueprintCallable, Category = "Contents", meta = (AllowPrivateAccess = "true"))
	void SetItemIconImage(UImage* _Image)
	{
		m_Image = _Image;
	}


private:
	UFUNCTION(BlueprintCallable, Category = "Contents", meta = (AllowPrivateAccess = "true"))
	void SlotInit();
};
