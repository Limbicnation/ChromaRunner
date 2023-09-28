// Fill out your copyright notice in the Description page of Project Settings.

#include "Spikes.h"

// Sets default values
ASpikes::ASpikes()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    // Initialize default values
    Speed = 100.0f; // Adjust this value as needed
    MaxHeightOffset = 100.0f; // Default value for the height offset
    MovementDirection = 1; // Start moving up by default
}

// Called when the game starts or when spawned
void ASpikes::BeginPlay()
{
    Super::BeginPlay();
    InitialZ = GetActorLocation().Z;
}

// Called every frame
void ASpikes::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    FVector NewLocation = GetActorLocation();
    NewLocation.Z += Speed * DeltaTime * MovementDirection;

    if (MovementDirection == 1 && NewLocation.Z >= InitialZ + MaxHeightOffset)
    {
        MovementDirection = -1; // Switch to moving down
    }
    else if (MovementDirection == -1 && NewLocation.Z <= InitialZ)
    {
        MovementDirection = 1; // Switch to moving up
    }

    SetActorLocation(NewLocation);
}
