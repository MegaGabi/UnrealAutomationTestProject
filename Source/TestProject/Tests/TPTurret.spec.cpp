#if WITH_AUTOMATION_TESTS

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "TestProject/Tests/TestUtils.h"
#include "Weapon/TPTurret.h"
#include "Weapon/TPProjectile.h"

BEGIN_DEFINE_SPEC(FTurret, "TestProject.Turret",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority)
	UWorld* World;
	ATPTurret* Turret;
END_DEFINE_SPEC(FTurret)

using namespace TestProject;

namespace
{
	const FString MapName = "/Game/Tests/EmptyTestLevel";
	const FString TurretBPName = "/Script/Engine.Blueprint'/Game/Weapon/BP_TPTurret.BP_TPTurret'";
	const FString TurretBPTestName = "/Script/Engine.Blueprint'/Game/Tests/BP_TestTPTurret.BP_TestTPTurret'";

	void SpecCloseLevel(UWorld* World)
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			PC->ConsoleCommand(TEXT("Exit"), true);
		}
	}

	template<class ObjectClass, class PropertyClass>
	PropertyClass GetPropertyValueByName(ObjectClass* Obj, const FString& PropName)
	{
		if (!Obj) return PropertyClass();

		for (TFieldIterator<FProperty> PropIt(Obj->StaticClass()); PropIt; ++PropIt)
		{
			const FProperty* Property = *PropIt;
			if (Property)
			{
				UE_LOG(LogTemp, Display, TEXT("%s"), *Property->GetName());
			}
			if (Property && Property->GetName().Equals(PropName))
			{
				return *Property->ContainerPtrToValuePtr<PropertyClass>(Obj);
			}
		}
		return PropertyClass();
	}
}

void FTurret::Define()
{
	Describe("Creational",
		[this]()
		{
			BeforeEach([this]()
				{
					AutomationOpenMap(MapName);

					World = GetTestGameWorld();
					TestNotNull(TEXT("World exists"), World);
				});
			It("Cpp instance can't be created", [this]() 
				{
					AddExpectedError("SpawnActor failed because class TPTurret is abstract", EAutomationExpectedMessageFlags::Contains);
					const ATPTurret* Turret = World->SpawnActor<ATPTurret>(ATPTurret::StaticClass(), FTransform::Identity);
					TestNull(TEXT("Inventory item exists"), Turret);
				});
			It("Blueprint can be created", [this]()
				{
					const UBlueprint* Blueprint = LoadObject<UBlueprint>(nullptr, *TurretBPName);
					TestNotNull(TEXT("Inventory item exists"), Blueprint);

					const ATPTurret* Turret = World->SpawnActor<ATPTurret>(Blueprint->GeneratedClass, FTransform::Identity);
					TestNotNull(TEXT("Turret exists"), Turret);
				});
			AfterEach([this]() 
				{
					SpecCloseLevel(World);
				});
		});

	Describe("Defaults",
		[this]()
		{
			BeforeEach([this]()
				{
					AutomationOpenMap(MapName);

					World = GetTestGameWorld();
					TestNotNull(TEXT("World exists"), World);
					const UBlueprint* Blueprint = LoadObject<UBlueprint>(nullptr, *TurretBPTestName);
					TestNotNull(TEXT("Inventory item exists"), Blueprint);

					Turret = World->SpawnActor<ATPTurret>(Blueprint->GeneratedClass, FTransform::Identity);
					TestNotNull(TEXT("Turret exists"), Turret);
				});
			const TArray<TTuple<int32, float>> TestData
			{ 
				{45, 2.0f},
				{15, 3.0f},
				{5, 5.0f}
			};
			for (const auto& Data : TestData)
			{
				const auto TestName = FString::Printf(TEXT("Ammo: %i and freq: %.0f should be set up correctly"), Data.Key, Data.Value);
				It(TestName, [this, Data]()
					{
						const auto [Ammo, Freq] = Data;
						CallFuncByNameWithParams(Turret, "SetTurretData", { FString::FromInt(Ammo), FString::SanitizeFloat(Freq) });
						
						const int32 AmmoCount = GetPropertyValueByName<ATPTurret, int32>(Turret, FString("AmmoCount"));
						TestTrueExpr(AmmoCount == Ammo);

						const float ShootingFreq = GetPropertyValueByName<ATPTurret, float>(Turret, FString("FireFrequency"));
						TestTrueExpr(ShootingFreq == Freq);
					});
			}

			AfterEach([this]()
				{
					SpecCloseLevel(World);
				});
		});

	/* Doesn't work in UE5.4*/
	/*
	Describe("Ammo",
		[this]()
		{
			const int32 InitialAmmoCount = 4;
			const float FireFreq = 1.5f;
			LatentBeforeEach(
				[this, InitialAmmoCount, FireFreq](const FDoneDelegate& TestDone)
				{
					AutomationOpenMap(MapName);
					World = GetTestGameWorld();
					TestNotNull("World exists", World);

					const UBlueprint* Blueprint = LoadObject<UBlueprint>(nullptr, *TurretBPTestName);
					TestNotNull(TEXT("Inventory item exists"), Blueprint);

					Turret = World->SpawnActor<ATPTurret>(Blueprint->GeneratedClass, FTransform::Identity);
					TestNotNull(TEXT("Turret exists"), Turret);

					CallFuncByNameWithParams(
						Turret, "SetTurretData", { FString::FromInt(InitialAmmoCount), FString::SanitizeFloat(FireFreq) });
					FPlatformProcess::Sleep(1.0f);
					TestDone.Execute();
				});

			LatentIt(FString::Printf(TEXT("Should be empty after %i sec"), FMath::RoundToInt(InitialAmmoCount * FireFreq)),
				EAsyncExecution::ThreadPool,
				[this, InitialAmmoCount, FireFreq](const FDoneDelegate& TestDone)
				{
					FPlatformProcess::Sleep(1.0f);
					TestDone.Execute();
				});

			LatentAfterEach(
				[this](const FDoneDelegate& TestDone)
				{
					SpecCloseLevel(World);
					FPlatformProcess::Sleep(1.0f);
					TestDone.Execute();
				});
		});*/
}

#endif