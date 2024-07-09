// Fill out your copyright notice in the Description page of Project Settings.

#if (WITH_DEV_AUTOMATION_TESTS || WITH_PERF_AUTOMATION_TESTS)

#include "Tests/TPCharacterTests.h"
#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Tests/TestUtils.h"
#include "TestUtils.h"
#include "TestProjectCharacter.h"
#include "Engine/World.h"
#include "TPTypes.h"
#include "Misc/OutputDeviceNull.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogTPCharacterTests, All, All);

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHealthChangedWithDamage, "TestProject.Character.HealthChangedWithDamage",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority);

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLatentCommandSimpleLog, "TestProject.LatentCommand.LatentCommandSimpleLog",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority);

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCharacterCanBeKilled, "TestProject.Character.CharacterCanBeKilled",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority);

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAutoHealShouldRestoreHealth, "TestProject.Character.AutoHealShouldRestoreHealth",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority);

namespace
{
	const FString CharacterBPName = "/Script/Engine.Blueprint'/Game/Tests/BP_TestFirstPersonCharacter.BP_TestFirstPersonCharacter'";
}


using namespace TestProject;
bool FHealthChangedWithDamage::RunTest(const FString& Parameters)
{
	LevelScope("/Game/Tests/EmptyTestLevel");

	UWorld* World = GetTestGameWorld();
	if (!TestNotNull(TEXT("World exists"), World)) return false;

	const UBlueprint* Blueprint = LoadObject<UBlueprint>(nullptr, *CharacterBPName);
	if (!TestNotNull(TEXT("Character exists"), Blueprint)) return false;

	const FTransform InitialTransform{ FVector{ 0.0f, 0.0f, 110.0f } };
	ATestProjectCharacter* Character = World->SpawnActorDeferred<ATestProjectCharacter>(Blueprint->GeneratedClass, InitialTransform);
	if (!TestNotNull(TEXT("Character exists"), Character)) return false;

	FHealthData HealthData;
	HealthData.MaxHealth = 1000.0f;

	CallFuncByNameWithParams(Character, "SetHealthData",
		{
			HealthData.ToString()
		});

	Character->FinishSpawning(InitialTransform);

	const float DamageAmount = 10.0f;
	TestEqual("Health is full", Character->GetHeallthPercent(), 1.0f);
	Character->TakeDamage(DamageAmount, FDamageEvent{}, nullptr, nullptr);
	TestEqual("Health was full", Character->GetHeallthPercent(), 1.0f - DamageAmount/HealthData.MaxHealth);

	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FTPLatentLogCommand, FString, LogMessage);

bool FTPLatentLogCommand::Update()
{
	UE_LOG(LogTPCharacterTests, Display, TEXT("%s"), *LogMessage);
	return true;
}

bool FLatentCommandSimpleLog::RunTest(const FString& Parameters)
{
	UE_LOG(LogTPCharacterTests, Display, TEXT("000"));
	ADD_LATENT_AUTOMATION_COMMAND(FTPLatentLogCommand("Latent log 1"));
	UE_LOG(LogTPCharacterTests, Display, TEXT("1 1/2"));
	ADD_LATENT_AUTOMATION_COMMAND(FTPLatentLogCommand("Latent log 2"));
	UE_LOG(LogTPCharacterTests, Display, TEXT("333"));

	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FCharacterDestroyedLatentCommand, ATestProjectCharacter*, Character, float, LifeSpan);

bool FCharacterDestroyedLatentCommand::Update()
{
	const double NewTime = FPlatformTime::Seconds();
	if (NewTime - StartTime >= LifeSpan)
	{
		if (IsValid(Character))
		{
			UE_LOG(LogTPCharacterTests, Error, TEXT("Character wasn't destroyed"));
		}
		return true;
	}
	return false;
}

bool FCharacterCanBeKilled::RunTest(const FString& Parameters)
{
	const auto Level = LevelScope("/Game/Tests/EmptyTestLevel");

	UWorld* World = GetTestGameWorld();
	if (!TestNotNull(TEXT("World exists"), World)) return false;

	const UBlueprint* Blueprint = LoadObject<UBlueprint>(nullptr, *CharacterBPName);
	if (!TestNotNull(TEXT("Character exists"), Blueprint)) return false;

	const FTransform InitialTransform{ FVector{ 0.0f, 0.0f, 110.0f } };
	ATestProjectCharacter* Character = World->SpawnActorDeferred<ATestProjectCharacter>(Blueprint->GeneratedClass, InitialTransform);
	if (!TestNotNull(TEXT("Character exists"), Character)) return false;

	FHealthData HealthData;
	HealthData.MaxHealth = 1000.0f;
	HealthData.LifeSpan = 0.5f;

	CallFuncByNameWithParams(Character, "SetHealthData",
		{
			HealthData.ToString()
		});

	Character->FinishSpawning(InitialTransform);

	const float DamageAmount = 10.0f;
	TestEqual("Health is full", Character->GetHeallthPercent(), 1.0f);

	const auto KillingDamageAmount = HealthData.MaxHealth;
	Character->TakeDamage(KillingDamageAmount, FDamageEvent{}, nullptr, nullptr);
	
	TestEqual("Health is empty", Character->GetHeallthPercent(), 0.0f);
	TestTrueExpr(Character->GetCharacterMovement()->MovementMode == EMovementMode::MOVE_None);
	TestTrueExpr(Character->GetCapsuleComponent()->GetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic) == ECollisionResponse::ECR_Ignore);
	TestTrueExpr(Character->GetMesh()->GetCollisionEnabled() == ECollisionEnabled::QueryAndPhysics);
	TestTrueExpr(FMath::IsNearlyEqual(Character->GetLifeSpan(), HealthData.LifeSpan));

	ADD_LATENT_AUTOMATION_COMMAND(FCharacterDestroyedLatentCommand(Character, HealthData.LifeSpan));

	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FAutoHealCheckLatentCommand, ATestProjectCharacter*, Character, float, HealingDuration);

bool FAutoHealCheckLatentCommand::Update()
{
	const double NewTime = FPlatformTime::Seconds();
	if (NewTime - StartTime >= HealingDuration)
	{
		if (!FMath::IsNearlyEqual(Character->GetHeallthPercent(), 1.0f))
		{
			UE_LOG(LogTPCharacterTests, Error, TEXT("Character wasn't healed"));
		}
		return true;
	}
	return false;
}

bool FAutoHealShouldRestoreHealth::RunTest(const FString& Parameters)
{
	const auto Level = LevelScope("/Game/Tests/EmptyTestLevel");

	UWorld* World = GetTestGameWorld();
	if (!TestNotNull(TEXT("World exists"), World)) return false;

	const UBlueprint* Blueprint = LoadObject<UBlueprint>(nullptr, *CharacterBPName);
	if (!TestNotNull(TEXT("Character exists"), Blueprint)) return false;

	const FTransform InitialTransform{ FVector{ 0.0f, 0.0f, 110.0f } };
	ATestProjectCharacter* Character = World->SpawnActorDeferred<ATestProjectCharacter>(Blueprint->GeneratedClass, InitialTransform);
	if (!TestNotNull(TEXT("Character exists"), Character)) return false;

	FHealthData HealthData;
	HealthData.MaxHealth = 200.0f;
	HealthData.HealModifier = 5.0f;
	HealthData.HealRate = 0.5f;

	CallFuncByNameWithParams(Character, "SetHealthData",
		{
			HealthData.ToString()
		});

	Character->FinishSpawning(InitialTransform);

	const float DamageAmount = 10.0f;
	TestEqual("Health is full", Character->GetHeallthPercent(), 1.0f);
	Character->TakeDamage(DamageAmount, FDamageEvent{}, nullptr, nullptr);
	TestEqual("Health was decreased", Character->GetHeallthPercent(), 1.0f - DamageAmount / HealthData.MaxHealth);

	const float HealthDiff = HealthData.MaxHealth * (1.0f - Character->GetHeallthPercent());
	const float HealingDuration = HealthData.HealRate * HealthDiff / HealthData.HealModifier;
	ADD_LATENT_AUTOMATION_COMMAND(FAutoHealCheckLatentCommand(Character, HealingDuration))

	return true;
}

#endif