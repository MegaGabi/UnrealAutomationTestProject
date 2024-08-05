// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "TestProject/Tests/Utils/InputRecordingUtils.h"
#include "TestProject/Tests/Utils/BoneRecordingUtils.h"
#include "CoreMinimal.h"

namespace TestProject
{
	class JsonUtils
	{
	public:
		static bool WriteInputData(const FString& FileName, const FInputData& InputData);
		static bool ReadInputData(const FString& FileName, FInputData& InputData);
		static bool WriteSkeletonData(const FString& FileName, const FRecordingAnimationData& AnimationData);
		static bool ReadSkeletonData(const FString& FileName, FRecordingAnimationData& AnimationData);
	};
}
