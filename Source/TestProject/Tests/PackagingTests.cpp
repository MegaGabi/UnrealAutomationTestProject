// Fill out your copyright notice in the Description page of Project Settings.

#if WITH_AUTOMATION_TESTS && WITH_EDITOR

#include "Tests/PackagingTests.h"
#include "CoreMinimal.h"
#include "Tests/AutomationCommon.h"
#include "Misc/AutomationTest.h"
#include "EditorCommandLineUtils.h"
#include "FileHelpers.h"
#include "AutomationBlueprintFunctionLibrary.h"
#include "Misc/OutputDeviceFile.h"
#include "Editor.h"
#include "IAutomationControllerModule.h"
#include "IUATHelperModule.h"
#include "AnalyticsEventAttribute.h"

namespace
{
	FString ArchiveDirectory = FPaths::Combine(FPaths::ConvertRelativePathToFull(FPaths::ProjectDir()), TEXT("BuildFromEditor"));
	FString LogFilePath = FPaths::Combine(FPaths::ConvertRelativePathToFull(FPaths::ProjectLogDir()), TEXT("Packaging.log"));
	FString ProjectPath = FPaths::ConvertRelativePathToFull(FPaths::GetProjectFilePath());

	class FPackagingOutputDevice : public FOutputDeviceFile
	{
	public:
		FPackagingOutputDevice(const TCHAR* InFilename, bool bDisableBackup = false)
			: FOutputDeviceFile(InFilename, bDisableBackup)
		{
		}

		virtual void Serialize(const TCHAR* Message, ELogVerbosity::Type Verbosity, const class FName& Category) override
		{
			FOutputDeviceFile::Serialize(Message, Verbosity, Category);
		}
	};

	struct
	{
		TUniquePtr<FPackagingOutputDevice> PackagingOutputDevice;
		bool PackagingFinished = false;
		bool GameRunOk = false;
		FProcHandle ProcessHandle;
		void* PipeRead = nullptr;
		void* PipeWrite = nullptr;
	} PackageForWindowsInfo;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPackageForWindows, "Packaging.PackageForWindows", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPackageForWindows::RunTest(const FString& Parameters)
{
	PackageForWindowsInfo.PackagingFinished = false;
	PackageForWindowsInfo.GameRunOk = false;
	PackageForWindowsInfo.PipeRead = nullptr;
	PackageForWindowsInfo.PipeWrite = nullptr;

	// Clear the log file
	if (!FFileHelper::SaveStringToFile(TEXT(""), *LogFilePath))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to clear log file."));
		return false;
	}

	auto SaveLogsToFileLambda =
		[]()
		{
			PackageForWindowsInfo.PackagingOutputDevice = MakeUnique<FPackagingOutputDevice>(*LogFilePath);

			// Add the custom output device to GLog
			GLog->AddOutputDevice(PackageForWindowsInfo.PackagingOutputDevice.Get());

			return true;
		};

	auto PackageProjectLambda = 
		[]() {
			FString UnrealCmdExePath = FPaths::Combine(FPaths::ConvertRelativePathToFull(FPaths::EngineDir()), TEXT("Binaries/Win64/UnrealEditor-Cmd.exe"));
			FString CommandLine = FString::Printf(TEXT("-ScriptsForProject=\"%s\" Turnkey -command=VerifySdk -platform=Win64 -UpdateIfNeeded -EditorIO -EditorIOPort=63020  -project=\"%s\" BuildCookRun -nop4 -utf8output -nocompileeditor -skipbuildeditor -cook  -project=\"%s\" -target=TestProject  -unrealexe=\"%s\" -platform=Win64 -stage -archive -package -build -pak -iostore -compressed -prereqs -archivedirectory=\"%s\" -clientconfig=Development"),
				*ProjectPath, *ProjectPath, *ProjectPath, *UnrealCmdExePath, *ArchiveDirectory);

			TArray<FAnalyticsEventAttribute> optionalEmptyArray;
			IUATHelperModule::Get().CreateUatTask(CommandLine, FText(), FText(), FText(),
				FAppStyle::GetBrush(TEXT("MainFrame.PackageProject")), &optionalEmptyArray,
				[](FString, double)
				{
					PackageForWindowsInfo.PackagingFinished = true;
				});

			return true;
		};

	auto StopLoggingLambda =
		[]()
		{
			if (PackageForWindowsInfo.PackagingOutputDevice.IsValid())
			{
				GLog->RemoveOutputDevice(PackageForWindowsInfo.PackagingOutputDevice.Get());
				PackageForWindowsInfo.PackagingOutputDevice->TearDown();
				PackageForWindowsInfo.PackagingOutputDevice.Reset();
			}
			return true;
		};

	auto CheckLogsForSuccessfullPackageLambda =
		[this]()
		{
			TArray<FString> LogLines;

			if (FFileHelper::LoadFileToStringArray(LogLines, *LogFilePath))
			{
				TArray<FString> PhrasesToCheck = {
					TEXT("BUILD COMMAND COMPLETED"),
					TEXT("COOK COMMAND COMPLETED"),
					TEXT("STAGE COMMAND COMPLETED"),
					TEXT("PACKAGE COMMAND COMPLETED"),
					TEXT("ARCHIVE COMMAND COMPLETED")
					// Add more phrases to check here
				};

				TArray<bool> PhraseFound;
				PhraseFound.Init(false, PhrasesToCheck.Num());

				for (const FString& Line : LogLines)
				{
					for (int32 i = 0; i < PhrasesToCheck.Num(); ++i)
					{
						if (!PhraseFound[i] && Line.Contains(PhrasesToCheck[i]))
						{
							PhraseFound[i] = true;
						}
					}
				}

				bool bAllFound = true;
				for (int32 i = 0; i < PhrasesToCheck.Num(); ++i)
				{
					if (!PhraseFound[i])
					{
						AddError(FString::Printf(TEXT("%s was not found in log"), *PhrasesToCheck[i]));
						bAllFound = false;
					}
				}
			}
			else
			{
				AddError(FString::Printf(TEXT("Failed to open log file: %s"), *LogFilePath));
			}

			return true;
		};

	auto CheckThatPackageFoldersAndFilesPresentLambda =
		[]()
		{
			FString ArchiveDirectoryWindows = FPaths::Combine(ArchiveDirectory, TEXT("Windows"));
			IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

			// Get the project name
			FString ProjectName = FApp::GetProjectName();

			// Define the paths to check
			TArray<FString> PathsToCheck = {
				FPaths::Combine(ArchiveDirectoryWindows, TEXT("Engine")),
				FPaths::Combine(ArchiveDirectoryWindows, ProjectName),
				FPaths::Combine(ArchiveDirectoryWindows, ProjectName, TEXT("Binaries"), TEXT("Win64")),
				FPaths::Combine(ArchiveDirectoryWindows, ProjectName, TEXT("Binaries"), TEXT("Win64"), ProjectName + TEXT(".exe")),
				FPaths::Combine(ArchiveDirectoryWindows, TEXT("Engine"), TEXT("Content"), TEXT("Slate")),
				FPaths::Combine(ArchiveDirectoryWindows, ProjectName + TEXT(".exe")),
				FPaths::Combine(ArchiveDirectoryWindows, TEXT("Manifest_NonUFSFiles_Win64.txt")),
				FPaths::Combine(ArchiveDirectoryWindows, TEXT("Manifest_UFSFiles_Win64.txt"))
			};

			bool bAllPresent = true;

			for (const FString& Path : PathsToCheck)
			{
				if (Path.EndsWith(TEXT(".exe")) || Path.EndsWith(TEXT(".txt")))
				{
					if (!PlatformFile.FileExists(*Path))
					{
						UE_LOG(LogTemp, Error, TEXT("File not found: %s"), *Path);
						bAllPresent = false;
					}
				}
				else
				{
					if (!PlatformFile.DirectoryExists(*Path))
					{
						UE_LOG(LogTemp, Error, TEXT("Directory not found: %s"), *Path);
						bAllPresent = false;
					}
				}
			}

			return true;
		};

	auto GameRunLambda =
		[this]()
		{
			FString BatchFilesPath = FPaths::Combine(FPaths::ConvertRelativePathToFull(FPaths::EngineDir()), TEXT("Build/BatchFiles"));
			FString Command = FString::Printf(TEXT("/c \"\"%s/RunUAT.bat\" RunUnreal -project=\"%s\" -build=\"%s/Windows/\" -test=\"UE.TargetAutomation\" -runtest=\"TestProject.Gameplay.InventoryItemCanBeTakenOnJump\"\""),
				*BatchFilesPath, *ProjectPath, *ArchiveDirectory);

			FPlatformProcess::CreatePipe(PackageForWindowsInfo.PipeRead, PackageForWindowsInfo.PipeWrite);

			// Attempt to create the process
			PackageForWindowsInfo.ProcessHandle = FPlatformProcess::CreateProc(
				TEXT("cmd.exe"),  // Path to cmd.exe
				*Command,
				false,           // bLaunchDetached
				true,            // bLaunchHidden
				false,           // bLaunchReallyHidden
				nullptr,         // OutProcessID
				0,               // PriorityModifier
				TEXT("D:/UE_src/UnrealEngine/Engine/Build/BatchFiles"),
				PackageForWindowsInfo.PipeWrite,
				PackageForWindowsInfo.PipeRead
			);

			return true;
		};

	auto EndGameLambda =
		[this]()
		{
			if (PackageForWindowsInfo.ProcessHandle.IsValid())
			{
				FString ReadString = FPlatformProcess::ReadPipe(PackageForWindowsInfo.PipeRead);

				// Get the return code
				int32 ReturnCode = 0;
				FPlatformProcess::GetProcReturnCode(PackageForWindowsInfo.ProcessHandle, &ReturnCode);

				if (ReturnCode != 0)
				{
					AddError(FString::Printf(TEXT("Game process ended with error with return code %d"), ReturnCode));
				}

				// Close the handle
				FPlatformProcess::CloseProc(PackageForWindowsInfo.ProcessHandle);

				FPlatformProcess::ClosePipe(PackageForWindowsInfo.PipeRead, PackageForWindowsInfo.PipeWrite);

				if (!PackageForWindowsInfo.GameRunOk)
				{
					AddError(FString("There was an error while running a game"));
				}
			}
			return true;
		};

	
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand(SaveLogsToFileLambda));
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand(PackageProjectLambda));
	ADD_LATENT_AUTOMATION_COMMAND(FUntilCommand(
		[]() { return PackageForWindowsInfo.PackagingFinished; },
		[]() 
		{ 
			UE_LOG(LogTemp, Error, TEXT("Packaging timeout"));
			return false;
		},
		600.f)); // Ten minutes timeout to package project
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand(StopLoggingLambda));
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand(CheckLogsForSuccessfullPackageLambda));
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand(CheckThatPackageFoldersAndFilesPresentLambda));
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand(GameRunLambda));
	ADD_LATENT_AUTOMATION_COMMAND(FUntilCommand(
		[]() 
		{
			if (!PackageForWindowsInfo.ProcessHandle.IsValid())
			{
				return true;
			}
			FString ReadString = FPlatformProcess::ReadPipe(PackageForWindowsInfo.PipeRead);

			if (ReadString.Contains(TEXT("UE.TargetAutomation(RunTest=TestProject.Gameplay.InventoryItemCanBeTakenOnJump) (Win64 Development Client) result=Passed")))
			{
				UE_LOG(LogTemp, Log, TEXT("Process output: %s"), *ReadString);
				PackageForWindowsInfo.GameRunOk = true;
			}
			return !FWindowsPlatformProcess::IsProcRunning(PackageForWindowsInfo.ProcessHandle); 
		},
		[]()
		{
			UE_LOG(LogTemp, Error, TEXT("Game run timeout"));
			return false;
		},
		300.f)); // five minutes timeout to run game and test
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand(EndGameLambda));

	return true;
}
#endif