// Fill out your copyright notice in the Description page of Project Settings.

#if WITH_AUTOMATION_TESTS

#include "Tests/TPScreenshots.h"
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
#include "AutomationBlueprintFunctionLibrary.h"
#include "Camera/CameraActor.h"
#include "BufferVisualizationData.h"
#include "Engine/DamageEvents.h"

using namespace TestProject;

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRenderingShouldBeCorrect, "TestProject.Screenshots.RenderingShouldBeCorrect",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority);

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMainPlayerHUDShouldBeRendered, "TestProject.Screenshots.MainPlayerHUDShouldBeRendered",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority);

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHealthWidgetShouldBeRenderedCorrectlyAfterDamage, "TestProject.Screenshots.HealthWidgetShouldBeRenderedCorrectlyAfterDamage",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority);

class FTakeScreenshotLatentCommand : public IAutomationLatentCommand
{
public:
	FTakeScreenshotLatentCommand(const FString& InScreenshotName) : ScreenshotName(InScreenshotName)
	{
		FAutomationTestFramework::Get().OnScreenshotTakenAndCompared.AddRaw(this, &FTakeScreenshotLatentCommand::OnScreenshotTakenAndCompared);
	}
	virtual ~FTakeScreenshotLatentCommand()
	{
		FAutomationTestFramework::Get().OnScreenshotTakenAndCompared.RemoveAll(this);
	}

	virtual bool Update() override
	{
		if (!ScreenshotRequested)
		{
			const auto Options = UAutomationBlueprintFunctionLibrary::GetDefaultScreenshotOptionsForRendering();
			UAutomationBlueprintFunctionLibrary::TakeAutomationScreenshotInternal(GetTestGameWorld(), ScreenshotName, FString{}, Options);
			ScreenshotRequested = true;
		}
		return CommandCompleted;
	}

private:
	void OnScreenshotTakenAndCompared()
	{
		CommandCompleted = true;
	}
private:
	const FString ScreenshotName;
	bool ScreenshotRequested{ false };
	bool CommandCompleted{ false };
};

class FTakeUIScreenshotLatentCommand : public IAutomationLatentCommand
{
public:
	FTakeUIScreenshotLatentCommand(const FString& InScreenshotName) : ScreenshotName(InScreenshotName)
	{
		FAutomationTestFramework::Get().OnScreenshotTakenAndCompared.AddRaw(this, &FTakeUIScreenshotLatentCommand::OnScreenshotTakenAndCompared);
	}
	virtual ~FTakeUIScreenshotLatentCommand()
	{
		FAutomationTestFramework::Get().OnScreenshotTakenAndCompared.RemoveAll(this);
	}

	virtual bool Update() override
	{
		if (!ScreenshotSetupDone)
		{
			ScreenshotSetupDone = true;
			SetBufferVisualization("Opacity");
			return false;
		}

		if (!ScreenshotRequested)
		{
			const auto Options = UAutomationBlueprintFunctionLibrary::GetDefaultScreenshotOptionsForRendering();
			UAutomationBlueprintFunctionLibrary::TakeAutomationScreenshotOfUI_Immediate(GetTestGameWorld(), ScreenshotName, Options);
			ScreenshotRequested = true;
		}
		return CommandCompleted;
	}

private:
	void OnScreenshotTakenAndCompared()
	{
		CommandCompleted = true;
		SetBufferVisualization(NAME_None);
	}

	void SetBufferVisualization(const FName& VisualizeBuffer)
	{
		if (UGameViewportClient* ViewportClient = AutomationCommon::GetAnyGameViewportClient())
		{
			static IConsoleVariable* ICVar = IConsoleManager::Get().FindConsoleVariable(FBufferVisualizationData::GetVisualizationTargetConsoleCommandName());
			if (ICVar)
			{
				if (ViewportClient->GetEngineShowFlags())
				{
					ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(VisualizeBuffer == NAME_None ? false : true);
					ViewportClient->GetEngineShowFlags()->SetTonemapper(VisualizeBuffer == NAME_None ? true : false);
					ICVar->Set(VisualizeBuffer == NAME_None ? TEXT("") : *VisualizeBuffer.ToString());
				}
			}
		}
	}
private:
	const FString ScreenshotName;
	bool ScreenshotRequested{ false };
	bool CommandCompleted{ false };
	bool ScreenshotSetupDone{ false };
};

bool FRenderingShouldBeCorrect::RunTest(const FString& Parameters)
{
	const auto Level = LevelScope("/Game/FirstPerson/Maps/FirstPersonMap");

	UWorld* World = GetTestGameWorld();
	if (!TestNotNull("World exists", World)) return false;

	const FTransform Transform{ FVector{1390.0f, 2390.0f, 160.0f} };
	ACameraActor* Camera = World->SpawnActor<ACameraActor>(ACameraActor::StaticClass(), Transform);

	APlayerController* PC = World->GetFirstPlayerController();
	if (!TestNotNull("Player controller exists", PC)) return false;

	PC->SetViewTarget(Camera);

	ADD_LATENT_AUTOMATION_COMMAND(FTakeScreenshotLatentCommand("rendering_check_screenshot"));

	return true;
}

bool FMainPlayerHUDShouldBeRendered::RunTest(const FString& Parameters)
{
	const auto Level = LevelScope("/Game/FirstPerson/Maps/FirstPersonMap");

	UWorld* World = GetTestGameWorld();
	if (!TestNotNull("World exists", World)) return false;

	ADD_LATENT_AUTOMATION_COMMAND(FTakeUIScreenshotLatentCommand("player_health_widget_screenshot"));

	return true;
}

bool FHealthWidgetShouldBeRenderedCorrectlyAfterDamage::RunTest(const FString& Parameters)
{
	const auto Level = LevelScope("/Game/FirstPerson/Maps/FirstPersonMap");

	UWorld* World = GetTestGameWorld();
	if (!TestNotNull("World exists", World)) return false;

	APlayerController* PC = World->GetFirstPlayerController();
	if (!TestNotNull("Player controller exists", PC)) return false;

	APawn* Pawn = PC->GetPawn();
	if (!TestNotNull("Pawn exists", Pawn)) return false;

	const float DamageAmount = 40.0f;
	Pawn->TakeDamage(DamageAmount, FDamageEvent{}, nullptr, nullptr);

	ADD_LATENT_AUTOMATION_COMMAND(FTakeUIScreenshotLatentCommand("main_player_screenshot"));

	return true;
}

#endif
