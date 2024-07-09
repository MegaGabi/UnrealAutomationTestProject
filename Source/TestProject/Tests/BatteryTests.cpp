// Fill out your copyright notice in the Description page of Project Settings.

#if (WITH_DEV_AUTOMATION_TESTS || WITH_PERF_AUTOMATION_TESTS)

#include "Tests/BatteryTests.h"
#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Tests/TestUtils.h"
#include "Items/Battery.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBatteryTests, "TestProject.Items.Battery",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority);

bool FBatteryTests::RunTest(const FString& Parameters)
{
	using namespace TestProject;

	AddInfo("Battery with default ctor");
	const Battery BatteryDefault;
	TestTrueExpr(FMath::IsNearlyEqual(BatteryDefault.GetPercent(), 1.0f));
	TestTrueExpr(BatteryDefault.GetColor() == FColor::Green);
	TestTrueExpr(BatteryDefault.ToString().Equals("100%"));

	AddInfo("Battery with custom ctor");
	const auto BatteryTestFunction =
		[this](float Percent, const FColor& Color, const FString& PercentString)
		{
			const Battery BatteryCustom{ Percent };
			TestTrueExpr(FMath::IsNearlyEqual(BatteryCustom.GetPercent(), FMath::Clamp(Percent, 0.0f, 1.0f)));
			TestTrueExpr(BatteryCustom.GetColor() == Color);
			TestTrueExpr(BatteryCustom.ToString().Equals(PercentString));
		};
	BatteryTestFunction(1.0f, FColor::Green, "100%");
	BatteryTestFunction(0.46f, FColor::Yellow, "46%");
	BatteryTestFunction(0.16f, FColor::Red, "16%");
	BatteryTestFunction(0.f, FColor::Red, "0%");

	BatteryTestFunction(3000.16f, FColor::Green, "100%");
	BatteryTestFunction(-3000.16f, FColor::Red, "0%");

	AddInfo("Battery charge/discharge");
	Battery BatteryObject{ 0.6f };
	TestTrueExpr(FMath::IsNearlyEqual(BatteryObject.GetPercent(), 0.6f));
	BatteryObject.Discharge();
	TestTrueExpr(FMath::IsNearlyEqual(BatteryObject.GetPercent(), 0.5f));
	BatteryObject.Charge();
	TestTrueExpr(FMath::IsNearlyEqual(BatteryObject.GetPercent(), 0.6f));

	AddInfo("Battery charge/discharge corner cases");
	for (int32 i = 0; i < 100; ++i)
	{
		BatteryObject.Discharge();
	}
	TestTrueExpr(FMath::IsNearlyEqual(BatteryObject.GetPercent(), 0.f));
	for (int32 i = 0; i < 100; ++i)
	{
		BatteryObject.Charge();
	}
	TestTrueExpr(FMath::IsNearlyEqual(BatteryObject.GetPercent(), 1.0f));

	AddInfo("Battery comparison");
	Battery BatteryLow{ .3f };
	Battery BatteryHigh{ .9f };

	TestTrueExpr(BatteryHigh >= BatteryLow);

	return true;
}

#endif

