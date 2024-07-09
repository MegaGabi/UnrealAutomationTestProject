// Fill out your copyright notice in the Description page of Project Settings.

#if WITH_AUTOMATION_TESTS

#include "Tests/TPGameplayTests.h"
#include "Tests/TestUtils.h"
#include "TPTypes.h"
#include "Items/TPInventoryItem.h"
#include "Components/SphereComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/StaticMeshComponent.h"
#include "TestProject/TestProjectCharacter.h"
#include "TestProject/Components/TPInventoryComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Utils/JsonUtils.h"
#include "Utils/InputRecordingUtils.h"

using namespace TestProject;

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryItemCanBeTakenOnJump, "TestProject.Gameplay.InventoryItemCanBeTakenOnJump",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority);

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryItemCantBeReachedOnJump, "TestProject.Gameplay.InventoryItemCantBeReachedOnJump",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority);

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAllItemsCanBeTakenOnMovement, "TestProject.Gameplay.AllItemsCanBeTakenOnMovement",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority);

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAllItemsAreTakenOnRecordingMovement, "TestProject.Gameplay.AllItemsAreTakenOnRecordingMovement",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority);

IMPLEMENT_COMPLEX_AUTOMATION_TEST(FAllItemsAreTakenOnRecordingMovementComplex, "TestProject.Gameplay.AllItemsAreTakenOnRecordingMovementComplex",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority);


namespace
{
	const UInputAction* GetActionBindingByIndexName(UEnhancedInputComponent* InputComp, const FString& ActionName)
	{
		if (!InputComp) return nullptr;

		for (int32 i = 0; i < InputComp->GetActionEventBindings().Num(); ++i)
		{
			FEnhancedInputActionEventBinding* Action = InputComp->GetActionEventBindings()[i].Get();
			const FString CurrentActionName = Action->GetAction()->GetName();

			if (CurrentActionName.Equals(ActionName))
			{
				return Action->GetAction();
			}
		}
		return nullptr;
	}

	class FSimulateMovementLatentCommand : public IAutomationLatentCommand
	{
	public:
		FSimulateMovementLatentCommand(UWorld* InWorld, UEnhancedInputComponent* EnhancedInputComponent, const TArray<FBindingsData>& InBindingsData, UEnhancedPlayerInput* InPlayerInput)
			:World(InWorld),
			 InputComponent(EnhancedInputComponent),
			 BindingsData(InBindingsData),
			 PlayerInput(InPlayerInput)
		{

		}

		virtual bool Update() override
		{
			if (!World || !InputComponent) return true;

			if (WorldStartTime == 0.0f)
			{
				WorldStartTime = World->TimeSeconds;
			}

			while (World->TimeSeconds - WorldStartTime >= BindingsData[Index].WorldTime)
			{
				for (const auto& AxisValue : BindingsData[Index].AxisValues)
				{
					const UInputAction* Action = GetActionBindingByIndexName(InputComponent, AxisValue.Name.ToString());
					if (!Action) return true;
					FInputActionValue ActionValue(AxisValue.Value);
					PlayerInput->InjectInputForAction(Action, ActionValue);
				}

				if (++Index >= BindingsData.Num()) return true;
			}
			return false;
		}

	private:
		const UWorld* World;
		UEnhancedInputComponent* InputComponent;
		UEnhancedPlayerInput* PlayerInput;
		const TArray<FBindingsData> BindingsData;
		int32 Index{ 0 };
		float WorldStartTime{ 0.0f };
	};
}

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FJumpLatentCommand, ACharacter*, Character);

bool FJumpLatentCommand::Update()
{
	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(Character->InputComponent);
	if (!EnhancedInputComponent) return true;

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(Character->GetLocalViewingPlayerController()->GetLocalPlayer());
	if (!Subsystem) return true;

	UEnhancedPlayerInput* PlayerInput = Subsystem->GetPlayerInput();
	if (!PlayerInput) return true;

	const UInputAction* Action = GetActionBindingByIndexName(EnhancedInputComponent, "IA_Jump");
	if (!Action) return true;

	FInputActionValue ActionValue(FVector2D{1.0f, 1.0f});
	PlayerInput->InjectInputForAction(Action, ActionValue);

	return true;
}

bool FInventoryItemCanBeTakenOnJump::RunTest(const FString& Parameters)
{
	const auto Level = LevelScope("/Game/Tests/InventoryTestLevel1");

	UWorld* World = GetTestGameWorld();
	if (!TestNotNull(TEXT("World exists"), World)) return false;

	ACharacter* Character = UGameplayStatics::GetPlayerCharacter(World, 0);
	if (!TestNotNull("Character exists", Character)) return false;

	TArray<AActor*> InventoryItems;
	UGameplayStatics::GetAllActorsOfClass(World, ATPInventoryItem::StaticClass(), InventoryItems);
	if (!TestEqual("Only one item exists", InventoryItems.Num(), 1)) return false;

	ADD_LATENT_AUTOMATION_COMMAND(FEngineWaitLatentCommand(0.1f));
	ADD_LATENT_AUTOMATION_COMMAND(FJumpLatentCommand(Character));
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, World]()
		{
			TArray<AActor*> InventoryItems;
			UGameplayStatics::GetAllActorsOfClass(World, ATPInventoryItem::StaticClass(), InventoryItems);
			TestTrueExpr(InventoryItems.Num() == 0);
		}, 1.0f))

	return true;
}

bool FInventoryItemCantBeReachedOnJump::RunTest(const FString& Parameters)
{
	const auto Level = LevelScope("/Game/Tests/InventoryTestLevel2");

	UWorld* World = GetTestGameWorld();
	if (!TestNotNull(TEXT("World exists"), World)) return false;

	ACharacter* Character = UGameplayStatics::GetPlayerCharacter(World, 0);
	if (!TestNotNull("Character exists", Character)) return false;

	TArray<AActor*> InventoryItems;
	UGameplayStatics::GetAllActorsOfClass(World, ATPInventoryItem::StaticClass(), InventoryItems);
	if (!TestEqual("Only one item exists", InventoryItems.Num(), 1)) return false;

	ADD_LATENT_AUTOMATION_COMMAND(FEngineWaitLatentCommand(0.1f));
	ADD_LATENT_AUTOMATION_COMMAND(FJumpLatentCommand(Character));
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, World]()
		{
			TArray<AActor*> InventoryItems;
			UGameplayStatics::GetAllActorsOfClass(World, ATPInventoryItem::StaticClass(), InventoryItems);
			TestTrueExpr(InventoryItems.Num() == 1);
		}, 1.0f))

		return true;
}

bool FAllItemsCanBeTakenOnMovement::RunTest(const FString& Parameters)
{
	const auto Level = LevelScope("/Game/Tests/InventoryTestLevel3");

	UWorld* World = GetTestGameWorld();
	if (!TestNotNull(TEXT("World exists"), World)) return false;

	ACharacter* Character = UGameplayStatics::GetPlayerCharacter(World, 0);
	if (!TestNotNull("Character exists", Character)) return false;

	TArray<AActor*> InventoryItems;
	UGameplayStatics::GetAllActorsOfClass(World, ATPInventoryItem::StaticClass(), InventoryItems);
	TestTrueExpr(InventoryItems.Num() == 9);

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(Character->InputComponent);
	if (!EnhancedInputComponent) return true;

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(Character->GetLocalViewingPlayerController()->GetLocalPlayer());
	if (!Subsystem) return true;

	UEnhancedPlayerInput* PlayerInput = Subsystem->GetPlayerInput();
	if (!PlayerInput) return true;

	const UInputAction* MoveAction = GetActionBindingByIndexName(EnhancedInputComponent, "IA_Move");
	if (!MoveAction) return true;

	const UInputAction* LookAction = GetActionBindingByIndexName(EnhancedInputComponent, "IA_Look");
	if (!LookAction) return true;

	const auto MoveForward = [MoveAction, PlayerInput]()
		{
			FInputActionValue ActionValue(FVector2D{0.0f, 1.0f});
			PlayerInput->InjectInputForAction(MoveAction, ActionValue);

			return true;
		};

	const auto MoveRight = [MoveAction, PlayerInput]()
		{
			FInputActionValue ActionValue(FVector2D{ 1.0f, 0.0f });
			PlayerInput->InjectInputForAction(MoveAction, ActionValue);

			return true;
		};

	const auto LookRight = [LookAction, PlayerInput]()
		{
			FInputActionValue ActionValue(FVector2D{ 1.0f, 0.0f });
			PlayerInput->InjectInputForAction(LookAction, ActionValue);

			return true;
		};

	ADD_LATENT_AUTOMATION_COMMAND(FEngineWaitLatentCommand(0.1f));
	ADD_LATENT_AUTOMATION_COMMAND(FJumpLatentCommand(Character));
	ADD_LATENT_AUTOMATION_COMMAND(FCustomUntilCommand(MoveForward, 2.f));
	ADD_LATENT_AUTOMATION_COMMAND(FJumpLatentCommand(Character));
	ADD_LATENT_AUTOMATION_COMMAND(FCustomUntilCommand(LookRight, 0.32f));
	ADD_LATENT_AUTOMATION_COMMAND(FCustomUntilCommand(MoveForward, 2.f));
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, World]()
		{
			TArray<AActor*> InventoryItems;
			UGameplayStatics::GetAllActorsOfClass(World, ATPInventoryItem::StaticClass(), InventoryItems);
			TestTrueExpr(InventoryItems.Num() == 0);
			return true;
		}, 1.0f));

	return true;
}

bool FAllItemsAreTakenOnRecordingMovement::RunTest(const FString& Parameters)
{
	const auto Level = LevelScope("/Game/Tests/InventoryTestLevel3");

	UWorld* World = GetTestGameWorld();
	if (!TestNotNull(TEXT("World exists"), World)) return false;

	ACharacter* Character = UGameplayStatics::GetPlayerCharacter(World, 0);
	if (!TestNotNull("Character exists", Character)) return false;

	TArray<AActor*> InventoryItems;
	UGameplayStatics::GetAllActorsOfClass(World, ATPInventoryItem::StaticClass(), InventoryItems);
	TestTrueExpr(InventoryItems.Num() == 9);

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(Character->InputComponent);
	if (!EnhancedInputComponent) return true;

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(Character->GetLocalViewingPlayerController()->GetLocalPlayer());
	if (!Subsystem) return true;

	UEnhancedPlayerInput* PlayerInput = Subsystem->GetPlayerInput();
	if (!PlayerInput) return true;

	FInputData InputData;
	JsonUtils::ReadInputData(FPaths::GameSourceDir().Append("TestProject/Tests/Data/ItemTestMoveData.json"), InputData);

	Character->SetActorTransform(InputData.InitialTransform);
	
	ADD_LATENT_AUTOMATION_COMMAND(FSimulateMovementLatentCommand(World, EnhancedInputComponent, InputData.Bindings, PlayerInput));
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, World]()
		{
			TArray<AActor*> InventoryItems;
			UGameplayStatics::GetAllActorsOfClass(World, ATPInventoryItem::StaticClass(), InventoryItems);
			TestTrueExpr(InventoryItems.Num() == 0);
			return true;
		}, 1.0f));

	return true;
}

void FAllItemsAreTakenOnRecordingMovementComplex::GetTests(TArray<FString>& OutBeautifiedNames, TArray <FString>& OutTestCommands) const
{
	struct FTestData
	{
		FString TestName;
		FString MapPath;
		FString JsonPath;
	};

	const TArray<FTestData> TestData =
	{
		{"MainMapParkour", "/Game/FirstPerson/Maps/FirstPersonMap", FPaths::GameSourceDir()
			.Append("TestProject/Tests/Data/ParkourTestMoveData.json")},
		{"CustomMapParkour", "/Game/Tests/CustomFirstPersonMap", FPaths::GameSourceDir()
			.Append("TestProject/Tests/Data/ParkourCustomMapMoveData.json")}
	};

	for (const auto OneTestData : TestData)
	{
		OutBeautifiedNames.Add(OneTestData.TestName);
		OutTestCommands.Add(FString::Printf(TEXT("%s,%s"), *OneTestData.MapPath, *OneTestData.JsonPath));
	}
}

bool FAllItemsAreTakenOnRecordingMovementComplex::RunTest(const FString& Parameters)
{
	TArray<FString> ParsedParams;
	Parameters.ParseIntoArray(ParsedParams, TEXT(","));
	if (!TestTrue("Map name and JSON params should exist", ParsedParams.Num() == 2)) return false;

	const auto Level = LevelScope(ParsedParams[0]);

	UWorld* World = GetTestGameWorld();
	if (!TestNotNull(TEXT("World exists"), World)) return false;

	ACharacter* Character = UGameplayStatics::GetPlayerCharacter(World, 0);
	if (!TestNotNull("Character exists", Character)) return false;

	TArray<AActor*> InventoryItems;
	UGameplayStatics::GetAllActorsOfClass(World, ATPInventoryItem::StaticClass(), InventoryItems);
	TestTrueExpr(InventoryItems.Num() == 9);

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(Character->InputComponent);
	if (!EnhancedInputComponent) return true;

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(Character->GetLocalViewingPlayerController()->GetLocalPlayer());
	if (!Subsystem) return true;

	UEnhancedPlayerInput* PlayerInput = Subsystem->GetPlayerInput();
	if (!PlayerInput) return true;

	FInputData InputData;
	JsonUtils::ReadInputData(FPaths::GameSourceDir().Append(ParsedParams[1]), InputData);

	Character->SetActorTransform(InputData.InitialTransform);

	ADD_LATENT_AUTOMATION_COMMAND(FSimulateMovementLatentCommand(World, EnhancedInputComponent, InputData.Bindings, PlayerInput));
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, World]()
		{
			TArray<AActor*> InventoryItems;
			UGameplayStatics::GetAllActorsOfClass(World, ATPInventoryItem::StaticClass(), InventoryItems);
			TestTrueExpr(InventoryItems.Num() == 0);
			return true;
		}, 1.0f));

	return true;

	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(1.0f));

	return true;
}

#endif