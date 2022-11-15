// Fill out your copyright notice in the Description page of Project Settings.


#include "SpawnLevel.h"

#include "BaseLevel.h"
#include "Engine.h"
#include "Components/BoxComponent.h"


// Sets default values
ASpawnLevel::ASpawnLevel()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ASpawnLevel::BeginPlay()
{
	Super::BeginPlay();

	// Spawn the first level and set it to true
	SpawnLevel(true);
	SpawnLevel(false);
	SpawnLevel(false);
	SpawnLevel(false);
	
}

// Called every frame
void ASpawnLevel::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ASpawnLevel::SpawnLevel(bool IsFirst)
{
	// the main magic that will happen in our level, will happen in here!

	SpawnLocation = FVector(0.0f, 1000.0f, 0.0f);
	SpawnRotation = FRotator(0,90,0);

	// if it'S not first level that we are spawning, then we are going to get the last level in out array
	if (!IsFirst)
	{
		ABaseLevel* LastLevel = LevelList.Last();
		// get the location from the last level to know where to spawn the next level
		SpawnLocation = LastLevel->GetSpawnLocation()->GetComponentTransform().GetTranslation();
	}
	// Randomize the values for the list of levels spawned
	RandomLevel = FMath::RandRange(1,6);
	ABaseLevel* NewLevel = nullptr;

	if (RandomLevel == 1)
	{
		NewLevel = GetWorld()->SpawnActor<ABaseLevel>(Level1,
			SpawnLocation, SpawnRotation, SpawnInfo);
	}
	else if (RandomLevel == 2)
	{
		NewLevel = GetWorld()->SpawnActor<ABaseLevel>(Level2,
			SpawnLocation, SpawnRotation, SpawnInfo);
	}
	else if (RandomLevel == 3)
	{
		NewLevel = GetWorld()->SpawnActor<ABaseLevel>(Level3,
			SpawnLocation, SpawnRotation, SpawnInfo);
	}
	else if (RandomLevel == 4)
	{
		NewLevel = GetWorld()->SpawnActor<ABaseLevel>(Level4,
			SpawnLocation, SpawnRotation, SpawnInfo);
	}
	else if (RandomLevel == 5)
	{
		NewLevel = GetWorld()->SpawnActor<ABaseLevel>(Level5,
			SpawnLocation, SpawnRotation, SpawnInfo);
	}
	else if (RandomLevel == 6)
	{
		NewLevel = GetWorld()->SpawnActor<ABaseLevel>(Level6,
			SpawnLocation, SpawnRotation, SpawnInfo);
	}
	// Add New Level to the level list
	if (NewLevel)
	{
		if(NewLevel->GetTrigger())
		{
			NewLevel->GetTrigger()->OnComponentBeginOverlap.AddDynamic(this, &ASpawnLevel::OnOverlapBegin);
		}
	}

	// Remove Levels at index 0 that exceed the number 5
	LevelList.Add(NewLevel);
	if(LevelList.Num() > 5)
	{
		LevelList.Remove(0);
	}
}

void ASpawnLevel::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	SpawnLevel(false);
}

