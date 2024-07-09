// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TestProject/TPTypes.h"
#include "TPInventoryItem.generated.h"

class USphereComponent;

UCLASS(Abstract)
class TESTPROJECT_API ATPInventoryItem : public AActor
{
	GENERATED_BODY()
	
public:	
	ATPInventoryItem();
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

protected:
	UPROPERTY(VisibleAnywhere)
	USphereComponent* CollisionComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FInventoryData InventoryData;
};
