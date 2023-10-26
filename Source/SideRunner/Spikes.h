// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Sound/SoundBase.h"
#include "Components/PrimitiveComponent.h" // If required for UPrimitiveComponent
#include "Spikes.generated.h"

UCLASS()
class SIDERUNNER_API ASpikes : public AActor
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    ASpikes();

    // Called every frame
    virtual void Tick(float DeltaTime) override;

    // Speed of spike movement, adjustable from Blueprints
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float Speed;

    // Maximum height offset the spikes should move up to from their initial position
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float MaxHeightOffset;

    // Sound to play when the player collides with the spikes
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    USoundBase* CollisionSound;

    // Function to handle collision with the spikes
    virtual void NotifyHit(
        UPrimitiveComponent* MyComp,
        AActor* Other,
        UPrimitiveComponent* OtherComp,
        bool bSelfMoved,
        FVector HitLocation,
        FVector HitNormal,
        FVector NormalImpulse,
        const FHitResult& Hit
    ) override;

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

private:
    float InitialZ; // Store the initial Z position of the spikes
    int32 MovementDirection; // 1 means up, -1 means down
};
