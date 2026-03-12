#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DifficultyScaler.generated.h"

/** Difficulty scaling constants for the default dual-slope ramp. */
namespace DifficultyConstants
{
    /** Distance at which Phase 1 ends and Phase 2 begins (meters). */
    constexpr float PHASE1_END_DISTANCE = 5000.0f;

    /** Length of Phase 2 ramp (meters from PHASE1_END_DISTANCE to max difficulty). */
    constexpr float PHASE2_RAMP_LENGTH = 15000.0f;

    /** Difficulty at distance 0 (easiest). */
    constexpr float MIN_DIFFICULTY = 1.0f;

    /** Difficulty at PHASE1_END_DISTANCE (midpoint). */
    constexpr float MID_DIFFICULTY = 5.0f;

    /** Maximum difficulty (hardest). */
    constexpr float MAX_DIFFICULTY = 10.0f;
}

/**
 * Stateless utility for mapping player distance to difficulty level.
 * Produces a value from 1.0 (easy) to 10.0 (hardest).
 *
 * Curve: linear ramp from 1.0 at 0m to 5.0 at 5000m,
 * then slower ramp to 10.0 at 20000m.
 *
 * Optionally overridden via a designer-authored UCurveFloat.
 */
UCLASS(BlueprintType, Blueprintable)
class SIDERUNNER_API UDifficultyScaler : public UObject
{
    GENERATED_BODY()

public:
    UDifficultyScaler();

    /**
     * Returns the difficulty level for the given distance.
     *
     * @param DistanceMeters - Player distance in meters
     * @return Difficulty between 1.0 and 10.0
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Difficulty")
    float GetDifficultyAtDistance(float DistanceMeters) const;

    /**
     * Returns difficulty mapped to a 0..1 alpha for use with FMath::Lerp.
     *
     * @param DistanceMeters - Player distance in meters
     * @return Alpha in range [0.0, 1.0]
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Difficulty")
    float GetDifficultyAlpha(float DistanceMeters) const;

    /** Optional designer-authored curve override. X = meters, Y = difficulty (1-10). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty")
    class UCurveFloat* DifficultyOverrideCurve;

private:
    /** Default built-in difficulty calculation (dual-slope linear ramp). */
    float CalculateDefaultDifficulty(float DistanceMeters) const;
};
