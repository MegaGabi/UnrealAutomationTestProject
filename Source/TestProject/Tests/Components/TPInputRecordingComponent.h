// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TestProject/Tests/Utils/InputRecordingUtils.h"
#include "TPInputRecordingComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TESTPROJECT_API UTPInputRecordingComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UTPInputRecordingComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	ACharacter* OwnerCharacter;
	UEnhancedInputComponent* InputComp;
	UPlayerInput* PlayerInput;

	FInputData InputData;

	FBindingsData MakeBindingsData() const;
	FString GenerateFileName() const;
};
