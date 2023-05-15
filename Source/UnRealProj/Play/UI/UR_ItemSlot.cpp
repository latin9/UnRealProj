// Fill out your copyright notice in the Description page of Project Settings.


#include "Play/UI/UR_ItemSlot.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UUR_ItemSlot::SlotInit()
{
	m_Image = Cast<UImage>(GetWidgetFromName(FName(TEXT("ItemIconImage"))));

	if (nullptr == m_Image)
	{
		UE_LOG(LogTemp, Error, TEXT("Item Image nullptr!"));
		return;
	}

	m_ItemCountText = Cast<UTextBlock>(GetWidgetFromName(TEXT("ItemIconCount")));

	if (nullptr == m_ItemCountText)
	{
		UE_LOG(LogTemp, Error, TEXT("Item Text Nullptr!"));
		return;
	}


	m_ItemCountString = TEXT("");

	m_Data = nullptr;
}

void UUR_ItemSlot::SetItemData(const FURItemData* _Data)
{
	// ��������� �Һ������� ����
	if (_Data->ItemType == ContentsItemType::Equip)
	{
		m_ItemCountString = TEXT("");
	}
	else if (_Data->ItemType == ContentsItemType::Consumable)
	{
		if (m_ItemCountString == TEXT(""))
		{
			// ó�����°��̶�� 0�� �־��ش�.
			m_ItemCountString = FString::FromInt(1);
		}
		else
		{
			// �װ� �ƴ϶�� ������ ���ڿ��� +1�� ���־���Ѵ�.
			int Count = FCString::Atoi(*m_ItemCountString);

			++Count;

			m_ItemCountString = FString::FromInt(Count);
		}
	}

	// ������ ���̺� �����س��� ������ �ؽ�ó�� �������ش�.
	m_Image->SetBrushFromTexture(_Data->IconTexture);

	// ������ ����
	m_Data = _Data;
}

void UUR_ItemSlot::NativeConstruct()
{
	int a = 0;
}
