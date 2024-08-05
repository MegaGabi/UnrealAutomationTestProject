// Fill out your copyright notice in the Description page of Project Settings.


#include "Tests/Components/BonesPositionRecorder.h"
#include "GameFramework/Character.h"
#include "Tests/Utils/JsonUtils.h"

using namespace TestProject;

UBonesPositionRecorder::UBonesPositionRecorder()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UBonesPositionRecorder::BeginPlay()
{
	Super::BeginPlay();

    check(GetOwner());
	
    ACharacter* Character = Cast<ACharacter>(GetOwner());

    check(Character);
    check(Character->GetMesh());

    SkeletalMeshComponent = Character->GetMesh();
    SkeletalMesh = Character->GetMesh()->SkeletalMesh;

    AnimationData.InitialTransform = GetOwner()->GetActorTransform();

    GetWorld()->GetTimerManager().SetTimer(SkeletonRecordTimer, this, &UBonesPositionRecorder::RecordSkeletonData, 0.1f, true);
}

void UBonesPositionRecorder::RecordSkeletonData()
{
    if (!SkeletalMesh)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid SkeletalMesh"));
        return;
    }

    FReferenceSkeleton ReferenceSkeleton = SkeletalMesh->GetRefSkeleton();
    int32 BoneCount = ReferenceSkeleton.GetNum();

    UAnimInstance* AnimInstance = SkeletalMeshComponent->GetAnimInstance();
    if (!AnimInstance)
    {
        return;
    }

    FRecordingSkeletonData SkeletonData;

    SkeletonData.WorldTime = GetWorld()->TimeSeconds;
    for (int32 BoneIndex = 0; BoneIndex < BoneCount; ++BoneIndex)
    {
        FName BoneName = ReferenceSkeleton.GetBoneName(BoneIndex);
        FTransform BoneTransform = SkeletalMeshComponent->GetBoneTransform(BoneIndex);

        FRecordingBoneData BoneData;

        BoneData.Name = BoneName;
        BoneData.Position = BoneTransform.GetLocation();
        BoneData.Rotation = BoneTransform.Rotator();

        SkeletonData.BoneValues.Add(BoneData);
    }
    AnimationData.SkeletonRecordings.Add(SkeletonData);
}

FString UBonesPositionRecorder::GenerateFileName() const
{
    return FPaths::GameSourceDir().Append("TestProject/Tests/Data/AnimationTestData.json");
}

void UBonesPositionRecorder::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    JsonUtils::WriteSkeletonData(GenerateFileName(), AnimationData);
}