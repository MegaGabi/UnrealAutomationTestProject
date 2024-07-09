// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TestProject/TPTypes.h"
#include "TPInventoryComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TESTPROJECT_API UTPInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UTPInventoryComponent();

	bool TryToAddItem(const FInventoryData& Data);
	
	UFUNCTION(BlueprintCallable)
	int32 GetInventoryAmountByType(EInventoryItemType Type) const;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
		
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TMap<EInventoryItemType, int32> InventoryLimits;
private:
	TMap<EInventoryItemType, int32> Inventory;
};
