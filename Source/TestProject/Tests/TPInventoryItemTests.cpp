// Fill out your copyright notice in the Description page of Project Settings.

#if (WITH_DEV_AUTOMATION_TESTS || WITH_PERF_AUTOMATION_TESTS)

#include "Tests/TPInventoryItemTests.h"
#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Tests/TestUtils.h"
#include "TPTypes.h"
#include "Items/TPInventoryItem.h"
#include "Engine/World.h"
#include "Engine/Blueprint.h"
#include "Components/SphereComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Misc/OutputDeviceNull.h"
#include "Kismet/GameplayStatics.h"
#include "TestProject/TestProjectCharacter.h"
#include "TestProject/Components/TPInventoryComponent.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCppActorCannotBeCreated, "TestProject.Items.Inventory.CppActorCannotBeCreated",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority);

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBlueprintShouldBeSetupCorrectly, "TestProject.Items.Inventory.BlueprintShouldBeSetupCorrectly",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority);

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryDataShouldBeSetupCorrectly, "TestProject.Items.Inventory.InventoryDataShouldBeSetupCorrectly",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority);

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryCanBePickedUp, "TestProject.Items.Inventory.InventoryCanBePickedUp",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority);

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEveryInventoryItemMeshExists, "TestProject.Items.Inventory.EveryInventoryItemMeshExists",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority);


namespace
{
	const FString InventoryItemBPName = "/Script/Engine.Blueprint'/Game/Inventory/BP_TPInventoryItem.BP_TPInventoryItem'";
	const FString InventoryItemBPTestName = "/Script/Engine.Blueprint'/Game/Tests/BP_TestTpInventoryItem.BP_TestTpInventoryItem'";
}

using namespace TestProject;

bool FCppActorCannotBeCreated::RunTest(const FString& Parameters)
{
	LevelScope("/Game/Tests/EmptyTestLevel");

	UWorld* World = GetTestGameWorld();
	if (!TestNotNull(TEXT("World exists"), World)) return false;

	AddExpectedError("SpawnActor failed because class TPInventoryItem is abstract", EAutomationExpectedMessageFlags::Contains);
	const ATPInventoryItem* InvItem = World->SpawnActor<ATPInventoryItem>(ATPInventoryItem::StaticClass(), FTransform::Identity);
	if (!TestNull(TEXT("Inventory item exists"), InvItem)) return false;

	return true;
}

bool FBlueprintShouldBeSetupCorrectly::RunTest(const FString& Parameters)
{
	LevelScope("/Game/Tests/EmptyTestLevel");

	UWorld* World = GetTestGameWorld();
	if (!TestNotNull(TEXT("World exists"), World)) return false;

	const UBlueprint* Blueprint = LoadObject<UBlueprint>(nullptr, *InventoryItemBPName);
	if (!TestNotNull(TEXT("Inventory item exists"), Blueprint)) return false;

	const ATPInventoryItem* InvItem = World->SpawnActor<ATPInventoryItem>(Blueprint->GeneratedClass, FTransform::Identity);
	if (!TestNotNull(TEXT("Inventory item exists"), InvItem)) return false;

	const auto CollisionComp = InvItem->FindComponentByClass<USphereComponent>();
	if (!TestNotNull(TEXT("Sphere Collision Component exists"), CollisionComp)) return false;

	TestTrueExpr(CollisionComp->GetScaledSphereRadius() >= 30.0f);
	TestTrueExpr(CollisionComp->GetCollisionEnabled() == ECollisionEnabled::QueryOnly);
	TestTrueExpr(CollisionComp->GetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic) == ECollisionResponse::ECR_Overlap);
	TestTrueExpr(CollisionComp->GetGenerateOverlapEvents());
	TestTrueExpr(InvItem->GetRootComponent() == CollisionComp);

	const auto TextRenderComp = InvItem->FindComponentByClass<UTextRenderComponent>();
	if (!TestNotNull(TEXT("Text renderer exists"), TextRenderComp)) return false;

	const auto StaticMeshComp = InvItem->FindComponentByClass<UStaticMeshComponent>();
	if (!TestNotNull(TEXT("Static mesh exists"), StaticMeshComp)) return false;

	TestTrueExpr(StaticMeshComp->GetCollisionEnabled() == ECollisionEnabled::NoCollision);

	return true;
}

bool FInventoryDataShouldBeSetupCorrectly::RunTest(const FString& Parameters)
{
	LevelScope("/Game/Tests/EmptyTestLevel");

	UWorld* World = GetTestGameWorld();
	if (!TestNotNull(TEXT("World exists"), World)) return false;

	const UBlueprint* Blueprint = LoadObject<UBlueprint>(nullptr, *InventoryItemBPTestName);
	if (!TestNotNull(TEXT("Inventory item exists"), Blueprint)) return false;

	ATPInventoryItem* InvItem = World->SpawnActor<ATPInventoryItem>(Blueprint->GeneratedClass, FTransform::Identity);
	if (!TestNotNull(TEXT("Inventory item exists"), InvItem)) return false;

	const FInventoryData InvData{ EInventoryItemType::CYLINDER, 13 };
	const FLinearColor Color = FLinearColor::Yellow;
	CallFuncByNameWithParams(InvItem, "SetInventoryData",
		{
			InvData.ToString(),
			Color.ToString()
		});

	const auto TextRenderComp = InvItem->FindComponentByClass<UTextRenderComponent>();
	if (!TestNotNull(TEXT("Text renderer exists"), TextRenderComp)) return false;

	TestTrueExpr(TextRenderComp->Text.ToString().Equals(FString::FromInt(InvData.Score)));
	TestTrueExpr(TextRenderComp->TextRenderColor == Color.ToFColor(true));

	const auto StaticMeshComp = InvItem->FindComponentByClass<UStaticMeshComponent>();
	if (!TestNotNull(TEXT("Static mesh exists"), StaticMeshComp)) return false;

	const auto Material = StaticMeshComp->GetMaterial(0);
	if (!TestNotNull(TEXT("Material exists"), Material)) return false;

	FLinearColor MaterialColor;
	Material->GetVectorParameterValue(FHashedMaterialParameterInfo{"Color"}, MaterialColor);
	TestTrueExpr(MaterialColor == Color);

	return true;
}

bool FInventoryCanBePickedUp::RunTest(const FString& Parameters)
{
	LevelScope("/Game/Tests/EmptyTestLevel");

	UWorld* World = GetTestGameWorld();
	if (!TestNotNull(TEXT("World exists"), World)) return false;

	const UBlueprint* Blueprint = LoadObject<UBlueprint>(nullptr, *InventoryItemBPTestName);
	if (!TestNotNull(TEXT("Inventory item exists"), Blueprint)) return false;

	ATPInventoryItem* InvItem = World->SpawnActor<ATPInventoryItem>(Blueprint->GeneratedClass, FTransform::Identity);
	if (!TestNotNull(TEXT("Inventory item exists"), InvItem)) return false;

	const FInventoryData InvData{ EInventoryItemType::CYLINDER, 13 };
	const FLinearColor Color = FLinearColor::Yellow;
	CallFuncByNameWithParams(InvItem, "SetInventoryData",
		{
			InvData.ToString(),
			Color.ToString()
		});

	TArray<AActor*> Pawns;
	UGameplayStatics::GetAllActorsOfClass(World, ATestProjectCharacter::StaticClass(), Pawns);

	if (!TestTrueExpr(Pawns.Num() == 1)) return false;

	const auto Pawn = Cast<ATestProjectCharacter>(Pawns[0]);
	if (!TestNotNull(TEXT("Character exists"), InvItem)) return false;

	const auto InvComp = Pawn->FindComponentByClass<UTPInventoryComponent>();
	if (!TestNotNull(TEXT("Inventory component exists"), InvItem)) return false;

	TestTrueExpr(InvComp->GetInventoryAmountByType(InvData.Type) == 0);

	Pawn->SetActorLocation(InvItem->GetActorLocation());

	TestTrueExpr(InvComp->GetInventoryAmountByType(InvData.Type) == InvData.Score);
	TestTrueExpr(!IsValid(InvItem));

	TArray<AActor*> InvItems;
	UGameplayStatics::GetAllActorsOfClass(World, ATPInventoryItem::StaticClass(), InvItems);
	TestTrueExpr(InvItems.IsEmpty());

	return true;
}

bool FEveryInventoryItemMeshExists::RunTest(const FString& Parameters)
{
	LevelScope("/Game/Tests/EmptyTestLevel");

	UWorld* World = GetTestGameWorld();
	if (!TestNotNull(TEXT("World exists"), World)) return false;

	const auto SpawnBP = [&](EInventoryItemType Type, FVector Location)
		{
			const UBlueprint* Blueprint = LoadObject<UBlueprint>(nullptr, *InventoryItemBPTestName);
			if (!TestNotNull(TEXT("Inventory item exists"), Blueprint)) return false;

			const FTransform InitialTransform{ Location };
			ATPInventoryItem* InvItem = World->SpawnActor<ATPInventoryItem>(Blueprint->GeneratedClass, InitialTransform);
			if (!TestNotNull(TEXT("Inventory item exists"), InvItem)) return false;

			const FInventoryData InvData{ Type, 13 };
			const FLinearColor Color = FLinearColor::Yellow;
			CallFuncByNameWithParams(InvItem, "SetInventoryData",
				{
					InvData.ToString(),
					Color.ToString()
				});

			const auto StaticMeshComp = InvItem->FindComponentByClass<UStaticMeshComponent>();
			if (!TestNotNull(TEXT("Static mesh exists"), StaticMeshComp)) return false;

			const FString MeshMsg = FString::Printf(TEXT("Static mesh for %s exists"), *UEnum::GetValueAsString(Type));
			TestNotNull(*MeshMsg, StaticMeshComp->GetStaticMesh().Get());

			return true;
		};
	if(!SpawnBP(EInventoryItemType::CYLINDER, FVector(100.0f))) return false;
	if (!SpawnBP(EInventoryItemType::CONE, FVector(200.0f))) return false;
	if (!SpawnBP(EInventoryItemType::CUBE, FVector(300.0f))) return false;
	if (!SpawnBP(EInventoryItemType::SPHERE, FVector(400.0f))) return false;


	return true;
}

#endif