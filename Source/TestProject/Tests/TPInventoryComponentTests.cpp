// Fill out your copyright notice in the Description page of Project Settings.

#if (WITH_DEV_AUTOMATION_TESTS || WITH_PERF_AUTOMATION_TESTS)

#include "Tests/TPInventoryComponentTests.h"
#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Tests/TestUtils.h"
#include "TPTypes.h"
#include "Components/TPInventoryComponent.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FComponentCouldBeCreated, "TestProject.Components.Inventory.ComponentCouldBeCreated",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority);

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FItemScoresShouldBeZeroByDefault, "TestProject.Components.Inventory.ItemScoresShouldBeZeroByDefault",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority);

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNegativeScoreShouldBeAdded, "TestProject.Components.Inventory.NegativeScoreShouldBeAdded",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority);

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPositiveScoreShouldBeAdded, "TestProject.Components.Inventory.PositiveScoreShouldBeAdded",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority);

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FScoreMoreThanLimit, "TestProject.Components.Inventory.ScoreMoreThanLimit",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority);

namespace
{
	class UTPInventoryComponentTestable : public UTPInventoryComponent
	{
	public:
		void SetLimits(const TMap<EInventoryItemType, int32>& Limits)
		{
			InventoryLimits = Limits;
		}
	};
}

bool FComponentCouldBeCreated::RunTest(const FString& Parameters)
{
	const UTPInventoryComponent* InvComp = NewObject<UTPInventoryComponent>();
	if (!TestNotNull("Inventory component exists", InvComp)) return false;

	return true;
}

bool FItemScoresShouldBeZeroByDefault::RunTest(const FString& Parameters)
{
	const UTPInventoryComponent* InvComp = NewObject<UTPInventoryComponent>();
	if (!TestNotNull("Inventory component exists", InvComp)) return false;

	TestTrueExpr(InvComp->GetInventoryAmountByType(EInventoryItemType::CONE) == 0);
	TestTrueExpr(InvComp->GetInventoryAmountByType(EInventoryItemType::CUBE) == 0);
	TestTrueExpr(InvComp->GetInventoryAmountByType(EInventoryItemType::CYLINDER) == 0);
	TestTrueExpr(InvComp->GetInventoryAmountByType(EInventoryItemType::SPHERE) == 0);

	return true;
}

bool FNegativeScoreShouldBeAdded::RunTest(const FString& Parameters)
{
	UTPInventoryComponent* InvComp = NewObject<UTPInventoryComponent>();
	if (!TestNotNull("Inventory component exists", InvComp)) return false;

	TestTrueExpr(!InvComp->TryToAddItem({ EInventoryItemType::CONE, -10 }));
	TestTrueExpr(InvComp->GetInventoryAmountByType(EInventoryItemType::CONE) == 0);

	TestTrueExpr(!InvComp->TryToAddItem({ EInventoryItemType::SPHERE, -10 }));
	TestTrueExpr(InvComp->GetInventoryAmountByType(EInventoryItemType::SPHERE) == 0);

	TestTrueExpr(!InvComp->TryToAddItem({ EInventoryItemType::CYLINDER, -10 }));
	TestTrueExpr(InvComp->GetInventoryAmountByType(EInventoryItemType::CYLINDER) == 0);

	TestTrueExpr(!InvComp->TryToAddItem({ EInventoryItemType::CUBE, -10 }));
	TestTrueExpr(InvComp->GetInventoryAmountByType(EInventoryItemType::CUBE) == 0);

	return true;
}

bool FPositiveScoreShouldBeAdded::RunTest(const FString& Parameters)
{
	UTPInventoryComponentTestable* InvComp = NewObject<UTPInventoryComponentTestable>();
	if (!TestNotNull("Inventory component exists", InvComp)) return false;

	InvComp->SetLimits({{EInventoryItemType::CONE, 300},
						{EInventoryItemType::CUBE, 300},
						{EInventoryItemType::CYLINDER, 300},
						{EInventoryItemType::SPHERE, 300} });

	TestTrueExpr(InvComp->TryToAddItem({ EInventoryItemType::CONE, 10 }));
	TestTrueExpr(InvComp->GetInventoryAmountByType(EInventoryItemType::CONE) == 10);

	TestTrueExpr(InvComp->TryToAddItem({ EInventoryItemType::CUBE, 10 }));
	TestTrueExpr(InvComp->GetInventoryAmountByType(EInventoryItemType::CUBE) == 10);

	TestTrueExpr(InvComp->TryToAddItem({ EInventoryItemType::CYLINDER, 10 }));
	TestTrueExpr(InvComp->GetInventoryAmountByType(EInventoryItemType::CUBE) == 10);

	TestTrueExpr(InvComp->TryToAddItem({ EInventoryItemType::SPHERE, 10 }));
	TestTrueExpr(InvComp->GetInventoryAmountByType(EInventoryItemType::CUBE) == 10);

	return true;
}

bool FScoreMoreThanLimit::RunTest(const FString& Parameters)
{
	UTPInventoryComponentTestable* InvComp = NewObject<UTPInventoryComponentTestable>();
	if (!TestNotNull("Inventory component exists", InvComp)) return false;

	InvComp->SetLimits({ {EInventoryItemType::CONE, 300},
						{EInventoryItemType::CUBE, 300},
						{EInventoryItemType::CYLINDER, 300},
						{EInventoryItemType::SPHERE, 300} });

	TestTrueExpr(InvComp->TryToAddItem({ EInventoryItemType::CONE, 10 }));
	TestTrueExpr(InvComp->GetInventoryAmountByType(EInventoryItemType::CONE) == 10);

	TestTrueExpr(!InvComp->TryToAddItem({ EInventoryItemType::CONE, 300 }));
	TestTrueExpr(InvComp->GetInventoryAmountByType(EInventoryItemType::CONE) == 10);

	return true;
}

#endif