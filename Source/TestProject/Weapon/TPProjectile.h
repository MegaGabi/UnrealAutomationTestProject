// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TPProjectile.generated.h"

UCLASS(Abstract)
class TESTPROJECT_API ATPProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	ATPProjectile();

	void SetShotDirection(const FVector& Direction);

protected:
	virtual void BeginPlay() override;
	
	UFUNCTION()
	void OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class USphereComponent* CollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UProjectileMovementComponent* MovementComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float DamageAmount{ 30.0f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (Units = s))
	float LifeSeconds{ 5.0f };

private:
	FVector ShotDirection;
};
