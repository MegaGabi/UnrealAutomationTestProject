#if WITH_AUTOMATION_TESTS && WITH_EDITOR

#include "Tests/MeshPreviewTest.spec.h"

#include "IAutomationDriver.h"
#include "IAutomationDriverModule.h"
#include "IDriverElement.h"
#include "LocateBy.h"
#include "Framework/MetaData/DriverMetaData.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "IDriverSequence.h"
#include "Editor.h"
#include "Widgets/Text/STextBlock.h"
#include "Editor/Persona/Public/PersonaModule.h"
#include "Editor/Persona/Public/IPersonaToolkit.h"
#include "Editor/SkeletalMeshEditor/Private/SkeletalMeshEditor.h"
#include "Editor/SkeletalMeshEditor/Public/ISkeletalMeshEditorModule.h"
#include "DetailLayoutBuilder.h"
#include "Editor/AssetDefinition/Public/AssetDefinition.h"
#include "IPersonaPreviewScene.h"
#include "Editor/AdvancedPreviewScene/Public/AssetViewerSettings.h"

BEGIN_DEFINE_SPEC(FSkyRotationTest, "MeshPreview", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::ApplicationContextMask)
USkeletalMesh* SkeletalMesh;
TSharedPtr<ISkeletalMeshEditor> SkeletalMeshEditor;
TSharedPtr<FAdvancedPreviewScene> PreviewScene;
END_DEFINE_SPEC(FSkyRotationTest)

void FSkyRotationTest::Define()
{
    BeforeEach([this]()
        {
            AddExpectedError("Menu already registered");

            FString AssetPath = TEXT("/Script/Engine.SkeletalMesh'/Game/Characters/Mannequins/Meshes/SKM_Manny.SKM_Manny'");
            SkeletalMesh = LoadObject<USkeletalMesh>(nullptr, *AssetPath);
            if (!TestNotNull(TEXT("Skeletal Mesh not null"), SkeletalMesh)) return;

            ISkeletalMeshEditorModule& SkeletalMeshEditorModule = FModuleManager::LoadModuleChecked<ISkeletalMeshEditorModule>("SkeletalMeshEditor");
            SkeletalMeshEditor = SkeletalMeshEditorModule.CreateSkeletalMeshEditor(EToolkitMode::Type::Standalone, TSharedPtr<IToolkitHost>(), SkeletalMesh).ToSharedPtr();
            if (!TestNotNull(TEXT("Skeletal Mesh Editor not null"), SkeletalMeshEditor.Get())) return;

            PreviewScene = StaticCastSharedRef<FAdvancedPreviewScene>(SkeletalMeshEditor->GetPersonaToolkit()->GetPreviewScene()).ToSharedPtr();
            if (!TestNotNull(TEXT("Preview Scene not null"), PreviewScene.Get())) return;

            UAssetViewerSettings::Get()->Profiles[PreviewScene->GetCurrentProfileIndex()].bRotateLightingRig = false;
            UAssetViewerSettings::Get()->Profiles[PreviewScene->GetCurrentProfileIndex()].RotationSpeed = 0.f;
            UAssetViewerSettings::Get()->Save();
        });

    Describe("Sky", [this]()
        {
            for (int i = 1; i <= 10; ++i)
            {
                float RotationSpeed = 10*i;

                FString TestName = FString::Printf(TEXT("Should Rotate by %d degrees in 1 second if rotation speed is %d"),
                    FMath::FloorToInt(RotationSpeed), FMath::FloorToInt(RotationSpeed));

                It(TestName, EAsyncExecution::ThreadPool,
                    [this, RotationSpeed]()
                    {
                        int32 CurrentProfileIndex = PreviewScene->GetCurrentProfileIndex();
                        UAssetViewerSettings::Get()->Profiles[CurrentProfileIndex].bRotateLightingRig = true;
                        UAssetViewerSettings::Get()->Profiles[CurrentProfileIndex].RotationSpeed = RotationSpeed;
                        UAssetViewerSettings::Get()->Profiles[CurrentProfileIndex].LightingRigRotation = 0.f;
                        UAssetViewerSettings::Get()->Save();
                        FPlatformProcess::Sleep(.5f);
                        // Get initial rotation
                        float ExpectedRotation = 0;
                        float ActualRotation = 0;

                        bool FirstIteration = true;
                        FDelegateHandle TickHandle = GEditor->OnPostEditorTick().AddLambda(
                            [this, &ExpectedRotation, &RotationSpeed, &FirstIteration, &ActualRotation](float DeltaTime)
                            {
                                if (FirstIteration)
                                {
                                    ExpectedRotation = PreviewScene->GetSkyRotation();
                                    FirstIteration = false;
                                }
                                else
                                {
                                    ActualRotation = PreviewScene->GetSkyRotation();
                                    ExpectedRotation += RotationSpeed * DeltaTime;
                                }
                            }
                        );
                        FPlatformProcess::Sleep(.5f);
                        GEditor->OnPostEditorTick().Remove(TickHandle);                     

                        FString ErrorMessage = FString::Printf(TEXT("Expected rotation %f to be equal to %f"), ActualRotation, ExpectedRotation);
                        TestTrue(ErrorMessage, FMath::IsNearlyEqual(ActualRotation, ExpectedRotation, 1.f));
                    });
            }
        });

    AfterEach([this]()
        {
            int32 CurrentProfileIndex = PreviewScene->GetCurrentProfileIndex();
            UAssetViewerSettings::Get()->Profiles[CurrentProfileIndex].bRotateLightingRig = false;
            UAssetViewerSettings::Get()->Profiles[CurrentProfileIndex].RotationSpeed = 0.f;
            UAssetViewerSettings::Get()->Profiles[CurrentProfileIndex].LightingRigRotation = 0.f;
            UAssetViewerSettings::Get()->Save();

            SkeletalMeshEditor->CloseWindow();
        });
}

#endif