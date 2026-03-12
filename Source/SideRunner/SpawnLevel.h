#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpawnLevel.generated.h"

class ABaseLevel;
class UProceduralLevelBuilder;
class UDifficultyScaler;
class USideRunnerGameInstance;

UCLASS()
class SIDERUNNER_API ASpawnLevel : public AActor
{
    GENERATED_BODY()
    
public:
    ASpawnLevel();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION()
    void SpawnLevel(bool IsFirst);

    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void DestroyOldestLevel(); // Marked as UFUNCTION to expose it to the engine

    /** Reset all spawned levels and respawn fresh levels around player position */
    UFUNCTION(BlueprintCallable, Category="Level Management")
    void ResetLevelsForRespawn();

protected:
    /** Weak reference to player pawn - handles pawn respawn/death correctly */
    TWeakObjectPtr<APawn> PlayerWeakPtr;

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

    // ======================================================================
    // Procedural Generation
    // ======================================================================

    /** When true, levels are generated procedurally instead of using BP variants. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Procedural Generation")
    bool bUseProceduralGeneration = false;

    /** Procedural content builder component. Created in constructor. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Procedural Generation")
    UProceduralLevelBuilder* ProceduralBuilder;

    /** Difficulty scaler instance. Created in constructor. */
    UPROPERTY()
    UDifficultyScaler* DifficultyScaler;

    /** Maximum number of active levels before oldest is destroyed. Replaces hardcoded 6. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Level Management", meta=(ClampMin="3", ClampMax="12"))
    int32 MaxActiveLevels = 6;

    /** Distance (meters) at which procedural generation begins in hybrid mode.
     *  Before this distance, handcrafted BP_Level1-6 are used. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Procedural Generation", meta=(ClampMin="0.0", ClampMax="10000.0"))
    float ProceduralStartDistance = 2000.0f;

private:
    FVector SpawnLocation;
    FRotator SpawnRotation;

    UPROPERTY()
    TArray<ABaseLevel*> LevelList;

    void SpawnInitialLevels(const FVector& StartPosition = FVector(0.0f, 1000.0f, 0.0f));
    void DelayedDestroyOldestLevel();
    void TryAcquirePlayerPawn();

    /** Procedural spawn path: spawn a bare ABaseLevel and fill with generated content. */
    void SpawnProceduralLevel(const FVector& SpawnPos, const FRotator& SpawnRot);

    /** Handcrafted spawn path: pick a random BP_Level1-6. */
    void SpawnHandcraftedLevel(const FVector& SpawnPos, const FRotator& SpawnRot);

    /** Returns current player distance in meters for difficulty calculation. */
    float GetCurrentDistanceMeters() const;

    /** Should we use procedural generation at the current distance? (hybrid mode check) */
    bool ShouldUseProceduralAtCurrentDistance() const;

    /** Cached first-level spawn position - updated on each spawn cycle to match player location */
    FVector FirstLevelSpawnPosition = FVector(0.0f, 1000.0f, 0.0f);

    /** Array of timer handles for pending destroy operations */
    TArray<FTimerHandle> PendingDestroyTimers;

    /** Current seed for procedural generation (incremented per chunk). */
    int32 CurrentSeed = 0;

    /** Cached game instance for distance queries. */
    UPROPERTY()
    USideRunnerGameInstance* CachedGameInstance;
};
