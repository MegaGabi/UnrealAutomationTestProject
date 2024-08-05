// Fill out your copyright notice in the Description page of Project Settings.

#if WITH_AUTOMATION_TESTS && WITH_EDITOR

#include "Tests/AnimationTests.h"
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Tests/TestUtils.h"
#include "Utils/JsonUtils.h"
#include "Utils/BoneRecordingUtils.h"

using namespace TestProject;

namespace
{
	ACharacter* GetAnimTestCharacter(UWorld* World)
	{
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(World, ACharacter::StaticClass(), FoundActors);

		for (auto Actor : FoundActors)
		{
			if (Actor->GetActorNameOrLabel().Equals("BP_AnimationTestCharacter"))
			{
				return Cast<ACharacter>(Actor);
			}
		}
		return nullptr;
	}

	class FCompareAnimationToSavedData : public IAutomationLatentCommand
	{
	public:
		FCompareAnimationToSavedData(
			UWorld* InWorld, 
			FRecordingAnimationData InDataToCompareTo, 
			USkeletalMeshComponent* InSkeletalMeshComponent)
			:World(InWorld),
			DataToCompareTo(InDataToCompareTo),
			SkeletalMeshComponent(InSkeletalMeshComponent)
		{
		}

		virtual bool Update() override
		{
			if (!World) return true;

			UAnimInstance* AnimInstance = SkeletalMeshComponent->GetAnimInstance();
			if (!AnimInstance)
			{
				return true;
			}

			float CurrentTime = World->TimeSeconds;
			float RecordTime = DataToCompareTo.SkeletonRecordings[Index].WorldTime;

			if (FMath::IsNearlyEqual(CurrentTime, RecordTime, 0.005))
			{
				FReferenceSkeleton ReferenceSkeleton = SkeletalMeshComponent->SkeletalMesh->GetRefSkeleton();
				int32 BoneCount = ReferenceSkeleton.GetNum();

				for (int32 BoneIndex = 0; BoneIndex < BoneCount; ++BoneIndex)
				{
					FRecordingBoneData RecordingBoneData = DataToCompareTo.SkeletonRecordings[Index].BoneValues[BoneIndex];

					FName BoneName = ReferenceSkeleton.GetBoneName(BoneIndex);
					FTransform BoneTransform = SkeletalMeshComponent->GetBoneTransform(BoneIndex);

					if (RecordingBoneData.Name.IsEqual(BoneName))
					{
						FString ErrorMessage = FString::Printf(TEXT("On animation time %s in bone %s "), 
							*FString::SanitizeFloat(DataToCompareTo.SkeletonRecordings[Index].WorldTime),
							*BoneName.ToString());
						if (!RecordingBoneData.Position.Equals(BoneTransform.GetLocation(), 30.f))
						{
							FString ErrorPositionMessage = FString::Printf(TEXT("%s recorded position %s is not equal actual %s"),
								*ErrorMessage, *RecordingBoneData.Position.ToString(), *BoneTransform.GetLocation().ToString());
							UE_LOG(LogTemp, Error, TEXT("%s"), *ErrorPositionMessage);
						}
						if (!RecordingBoneData.Rotation.Equals(BoneTransform.Rotator(), 30.f))
						{
							FString ErrorRotationMessage = FString::Printf(TEXT("%s recorded rotation %s is not equal actual %s"),
								*ErrorMessage, *RecordingBoneData.Rotation.ToString(), *BoneTransform.Rotator().ToString());
							UE_LOG(LogTemp, Error, TEXT("%s"), *ErrorRotationMessage);
						}
					}
				}

				if (++Index >= DataToCompareTo.SkeletonRecordings.Num()) return true;
			}
			else if (CurrentTime > RecordTime)
			{
				if (++Index >= DataToCompareTo.SkeletonRecordings.Num()) return true;
			}
			return false;
		}

	private:
		const UWorld* World;
		FRecordingAnimationData DataToCompareTo;
		USkeletalMeshComponent* SkeletalMeshComponent;
		int32 Index{ 0 };
		float WorldStartTime{ 0.0f };
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWalkAnimationIsCorrect, "TestProject.Animation.WalkAnimationIsCorrect",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority);

bool FWalkAnimationIsCorrect::RunTest(const FString& Parameters)
{
	const auto Level = LevelScope("/Game/Tests/AnimationTestLevelMap");

	UWorld* World = GetTestGameWorld();
	if (!TestNotNull(TEXT("World exists"), World)) return false;

	ACharacter* AnimTestChar = GetAnimTestCharacter(World);
	if (!TestNotNull("Character for animation testing is not found", AnimTestChar)) return false;

	USkeletalMeshComponent* SkeletalMeshComponent = AnimTestChar->GetMesh();
	if (!TestNotNull("Character for animation test has skeletal mesh component", SkeletalMeshComponent)) return false;

	USkeletalMesh* SkeletalMesh = AnimTestChar->GetMesh()->SkeletalMesh;
	if (!TestNotNull("Character for animation test has skeletal mesh", SkeletalMesh)) return false;

	FRecordingAnimationData AnimationData;
	JsonUtils::ReadSkeletonData(FPaths::GameSourceDir().Append("TestProject/Tests/Data/AnimationTestData.json"), AnimationData);

	AnimTestChar->SetActorTransform(AnimationData.InitialTransform);

	ADD_LATENT_AUTOMATION_COMMAND(FCompareAnimationToSavedData(World, AnimationData, SkeletalMeshComponent));
	//AnimTestChar->GetMesh()->GetAnimInstance()->OnPlayMontageNotifyBegin.AddDynamic(TEXT("L"),

	//FInputData InputData;
	//JsonUtils::ReadInputData(FPaths::GameSourceDir().Append("TestProject/Tests/Data/ItemTestMoveData.json"), InputData);

	return true;
}

#endif