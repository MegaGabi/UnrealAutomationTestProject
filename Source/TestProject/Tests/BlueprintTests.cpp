// Fill out your copyright notice in the Description page of Project Settings.

#if WITH_AUTOMATION_TESTS && WITH_EDITOR

#include "BlueprintTests.h"
#include "CoreMinimal.h"
#include "Tests/AutomationCommon.h"
#include "Misc/AutomationTest.h"

#include "Kismet2/KismetEditorUtilities.h"
#include "AssetRegistry/AssetRegistryModule.h"

namespace
{
	const FString MissingNodeBlueprintName = "/Game/BP_NodeClassIsMissing";

	bool CompileBlueprint(const FString& BlueprintPath, FString& OutString)
	{
		UBlueprint* Blueprint = Cast<UBlueprint>(StaticLoadObject(UBlueprint::StaticClass(), nullptr, *BlueprintPath));

		if (Blueprint == nullptr)
		{
			OutString = FString::Printf(TEXT("Blueprint %s couldn't be created"), *BlueprintPath);
			return false;
		}

		FCompilerResultsLog Result;
		FKismetEditorUtilities::CompileBlueprint(Blueprint, EBlueprintCompileOptions::SkipSave, &Result);

		if (Result.NumErrors > 0)
		{
			for (auto Message : Result.Messages)
			{
				if (Message.Get().GetSeverity() == EMessageSeverity::Error)
				{
					OutString += Message.Get().ToText().ToString() + "\n";
				}
			}
			return false;
		}

		return true;
	}

	bool OpenBlueprint(const FString& BlueprintPath, FString& OutString)
	{
		UBlueprint* Blueprint = Cast<UBlueprint>(StaticLoadObject(UBlueprint::StaticClass(), nullptr, *BlueprintPath));

		if (Blueprint == nullptr)
		{
			OutString = FString::Printf(TEXT("Blueprint %s couldn't be created"), *BlueprintPath);
			return false;
		}

		FKismetEditorUtilities::BringKismetToFocusAttentionOnObject(Blueprint);

		IAssetEditorInstance* OpenedEditor = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->FindEditorForAsset(Blueprint, false);

		if (OpenedEditor != nullptr)
		{
			OutString = FString::Printf(TEXT("Blueprint %s couldn't be opened"), *BlueprintPath);
			return true;
		}

		return false;
	}

}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCheckIfBPCanBeCompiledWithMissingNode, "Blueprints.Compiler.CheckIfBPCanBeCompiledWithMissingNode", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCheckIfBPCanBeOpenedWithMissingNode, "Blueprints.Compiler.CheckIfBPCanBeOpenedWithMissingNode", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCheckIfBPCanBeCompiledWithMissingNode::RunTest(const FString& Parameters)
{
	FString ErrorString;
	if(!TestTrue("Compilation of blueprint with a missing node should be successful", CompileBlueprint(MissingNodeBlueprintName, ErrorString)))
	{
		UE_LOG(LogTemp, Error, TEXT("%s"), *ErrorString);
	}
	return true;
}

bool FCheckIfBPCanBeOpenedWithMissingNode::RunTest(const FString& Parameters)
{
	FString ErrorString;
	if (!TestTrue("Opening of blueprint with a missing node should be successful", OpenBlueprint(MissingNodeBlueprintName, ErrorString)))
	{
		UE_LOG(LogTemp, Error, TEXT("%s"), *ErrorString);
	}
	return true;
}

#endif
