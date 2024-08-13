// Fill out your copyright notice in the Description page of Project Settings.

#if WITH_AUTOMATION_TESTS && WITH_EDITOR

#include "Tests/SessionFrontendTests.h"

#include "CoreMinimal.h"

#include "Developer/LauncherServices/Public/ILauncherServicesModule.h"
#include "Developer/LauncherServices/Public/ILauncherWorker.h"
#include "Developer/TargetDeviceServices/Public/ITargetDeviceProxyManager.h"
#include "Developer/TargetDeviceServices/Public/ITargetDeviceServicesModule.h"
#include "Editor/DeviceProfileServices/Public/IDeviceProfileServicesModule.h"
#include "Misc/AutomationTest.h"

namespace
{
    FString ProjectPath = FPaths::ConvertRelativePathToFull(FPaths::GetProjectFilePath());
    FString OutputPath = FPaths::Combine(FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()), TEXT("StagedBuilds"));
    FString PackagedDir = FPaths::Combine(OutputPath, TEXT("Windows"));

    struct
    {
        ITargetDeviceServicesModule* TargetDeviceServicesModule;
        TSharedPtr<ITargetDeviceProxyManager> InDeviceProxyManager;
        ILauncherServicesModule* LauncherServicesModule;
        ILauncherProfileManagerPtr ProfileManager;
        ILauncherProfilePtr CustomProfile;
        ILauncherPtr Launcher;
        ILauncherWorkerPtr LauncherThread;
        TArray<FString> PackagingLogLines;
        void* PipeRead = nullptr;
        void* PipeWrite = nullptr;
        FProcHandle ProcessHandle;
        bool GameRunOk = false;
        bool ProfileRemoved = false;
    } TestRunData;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWindowsByTheBook, "SessionFrontend.Launch.WindowsByTheBook", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FWindowsByTheBook::RunTest(const FString& Parameters)
{
    auto InitProfileLambda = [this]()
        {
            TestRunData.TargetDeviceServicesModule = &FModuleManager::LoadModuleChecked<ITargetDeviceServicesModule>("TargetDeviceServices");
            if (!TestRunData.TargetDeviceServicesModule)
            {
                AddError(TEXT("Failed to load TargetDeviceServices module"));
                return false;
            }

            TestRunData.InDeviceProxyManager = TestRunData.TargetDeviceServicesModule->GetDeviceProxyManager().ToSharedPtr();
            if (!TestRunData.InDeviceProxyManager.IsValid())
            {
                AddError(TEXT("Failed to get DeviceProxyManager"));
                return false;
            }

            TestRunData.LauncherServicesModule = &FModuleManager::LoadModuleChecked<ILauncherServicesModule>("LauncherServices");
            if (!TestRunData.LauncherServicesModule)
            {
                AddError(TEXT("Failed to load LauncherServices module"));
                return false;
            }

            TestRunData.ProfileManager = TestRunData.LauncherServicesModule->GetProfileManager().ToSharedPtr();
            if (!TestRunData.ProfileManager.IsValid())
            {
                AddError(TEXT("Failed to get ProfileManager"));
                return false;
            }

            TestRunData.CustomProfile = TestRunData.ProfileManager->AddNewProfile().ToSharedPtr();
            if (!TestRunData.CustomProfile.IsValid())
            {
                AddError(TEXT("Failed to create new profile"));
                return false;
            }

            FString ProfileFileName = TestRunData.CustomProfile->GetFilePath();

            TestRunData.CustomProfile->SetProjectPath(ProjectPath);
            TestRunData.CustomProfile->SetName(TEXT("AutomatedTestProfile"));
            TestRunData.CustomProfile->SetBuildConfiguration(EBuildConfigurations::Development);
            TestRunData.CustomProfile->AddCookedPlatform(TEXT("Windows"));
            TestRunData.CustomProfile->SetCookMode(ELauncherProfileCookModes::ByTheBook);
            TestRunData.CustomProfile->SetBuildMode(ELauncherProfileBuildModes::Build);
            TestRunData.CustomProfile->SetDeploymentMode(ELauncherProfileDeploymentModes::DoNotDeploy);
            TestRunData.CustomProfile->SetPackagingMode(ELauncherProfilePackagingModes::Locally);
            TestRunData.CustomProfile->SetPackageDirectory(OutputPath);

            TestRunData.Launcher = TestRunData.LauncherServicesModule->CreateLauncher().ToSharedPtr();
            if (!TestRunData.Launcher.IsValid())
            {
                AddError(TEXT("Failed to create Launcher"));
                return false;
            }

            TestRunData.LauncherThread = TestRunData.Launcher->Launch(TestRunData.InDeviceProxyManager.ToSharedRef(), TestRunData.CustomProfile.ToSharedRef());
            if (!TestRunData.LauncherThread.IsValid())
            {
                AddError(TEXT("Failed to launch LauncherThread"));
                return false;
            }

            if (!TestRunData.ProfileManager->SaveProfile(TestRunData.CustomProfile.ToSharedRef()))
            {
                AddError(TEXT("Failed to save CustomProfile"));
                return false;
            }

            if (!IFileManager::Get().Delete(*ProfileFileName))
            {
                UE_LOG(LogTemp, Warning, TEXT("Failed to delete initial profile file: %s"), *ProfileFileName);
            }

            TestRunData.PackagingLogLines.Empty();
            TestRunData.LauncherThread->OnOutputReceived().
                AddLambda([this](const FString& InMessage)
                {
                    TestRunData.PackagingLogLines.Add(InMessage);
                });

            return true;
        };

    auto CheckLogsForSuccessfullPackageLambda =
        [this]()
        {
            TArray<FString> PhrasesToCheck = {
                TEXT("BUILD COMMAND COMPLETED"),
                TEXT("COOK COMMAND COMPLETED"),
                TEXT("STAGE COMMAND COMPLETED"),
                TEXT("PACKAGE COMMAND COMPLETED")
                // Add more phrases to check here
            };

            TArray<bool> PhraseFound;
            PhraseFound.Init(false, PhrasesToCheck.Num());

            for (const FString& Line : TestRunData.PackagingLogLines)
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

            return true;
        };

    auto CheckThatPackageFoldersAndFilesPresentLambda =
        []()
        {
            IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

            // Get the project name
            FString ProjectName = FApp::GetProjectName();

            // Define the paths to check
            TArray<FString> PathsToCheck = {
                FPaths::Combine(PackagedDir, TEXT("Engine")),
                FPaths::Combine(PackagedDir, ProjectName),
                FPaths::Combine(PackagedDir, ProjectName, TEXT("Binaries"), TEXT("Win64")),
                FPaths::Combine(PackagedDir, ProjectName, TEXT("Binaries"), TEXT("Win64"), ProjectName + TEXT(".exe")),
                FPaths::Combine(PackagedDir, TEXT("Engine"), TEXT("Content"), TEXT("Slate")),
                FPaths::Combine(PackagedDir, ProjectName + TEXT(".exe")),
                FPaths::Combine(PackagedDir, TEXT("Manifest_NonUFSFiles_Win64.txt")),
                FPaths::Combine(PackagedDir, TEXT("Manifest_UFSFiles_Win64.txt"))
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
                *BatchFilesPath, *ProjectPath, *OutputPath);

            if (!FPlatformProcess::CreatePipe(TestRunData.PipeRead, TestRunData.PipeWrite))
            {
                AddError(TEXT("Failed to create pipe for process communication"));
                return false;
            }

            // Attempt to create the process
            TestRunData.ProcessHandle = FPlatformProcess::CreateProc(
                TEXT("cmd.exe"),  // Path to cmd.exe
                *Command,
                false,           // bLaunchDetached
                true,            // bLaunchHidden
                false,           // bLaunchReallyHidden
                nullptr,         // OutProcessID
                0,               // PriorityModifier
                TEXT("D:/UE_src/UnrealEngine/Engine/Build/BatchFiles"),
                TestRunData.PipeWrite,
                TestRunData.PipeRead
            );

            if (!TestRunData.ProcessHandle.IsValid())
            {
                AddError(TEXT("Failed to create game process"));
                return false;
            }

            return true;
        };

    auto EndGameLambda =
        [this]()
        {
            if (TestRunData.ProcessHandle.IsValid())
            {
                FString ReadString = FPlatformProcess::ReadPipe(TestRunData.PipeRead);

                // Get the return code
                int32 ReturnCode = 0;
                FPlatformProcess::GetProcReturnCode(TestRunData.ProcessHandle, &ReturnCode);

                if (ReturnCode != 0)
                {
                    AddError(FString::Printf(TEXT("Game process ended with error with return code %d"), ReturnCode));
                }

                // Close the handle
                FPlatformProcess::CloseProc(TestRunData.ProcessHandle);

                FPlatformProcess::ClosePipe(TestRunData.PipeRead, TestRunData.PipeWrite);

                if (!TestRunData.GameRunOk)
                {
                    AddError(FString("There was an error while running a game"));
                }
            }
            return true;
        };

    auto CleanupPackageDirectoryLambda = [this]()
        {
            // Delete the packaged build directory
            if (IFileManager::Get().DirectoryExists(*PackagedDir))
            {
                IFileManager::Get().DeleteDirectory(*PackagedDir, false, true);
            }
            TestFalse("Package directory cleaned up", IFileManager::Get().DirectoryExists(*PackagedDir));

            return true;
        };

    ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand(InitProfileLambda));
    ADD_LATENT_AUTOMATION_COMMAND(FUntilCommand(
        []() { return TestRunData.LauncherThread->GetStatus() == ELauncherWorkerStatus::Completed; },
        []() { return true; },
        600.f));
    ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand(CheckLogsForSuccessfullPackageLambda));
    ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([]()
        {
            TestRunData.ProfileManager->RemoveProfile(TestRunData.CustomProfile.ToSharedRef());
            return true;
        }));
    ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand(CheckThatPackageFoldersAndFilesPresentLambda));
    ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand(GameRunLambda));
    ADD_LATENT_AUTOMATION_COMMAND(FUntilCommand(
        []()
        {
            if (!TestRunData.ProcessHandle.IsValid())
            {
                return true;
            }
            FString ReadString = FPlatformProcess::ReadPipe(TestRunData.PipeRead);

            if (ReadString.Contains(TEXT("UE.TargetAutomation(RunTest=TestProject.Gameplay.InventoryItemCanBeTakenOnJump) (Win64 Development Client) result=Passed")))
            {
                UE_LOG(LogTemp, Log, TEXT("Process output: %s"), *ReadString);
                TestRunData.GameRunOk = true;
            }
            return !FWindowsPlatformProcess::IsProcRunning(TestRunData.ProcessHandle);
        },
        []()
        {
            UE_LOG(LogTemp, Error, TEXT("Game run timeout"));
            return false;
        },
        300.f)); // five minutes timeout to run game and test
    ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand(EndGameLambda));
    ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand(CleanupPackageDirectoryLambda));

    return true;
}

#endif

