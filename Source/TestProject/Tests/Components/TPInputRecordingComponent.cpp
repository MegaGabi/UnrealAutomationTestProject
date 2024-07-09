// Fill out your copyright notice in the Description page of Project Settings.


#include "Tests/Components/TPInputRecordingComponent.h"
#include "Engine/World.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/Character.h"
#include "TestProject/Tests/Utils/JsonUtils.h"
#include "GameFramework/PlayerInput.h"

using namespace TestProject;

UTPInputRecordingComponent::UTPInputRecordingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}


void UTPInputRecordingComponent::BeginPlay()
{
	Super::BeginPlay();

	check(GetOwner());
	check(GetWorld());
	check(GetOwner()->InputComponent);

	const APawn* Pawn = Cast<APawn>(GetOwner());
	check(Pawn);

	APlayerController* PlayerController = Pawn->GetController<APlayerController>();
	check(PlayerController);

	PlayerInput = PlayerController->PlayerInput;
	check(PlayerInput)

	OwnerCharacter = Cast<ACharacter>(GetOwner());
	check(OwnerCharacter);
	InputComp = Cast<UEnhancedInputComponent>(OwnerCharacter->InputComponent);

	InputData.InitialTransform = GetOwner()->GetActorTransform();
	InputData.Bindings.Add(MakeBindingsData());
}

void UTPInputRecordingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	InputData.Bindings.Add(MakeBindingsData());
}

FBindingsData UTPInputRecordingComponent::MakeBindingsData() const
{
	FBindingsData BindingsData;
	BindingsData.WorldTime = GetWorld()->TimeSeconds;
	for (int32 i = 0; i < InputComp->GetActionEventBindings().Num(); ++i)
	{
		FEnhancedInputActionEventBinding* Action = InputComp->GetActionEventBindings()[i].Get();
		const FName CurrentActionName = FName{Action->GetAction()->GetName()};

		FAxisData CurrentAxisData{};
		CurrentAxisData.Name = CurrentActionName;
		CurrentAxisData.Value = InputComp->BindActionValue(Action->GetAction()).GetValue().Get<FVector2D>();
		BindingsData.AxisValues.Add(CurrentAxisData);
	}

	return BindingsData;
}

FString UTPInputRecordingComponent::GenerateFileName() const
{
	return FPaths::GameSourceDir().Append("TestProject/Tests/Data/CharacterTestInput.json");
}

void UTPInputRecordingComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	JsonUtils::WriteInputData(GenerateFileName(), InputData);
}
