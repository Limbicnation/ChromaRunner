#include "DifficultyScaler.h"
#include "Curves/CurveFloat.h"
#include "SideRunner.h" // Custom log categories

UDifficultyScaler::UDifficultyScaler()
    : DifficultyOverrideCurve(nullptr)
{
}

float UDifficultyScaler::GetDifficultyAtDistance(float DistanceMeters) const
{
    // Use designer curve if provided
    if (DifficultyOverrideCurve)
    {
        const float CurveValue = DifficultyOverrideCurve->GetFloatValue(DistanceMeters);
        return FMath::Clamp(CurveValue, DifficultyConstants::MIN_DIFFICULTY, DifficultyConstants::MAX_DIFFICULTY);
    }

    return CalculateDefaultDifficulty(DistanceMeters);
}

float UDifficultyScaler::GetDifficultyAlpha(float DistanceMeters) const
{
    const float Difficulty = GetDifficultyAtDistance(DistanceMeters);
    // Map 1..10 → 0..1
    return (Difficulty - 1.0f) / 9.0f;
}

float UDifficultyScaler::CalculateDefaultDifficulty(float DistanceMeters) const
{
    // Clamp to non-negative
    DistanceMeters = FMath::Max(0.0f, DistanceMeters);

    float Difficulty;

    if (DistanceMeters <= DifficultyConstants::PHASE1_END_DISTANCE)
    {
        // Phase 1: Linear ramp from MIN to MID over PHASE1_END_DISTANCE
        const float Alpha = DistanceMeters / DifficultyConstants::PHASE1_END_DISTANCE;
        Difficulty = FMath::Lerp(DifficultyConstants::MIN_DIFFICULTY, DifficultyConstants::MID_DIFFICULTY, Alpha);
    }
    else
    {
        // Phase 2: Slower ramp from MID to MAX over PHASE2_RAMP_LENGTH
        const float Alpha = FMath::Min((DistanceMeters - DifficultyConstants::PHASE1_END_DISTANCE) / DifficultyConstants::PHASE2_RAMP_LENGTH, 1.0f);
        Difficulty = FMath::Lerp(DifficultyConstants::MID_DIFFICULTY, DifficultyConstants::MAX_DIFFICULTY, Alpha);
    }

    return FMath::Clamp(Difficulty, DifficultyConstants::MIN_DIFFICULTY, DifficultyConstants::MAX_DIFFICULTY);
}
