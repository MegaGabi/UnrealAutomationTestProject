// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TPTurret.generated.h"

UCLASS(Abstract)
class TESTPROJECT_API ATPTurret : public AActor
{
	GENERATED_BODY()
	
public:	
	ATPTurret();

protected:
	virtual void BeginPlay() override;
	
private:
	void OnFire();
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UStaticMeshComponent* TurretMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	int32 AmmoCount{ 10 };

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta=(Units=s))
	float FireFrequency{ 1.0f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSubclassOf<class ATPProjectile> ProjectileClass;

private:
	FTimerHandle FireTimerHandle;
};
