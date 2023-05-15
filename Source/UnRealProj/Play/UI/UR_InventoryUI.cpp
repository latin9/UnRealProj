// Fill out your copyright notice in the Description page of Project Settings.


#include "Play/UI/UR_InventoryUI.h"
#include "Global/URGameInstance.h"
#include "Global/ConstantValues.h"
#include "UR_InvenSlotData.h"
#include "Components/TileView.h"
#include "UR_ItemSlot.h"

void UUR_InventoryUI::InventroyInit(UTileView* _TileView, int32 _Count)
{
	if (!_TileView)
	{
		return;
	}

	UURGameInstance* Inst = GetWorld()->GetGameInstance<UURGameInstance>();

	if (!Inst && 
		!Inst->IsValidLowLevel())
	{
		return;
	}

	const FURItemData* ItemData = Inst->GetItemData(ItemNames::NoneItem);

	for (size_t i = 0; i < _Count; ++i)
	{
		UUR_InvenSlotData* InvenSlot = NewObject<UUR_InvenSlotData>();
		InvenSlot->ItemData = ItemData;
		_TileView->AddItem(InvenSlot);

		//m_ItemSlot.Add(Cast<UUR_ItemSlot>(_TileView->GetDisplayedEntryWidgets()));
	}

	m_InitItemCount = _Count;
	// ���� �ν��Ͻ��� �κ��丮�� ����ֵ��� ����
	// �ٸ��������� �κ��丮�� �� ������ �� ����.
	Inst->SetInven(this);
}

int UUR_InventoryUI::AddInvenItemEvent(UObject* _StartData, UUserWidget* _Slot)
{
	UUR_InvenSlotData* Data = Cast<UUR_InvenSlotData>(_StartData);

	if (nullptr == Data)
	{
		return 1;
	}

	UUR_ItemSlot* Widget = Cast<UUR_ItemSlot>(_Slot);

	if (nullptr == Widget)
	{
		return 1;
	}

	m_ItemSlot.Add(Widget);

	return --m_InitItemCount;
}

bool UUR_InventoryUI::IsFull(const FURItemData* _ItemData)
{
	if (!_ItemData)
	{
		return false;
	}

	// ���� �Һ����̶� ó������� �ٸ���.
	if (_ItemData->ItemType == ContentsItemType::Equip)
	{
		for (size_t i = 0; i < m_ItemSlot.Num(); ++i)
		{
			// �� �Ѱ��� �κ������� ���̶�� �������� �ʴٴ� �ǹ��̴�.
			if (!m_ItemSlot[i]->GetItemData())
			{
				return false;
			}
		}
	}
	else if (_ItemData->ItemType == ContentsItemType::Consumable)
	{
		for (size_t i = 0; i < m_ItemSlot.Num(); ++i)
		{
			// �Һ�������� ��� �������� ������ ���� Ÿ���� �����Ͱ� �ִٸ� �� ���� �ʾҴٴ� �ǹ�
			if (_ItemData == m_ItemSlot[i]->GetItemData())
			{
				return false;
			}

			// �� �Ѱ��� �κ������� ���̶�� �������� �ʴٴ� �ǹ��̴�.
			if (!m_ItemSlot[i]->GetItemData())
			{
				return false;
			}
		}
	}

	// �װ� ���� �ƴ϶�� �� ���ִٴ� �ǹ��̴�.
	return true;
}

void UUR_InventoryUI::AddItem(const FURItemData* _ItemData)
{
	if (!_ItemData)
	{
		return;
	}

	if (_ItemData->ItemType == ContentsItemType::Equip)
	{
		for (size_t i = 0; i < m_ItemSlot.Num(); ++i)
		{
			// ���� �ƴ϶�� �ش� �ε��� ���Կ� �������� �ִٴ� �ǹ��̹Ƿ� ���� �ε����� �Ѿ��.
			if (m_ItemSlot[i]->GetItemData())
			{
				continue;
			}
			// �װ� �ƴ϶�� ���ٴ� �ǹ��̹Ƿ� �ش� �ε��� ���Կ� �������� �־��ְ� ���̻� ó���� �ʿ䰡 �����Ƿ� ����
			m_ItemSlot[i]->SetItemData(_ItemData);
			return;
		}
	}
	else if (_ItemData->ItemType == ContentsItemType::Consumable)
	{
		for (size_t i = 0; i < m_ItemSlot.Num(); ++i)
		{
			if (_ItemData == m_ItemSlot[i]->GetItemData())
			{
				m_ItemSlot[i]->SetItemData(_ItemData);
				return;
			}

			else if (m_ItemSlot[i]->GetItemData())
			{
				continue;
			}

			m_ItemSlot[i]->SetItemData(_ItemData);
			return;
		}
	}
}
