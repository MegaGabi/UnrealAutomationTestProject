#pragma once

#include "CoreMinimal.h"
#include "TPTypes.generated.h"

UENUM(BlueprintType)
enum class EInventoryItemType : uint8
{
	SPHERE =0,
	CUBE, 
	CYLINDER, 
	CONE
};

USTRUCT(BlueprintType)
struct FInventoryData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EInventoryItemType Type;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0"))
	int32 Score;

	FString ToString() const
	{
		return FString::Printf(TEXT("(Type=%i,Score=%i)"), Type, Score);
	}
};

USTRUCT(BlueprintType)
struct FHealthData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1.0"))
	float MaxHealth{100.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.1"))
	float HealModifier{ 10.0f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.1", Units="s"))
	float HealRate{ 0.5f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.1", Units = "s"))
	float LifeSpan{ 5.f };

	FString ToString() const
	{
		return FString::Printf(TEXT("(MaxHealth=%f,HealModifier=%f,HealRate=%f,LifeSpan=%f)"), 
			MaxHealth, HealModifier, HealRate, LifeSpan);
	}
};