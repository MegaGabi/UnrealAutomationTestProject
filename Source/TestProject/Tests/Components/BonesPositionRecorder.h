// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TestProject/Tests/Utils/BoneRecordingUtils.h"
#include "BonesPositionRecorder.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TESTPROJECT_API UBonesPositionRecorder : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UBonesPositionRecorder();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
	UFUNCTION()
	void RecordSkeletonData();

	FString GenerateFileName() const;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
private:
	const USkeletalMeshComponent* SkeletalMeshComponent;
	const USkeletalMesh* SkeletalMesh;
	FTimerHandle SkeletonRecordTimer;

	FRecordingAnimationData AnimationData;
};
