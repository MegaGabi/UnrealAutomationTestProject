// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/TPTurret.h"
#include "TPProjectile.h"
#include "Components/StaticMeshComponent.h"

ATPTurret::ATPTurret()
{
	PrimaryActorTick.bCanEverTick = false;

	TurretMesh = CreateDefaultSubobject<UStaticMeshComponent>("TurretMesh");
	check(TurretMesh);

	SetRootComponent(TurretMesh);
}

void ATPTurret::BeginPlay()
{
	Super::BeginPlay();
	
	check(AmmoCount > 0);
	check(FireFrequency > 0.0f);

	const float FirstDelay = FireFrequency;
	GetWorldTimerManager().SetTimer(FireTimerHandle, this, &ATPTurret::OnFire, FireFrequency, true, FirstDelay);
}

void ATPTurret::OnFire()
{
	if (--AmmoCount == 0)
	{
		GetWorldTimerManager().ClearTimer(FireTimerHandle);
	}

	if (GetWorld())
	{
		const auto SocketTransform = TurretMesh->GetSocketTransform("Muzzle");
		auto ProjectileObj = GetWorld()->SpawnActorDeferred<ATPProjectile>(ProjectileClass, SocketTransform);
		if (ProjectileObj)
		{
			ProjectileObj->SetShotDirection(SocketTransform.GetRotation().GetForwardVector());
			ProjectileObj->FinishSpawning(SocketTransform);
		}
	}
}

