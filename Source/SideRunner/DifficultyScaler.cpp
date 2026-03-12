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
        return FMath::Clamp(CurveValue, 1.0f, 10.0f);
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

    if (DistanceMeters <= 5000.0f)
    {
        // Phase 1: Linear ramp from 1.0 at 0m to 5.0 at 5000m
        const float Alpha = DistanceMeters / 5000.0f;
        Difficulty = FMath::Lerp(1.0f, 5.0f, Alpha);
    }
    else
    {
        // Phase 2: Slower ramp from 5.0 at 5000m to 10.0 at 20000m
        const float Alpha = FMath::Min((DistanceMeters - 5000.0f) / 15000.0f, 1.0f);
        Difficulty = FMath::Lerp(5.0f, 10.0f, Alpha);
    }

    return FMath::Clamp(Difficulty, 1.0f, 10.0f);
}
