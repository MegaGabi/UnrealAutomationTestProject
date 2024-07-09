// Fill out your copyright notice in the Description page of Project Settings.

#if (WITH_DEV_AUTOMATION_TESTS || WITH_PERF_AUTOMATION_TESTS)

#include "Tests/SandboxTests.h"
#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Tests/TestUtils.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMathMaxInt, "TestProject.Math.MaxInt",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority);

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMathSqrt, "TestProject.Math.Sqrt",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority);

bool FMathMaxInt::RunTest(const FString& Parameters)
{
	const TArray<TestProject::TestPayload<TInterval<int32>, int32>> TestData
	{
		{ { 13, 25 }, 25 },
		{ {25, 25}, 25 },
		{ {0, 123}, 123 },
		{ {0, 0}, 0 },
		{ {-2345, 0}, 0 },
		{ {-45, -67}, -45 },
		{ {-9, -9}, -9 },
		{ {-78, 34}, 34 }
	};

	for (const auto Data : TestData)
	{
		TestTrueExpr(FMath::Max(Data.TestValue.Min, Data.TestValue.Max) == Data.ExpectedValue);
	}

	return true;
}

bool FMathSqrt::RunTest(const FString& Parameters)
{
	AddInfo("Sqrt function testing");
	TestEqual("Sqrt(4)", FMath::Sqrt(4.0f), 2.0f);
	TestEqual("Sqrt(3)", FMath::Sqrt(3.0f), 1.732f);
	TestEqual("Sqrt(3)", FMath::Sqrt(3.0f), 1.7f, 0.1f);
	TestEqual("Sqrt(3)", FMath::Sqrt(3.0f), 1.73205f, 1.e-5f);
	return true;
}

#endif