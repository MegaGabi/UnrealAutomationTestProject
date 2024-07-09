// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/TPInventoryComponent.h"


UTPInventoryComponent::UTPInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}

bool UTPInventoryComponent::TryToAddItem(const FInventoryData& Data)
{
	if (Data.Score < 0) return false;

	if (!Inventory.Contains(Data.Type))
	{
		Inventory.Add(Data.Type, 0);
	}

	const auto NextScore = Inventory[Data.Type] + Data.Score;
	if (NextScore > InventoryLimits[Data.Type]) return false;

	Inventory[Data.Type] = NextScore;

	return true;
}

int32 UTPInventoryComponent::GetInventoryAmountByType(EInventoryItemType Type) const
{
	return Inventory.Contains(Type) ? Inventory[Type] : 0;
}


// Called when the game starts
void UTPInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	const auto InvEnum = StaticEnum<EInventoryItemType>();
	for (int32 i = 0; i < InvEnum->NumEnums() - 1; ++i)
	{
		const EInventoryItemType EnumElem = static_cast<EInventoryItemType>(i);
		const FString EnumElemName = UEnum::GetValueAsString(EnumElem);
		const bool LimitCheckCond = InventoryLimits.Contains(EnumElem) && InventoryLimits[EnumElem] >= 0 ;
		checkf(LimitCheckCond, TEXT("Limits for %s doesn't exist or less then zero"), *EnumElemName)
	}
}
