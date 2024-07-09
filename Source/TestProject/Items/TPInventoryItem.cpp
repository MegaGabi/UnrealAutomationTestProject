// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/TPInventoryItem.h"
#include "GameFramework/Pawn.h"
#include "Components/SphereComponent.h"
#include "TestProject/Components/TPInventoryComponent.h"

// Sets default values
ATPInventoryItem::ATPInventoryItem()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>("SphereComponent");
	check(CollisionComponent);
	CollisionComponent->InitSphereRadius(30.0f);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	CollisionComponent->SetGenerateOverlapEvents(true);
	SetRootComponent(CollisionComponent);
}

void ATPInventoryItem::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if (const auto Pawn = Cast<APawn>(OtherActor))
	{
		if (const auto InvComp = Pawn->FindComponentByClass<UTPInventoryComponent>())
		{
			if (InvComp->TryToAddItem(InventoryData))
			{
				Destroy();
			}
		}
	}
}
