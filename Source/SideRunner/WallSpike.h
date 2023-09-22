// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Spikes.h"
#include "WallSpike.generated.h"

/**
 * 
 */
UCLASS()
class SIDERUNNER_API AWallSpike : public ASpikes
{
	GENERATED_BODY()

public:

	AWallSpike();

	// If you want the WallSpike to have the Tick functionality, ensure you also set PrimaryActorTick.bCanEverTick = true; in the constructor.
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;
};
