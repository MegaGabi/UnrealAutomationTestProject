// Fill out your copyright notice in the Description page of Project Settings.


#include "Tests/Utils/JsonUtils.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonWriter.h"
#include "JsonObjectConverter.h"
#include "Misc/FileHelper.h"

namespace TestProject
{
	bool JsonUtils::WriteInputData(const FString& FileName, const FInputData& InputData)
	{
		TSharedPtr<FJsonObject> MainJsonObject = FJsonObjectConverter::UStructToJsonObject(InputData);
		if (!MainJsonObject.IsValid()) return false;

		FString OutputString;
		TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&OutputString);

		if (!FJsonSerializer::Serialize(MainJsonObject.ToSharedRef(), JsonWriter)) return false;
		if (!JsonWriter->Close()) return false;

		if (!FFileHelper::SaveStringToFile(OutputString, *FileName)) return false;

		return true;
	}

	bool JsonUtils::ReadInputData(const FString& FileName, FInputData& InputData)
	{
		TSharedPtr<FJsonObject> MainJsonObject = MakeShareable(new FJsonObject());

		FString FileContentString;
		if (!FFileHelper::LoadFileToString(FileContentString, *FileName)) return false;

		TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(FileContentString);
		if (!FJsonSerializer::Deserialize(JsonReader, MainJsonObject)) return false;

		return FJsonObjectConverter::JsonObjectToUStruct(MainJsonObject.ToSharedRef(), &InputData, 0, 0);
	}
	
	bool JsonUtils::WriteSkeletonData(const FString& FileName, const FRecordingAnimationData& AnimationData)
	{
		TSharedPtr<FJsonObject> MainJsonObject = FJsonObjectConverter::UStructToJsonObject(AnimationData);
		if (!MainJsonObject.IsValid()) return false;
		
		FString OutputString;
		TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&OutputString);

		if (!FJsonSerializer::Serialize(MainJsonObject.ToSharedRef(), JsonWriter)) return false;
		if (!JsonWriter->Close()) return false;

		if (!FFileHelper::SaveStringToFile(OutputString, *FileName)) return false;

		return true;
	}

	bool JsonUtils::ReadSkeletonData(const FString& FileName, FRecordingAnimationData& AnimationData)
	{
		TSharedPtr<FJsonObject> MainJsonObject = MakeShareable(new FJsonObject());

		FString FileContentString;
		if (!FFileHelper::LoadFileToString(FileContentString, *FileName)) return false;

		TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(FileContentString);
		if (!FJsonSerializer::Deserialize(JsonReader, MainJsonObject)) return false;

		return FJsonObjectConverter::JsonObjectToUStruct(MainJsonObject.ToSharedRef(), &AnimationData, 0, 0);
	}
}
