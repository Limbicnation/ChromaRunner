#pragma once

#include "CoreMinimal.h"
#include "EndlessRunnerTypes.generated.h"

/**
 * Placement data for a single platform in a procedurally generated chunk.
 */
USTRUCT(BlueprintType)
struct FPlatformPlacement
{
    GENERATED_BODY()

    /** Y-axis position (forward direction) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement")
    float YPosition = 0.0f;

    /** Z-axis position (height) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement")
    float ZPosition = 0.0f;

    /** Platform width along Y-axis */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement")
    float Width = 300.0f;

    /** Platform length along Y-axis */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement")
    float Length = 200.0f;

    /** Whether this platform moves */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement")
    bool bIsMoving = false;

    /** Whether a collectible should be spawned above this platform */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement")
    bool bHasCollectible = false;
};

/**
 * Configuration for a single procedural level chunk.
 */
USTRUCT(BlueprintType)
struct FProceduralLevelConfig
{
    GENERATED_BODY()

    /** Length of this chunk along the Y-axis (Unreal units) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    float ChunkLength = 2000.0f;

    /** Random seed for deterministic generation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    int32 Seed = 0;

    /** Difficulty level (1.0 to 10.0) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    float Difficulty = 1.0f;
};

/**
 * Types of procedural features that can be generated.
 */
UENUM(BlueprintType)
enum class EProceduralFeature : uint8
{
    StaticPlatform   UMETA(DisplayName = "Static Platform"),
    MovingPlatform   UMETA(DisplayName = "Moving Platform"),
    GapChallenge     UMETA(DisplayName = "Gap Challenge"),
    CoinRun          UMETA(DisplayName = "Coin Run")
};
