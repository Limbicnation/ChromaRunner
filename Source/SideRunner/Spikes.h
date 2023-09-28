// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Spikes.generated.h"

UCLASS()
class SIDERUNNER_API ASpikes : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASpikes();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

    // Speed of spike movement, adjustable from Blueprints
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
    float Speed;

    // Maximum height offset the spikes should move up to from their initial position
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
    float MaxHeightOffset;

private:
    float InitialZ; // Store the initial Z position of the spikes
    int32 MovementDirection; // 1 means up, -1 means down
};
