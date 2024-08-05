#pragma once

#include "CoreMinimal.h"
#include "BoneRecordingUtils.generated.h"

USTRUCT()
struct FRecordingBoneData
{
	GENERATED_BODY()

	UPROPERTY()
	FName Name;

	UPROPERTY()
	FVector Position;

	UPROPERTY()
	FRotator Rotation;
};

USTRUCT()
struct FRecordingSkeletonData
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FRecordingBoneData> BoneValues;

	UPROPERTY()
	float WorldTime{ 0.0f };
};

USTRUCT()
struct FRecordingAnimationData
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FRecordingSkeletonData> SkeletonRecordings;

	UPROPERTY()
	FTransform InitialTransform;
};