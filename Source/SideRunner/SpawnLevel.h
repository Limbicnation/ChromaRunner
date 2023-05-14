#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpawnLevel.generated.h"

class ABaseLevel;

UCLASS()
class SIDERUNNER_API ASpawnLevel : public AActor
{
	GENERATED_BODY()
	
public:
	ASpawnLevel();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void SpawnLevel(bool IsFirst);

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

protected:
	APawn* Player;

	UPROPERTY(EditAnywhere)
	TSubclassOf<ABaseLevel> Level1;

	UPROPERTY(EditAnywhere)
	TSubclassOf<ABaseLevel> Level2;

	UPROPERTY(EditAnywhere)
	TSubclassOf<ABaseLevel> Level3;

	UPROPERTY(EditAnywhere)
	TSubclassOf<ABaseLevel> Level4;

	UPROPERTY(EditAnywhere)
	TSubclassOf<ABaseLevel> Level5;

	UPROPERTY(EditAnywhere)
	TSubclassOf<ABaseLevel> Level6;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level Spawning")
	int32 RandomLevel; // Randomize the spawning of the level

private:
	FVector SpawnLocation; // Characters Spawn location
	FRotator SpawnRotation; // Characters Spawn rotation
	FActorSpawnParameters SpawnInfo; // Characters info

	UPROPERTY()
	TArray<ABaseLevel*> LevelList;

};
