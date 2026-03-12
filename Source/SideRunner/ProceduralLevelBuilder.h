#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EndlessRunnerTypes.h"
#include "ActorPool.h"
#include "ProceduralLevelBuilder.generated.h"

class ASpikes;
class ACoinPickup;
class ASimpleEnemy;

/**
 * Core procedural content generation component for ChromaRunner.
 * Attached to ASpawnLevel. Generates platforms, obstacles, and coins
 * using a controlled random walk algorithm with difficulty-driven parameters.
 *
 * PERFORMANCE: Uses FRandomStream for deterministic generation, FActorPool for reuse.
 * Follows UE5 skill guidelines: no tick, cached references, UPROPERTY on all UObject*.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SIDERUNNER_API UProceduralLevelBuilder : public UActorComponent
{
    GENERATED_BODY()

public:
    UProceduralLevelBuilder();

    // ======================================================================
    // Core Generation API
    // ======================================================================

    /**
     * Generates all content for a single level chunk.
     *
     * @param World - World context for spawning actors
     * @param StartY - Y-axis start position for this chunk
     * @param Difficulty - Difficulty level (1.0 to 10.0)
     * @param Seed - Random seed for deterministic generation
     * @return Array of spawned actors (attached to parent level)
     */
    UFUNCTION(BlueprintCallable, Category = "Procedural Generation")
    TArray<AActor*> GenerateLevelContent(UWorld* World, float StartY, float Difficulty, int32 Seed);

    /**
     * Returns spawned actors to pools for reuse. Call before destroying a level.
     *
     * @param Actors - Array of actors to return to pools
     */
    UFUNCTION(BlueprintCallable, Category = "Procedural Generation")
    void ReturnActorsToPool(const TArray<AActor*>& Actors);

    /** Clears all object pools. */
    UFUNCTION(BlueprintCallable, Category = "Procedural Generation")
    void ClearPools();

    // ======================================================================
    // Platform Configuration
    // ======================================================================

    /** Static mesh actor class used for ground platform segments. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform Config")
    TSubclassOf<AActor> PlatformClass;

    /** Array of platform mesh variants for visual variety. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform Config")
    TArray<TSubclassOf<AActor>> PlatformVariants;

    /** Y-axis span per generated chunk (Unreal units). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform Config", meta = (ClampMin = "500.0", ClampMax = "5000.0"))
    float ChunkLength;

    /** Minimum platform width (at highest difficulty). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform Config", meta = (ClampMin = "100.0", ClampMax = "500.0"))
    float MinPlatformWidth;

    /** Maximum platform width (at lowest difficulty). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform Config", meta = (ClampMin = "200.0", ClampMax = "800.0"))
    float MaxPlatformWidth;

    /** Minimum gap between platforms. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform Config", meta = (ClampMin = "50.0", ClampMax = "500.0"))
    float MinGapSize;

    /** Maximum gap between platforms (must be < MaxJumpDistance). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform Config", meta = (ClampMin = "100.0", ClampMax = "800.0"))
    float MaxGapSize;

    /** Base platform mesh size in Unreal units (used for scale calculation). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform Config", meta = (ClampMin = "1.0", ClampMax = "1000.0"))
    float BasePlatformMeshSize;

    // ======================================================================
    // Obstacle Configuration
    // ======================================================================

    /** Spike obstacle variants to place on platforms. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle Config")
    TArray<TSubclassOf<ASpikes>> ObstacleClasses;

    /** Coin pickup class for collectible generation. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle Config")
    TSubclassOf<ACoinPickup> CoinClass;

    /** Enemy class for spawning on platforms at high difficulty. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle Config")
    TSubclassOf<ASimpleEnemy> EnemyClass;

    /** Wall spike class for rare high-difficulty events. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle Config")
    TSubclassOf<AActor> WallSpikeClass;

    // ======================================================================
    // Jump Physics Constraints (used for gap validation)
    // ======================================================================

    /** Character jump Z velocity — from RunnerCharacter constructor. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Constraints")
    float JumpZVelocity;

    /** Double jump Z velocity — from RunnerCharacter. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Constraints")
    float DoubleJumpZVelocity;

    /** Gravity scale — from RunnerCharacter. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Constraints")
    float GravityScale;

    /** Maximum walk speed — from RunnerCharacter. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Constraints")
    float MaxWalkSpeed;

    /** Maximum single-jump horizontal distance (computed from physics). */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics Constraints")
    float MaxSingleJumpDistance;

    /** Maximum double-jump horizontal distance (computed from physics). */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics Constraints")
    float MaxDoubleJumpDistance;

private:
    // ======================================================================
    // Generation Helpers
    // ======================================================================

    /** Spawns platforms along the chunk using controlled random walk. */
    void GeneratePlatforms(UWorld* World, float StartY, float Difficulty, FRandomStream& RandomStream,
                           TArray<AActor*>& OutActors, TArray<FPlatformPlacement>& OutPlacements);

    /** Spawns obstacles on platforms based on difficulty. */
    void GenerateObstacles(UWorld* World, float Difficulty, FRandomStream& RandomStream,
                           const TArray<FPlatformPlacement>& Placements, TArray<AActor*>& OutActors);

    /** Spawns coins above platforms. */
    void GenerateCoins(UWorld* World, float Difficulty, FRandomStream& RandomStream,
                       const TArray<FPlatformPlacement>& Placements, TArray<AActor*>& OutActors);

    /** Computes max jump distances from physics constants. */
    void CalculateJumpDistances();

    /**
     * Retrieves an actor from the specified pool, or spawns a new one if the pool is empty.
     * Handles GC ref array bookkeeping and actor reactivation.
     *
     * @param Pool - Actor pool to check
     * @param GCRefs - GC root reference array for this pool
     * @param World - World context for spawning
     * @param ActorClass - Class to spawn if pool is empty
     * @param SpawnLocation - Location for the actor
     * @return Retrieved or newly spawned actor, or nullptr on failure
     */
    AActor* GetOrSpawnActor(FActorPool<AActor>& Pool, TArray<AActor*>& GCRefs,
                            UWorld* World, UClass* ActorClass, const FVector& SpawnLocation);

    /** Returns a difficulty alpha in [0,1] from difficulty [1,10]. */
    FORCEINLINE float GetDifficultyAlpha(float Difficulty) const
    {
        return FMath::Clamp((Difficulty - 1.0f) / 9.0f, 0.0f, 1.0f);
    }

    /** Selects an obstacle movement type appropriate for the difficulty. */
    uint8 SelectMovementTypeForDifficulty(float Difficulty, FRandomStream& RandomStream) const;

    /** Returns true if the actor matches PlatformClass or any PlatformVariant class. */
    bool IsPlatformActor(const AActor* Actor) const;

    // ======================================================================
    // Object Pools
    // ======================================================================

    FActorPool<AActor> PlatformPool;
    FActorPool<AActor> ObstaclePool;
    FActorPool<AActor> CoinPool;

    // GC roots: mirror arrays keep pooled actors referenced so UE GC doesn't collect them
    // while they sit in FActorPool (which uses raw pointers outside UPROPERTY).
    UPROPERTY()
    TArray<AActor*> PlatformPoolGCRefs;

    UPROPERTY()
    TArray<AActor*> ObstaclePoolGCRefs;

    UPROPERTY()
    TArray<AActor*> CoinPoolGCRefs;

    /** Base ground Z-level. */
    static constexpr float BaseGroundZ = 0.0f;

    /** Height above platform to place coins. */
    static constexpr float CoinHeightOffset = 150.0f;

    /** Wall spike spawn chance per chunk at difficulty 5+. */
    static constexpr float WallSpikeChancePerChunk = 0.05f;
};
