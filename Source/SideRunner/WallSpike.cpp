// Fill out your copyright notice in the Description page of Project Settings.

#include "WallSpike.h"

AWallSpike::AWallSpike()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AWallSpike::BeginPlay()
{
	Super::BeginPlay();
	// Any initial setup can go here. The movement logic is now handled in Tick.
}

void AWallSpike::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//Move Actor in Y direction by getting the current location and adding the Y value
	FVector NewLocation = GetActorLocation() + FVector(0, 350 * DeltaTime, 0);
	SetActorLocation(NewLocation, true);
}
