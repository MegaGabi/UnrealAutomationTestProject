// Fill out your copyright notice in the Description page of Project Settings.

#if WITH_AUTOMATION_TESTS && WITH_EDITOR

#include "IAutomationDriver.h"
#include "IAutomationDriverModule.h"
#include "IDriverElement.h"
#include "LocateBy.h"
#include "Framework/MetaData/DriverMetaData.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "IDriverSequence.h"
#include "Editor.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
    void PrintWidgetInfo(const TSharedPtr<SWidget>& Widget, int Depth = 0)
    {
        if (!Widget.IsValid())
        {
            return;
        }

        // Create indentation based on depth
        FString Indent = FString::ChrN(Depth * 2, ' ');

        // Print basic widget info
        //if (Widget->ToString().Equals("SFilterList"))
        {
            UE_LOG(LogTemp, Warning, TEXT("%s%s Tag: %s"),
                *Indent,
                *Widget->ToString(),
                *Widget->GetTag().ToString());
        }

        // Get the metadata list for the Widget.
        auto MetaDataList = Widget->GetAllMetaData<FTagMetaData>();

        // Check if the Widget has any metadata.
        for (auto MetaData : MetaDataList)
        {
            UE_LOG(LogTemp, Error, TEXT("Meta data class name: %s"), *MetaData->Tag.ToString());
        }

        // Recursively print info for child widgets
        FChildren* Children = Widget->GetChildren();
        for (int32 i = 0; i < Children->Num(); ++i)
        {
            TSharedRef<SWidget> ChildWidget = Children->GetChildAt(i);
            PrintWidgetInfo(ChildWidget, Depth + 1);
        }
    }
}

BEGIN_DEFINE_SPEC(FFilterListTest, "ContentBrowser.FilterListTest", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::ApplicationContextMask)
TSharedPtr<SWindow> ContentBrowserWindow;
TSharedPtr<SWidget> ContentBrowserWidget;
FAutomationDriverPtr Driver;
END_DEFINE_SPEC(FFilterListTest)

void FFilterListTest::Define()
{
    BeforeEach([this]()
        {
            if (IAutomationDriverModule::Get().IsEnabled())
            {
                IAutomationDriverModule::Get().Disable();
            }

            IAutomationDriverModule::Get().Enable();

            FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
            ContentBrowserModule.Get().ForceShowPluginContent(true);

            ContentBrowserWidget = ContentBrowserModule.Get().CreateContentBrowser("MyContentBrowser", nullptr, nullptr);

            if (!ContentBrowserWindow.IsValid())
            {
                // Add the window to the Slate application
                ContentBrowserWindow = FSlateApplication::Get().AddWindow(
                    SNew(SWindow)
                    .Title(FText::FromString("Standalone Content Browser"))
                    .ClientSize(FVector2D(800, 600))
                    .SupportsMaximize(true)
                    .SupportsMinimize(true)
                    .SizingRule(ESizingRule::UserSized)
                    [
                        ContentBrowserWidget.ToSharedRef()
                    ]);
                ContentBrowserWindow->SetOnWindowClosed(FOnWindowClosed::CreateLambda([this](const TSharedRef<SWindow>& Window) { ContentBrowserWindow.Reset(); }));
            }

            ContentBrowserWindow->BringToFront(true);
            FSlateApplication::Get().SetKeyboardFocus(ContentBrowserWindow, EFocusCause::SetDirectly);

            Driver = IAutomationDriverModule::Get().CreateDriver();
        });

    Describe("Filter options", [this]()
        {
            It("Automation Driver check filter options", EAsyncExecution::ThreadPool, [this]()
                {
                    //PrintWidgetInfo(ContentBrowserWidget);
                    TestTrue("Filter button click", Driver->FindElement(By::Path("ContentBrowserFiltersCombo"))->Click());
                    FPlatformProcess::Sleep(0.1);

                    FDriverSequenceRef MoveToFilterListSequence = Driver->CreateSequence();
                    MoveToFilterListSequence->Actions().MoveByOffset(100, 160);
                    MoveToFilterListSequence->Perform();

                    Driver->FindElement(By::Cursor())->Hover();

                    int ExpectedFilterOptionsIndex = 0;
                    const TArray<FString> ExpectedFilterOptions =
                    {
                        "Niagara System",
                        "Blueprint Class",
                        "C++ Class",
                        "Material",
                        "Skeletal Mesh",
                        "Static Mesh",
                        "Texture"
                    };

                    for (int i = 0; i < 7 && ExpectedFilterOptionsIndex < ExpectedFilterOptions.Num(); ++i)
                    {
                        FString CurrentExpectedItem = ExpectedFilterOptions[ExpectedFilterOptionsIndex];

                        auto FilterListItem = Driver->FindElement(By::Cursor());
                        FilterListItem->Hover();

                        auto FilterListItemText = FilterListItem->GetText().ToString();
                        if (!FilterListItemText.IsEmpty())
                        {
                            FString Message = FString::Printf(TEXT("List item %s is expected to be equal to %s"), 
                                *FilterListItemText, *CurrentExpectedItem);
                            TestTrue(Message, FilterListItemText.Equals(CurrentExpectedItem));
                            ++ExpectedFilterOptionsIndex;
                        }

                        FPlatformProcess::Sleep(0.1); // Mouse moves too fast without sleep
                        FDriverSequenceRef Sequence = Driver->CreateSequence();
                        Sequence->Actions().MoveByOffset(0, 30);
                        Sequence->Perform();
                    }

                    //PrintWidgetInfo(ContentBrowserWidget);

                    //auto FilterWidget = Driver->FindElement(By::Id("ContentBrowserFilters")); 
                });
        });

    AfterEach([this]()
        {
            Driver.Reset();
            IAutomationDriverModule::Get().Disable();
        });
}

#endif