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

    UFUNCTION()
    void DestroyOldestLevel(); // Marked as UFUNCTION to expose it to the engine

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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Level Management")
    float LevelDestroyDelay = 1.0f; // Delay before destroying the oldest level

private:
    FVector SpawnLocation;
    FRotator SpawnRotation;

    UPROPERTY()
    TArray<ABaseLevel*> LevelList;

    void SpawnInitialLevels();
    void DelayedDestroyOldestLevel();

    FTimerHandle DestroyTimerHandle; // Add a timer handle for the destruction delay
};
