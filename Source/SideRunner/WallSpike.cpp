// Fill out your copyright notice in the Description page of Project Settings.


#include "WallSpike.h"

AWallSpike::AWallSpike()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AWallSpike::BeginPlay()
{
	Super::BeginPlay();
	// Set the speed of the Actor in the Y direction
	this->GetRootComponent()->ComponentVelocity = FVector(0,25,0);
	
}

void AWallSpike::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//Move Actor in Y direction by getting the current location and adding the Y value
	SetActorLocation(GetActorLocation() + FVector(0, 350 * DeltaTime, 0), true);
}
