#include "SideRunnerGameInstance.h"
#include "Engine/Engine.h"

void USideRunnerGameInstance::Init()
{
    Super::Init();

    // Initialize scoring state
    CurrentScore = 0;
    DistanceTraveled = 0.0f;
    HighScore = 0;
    LastRecordedX = 0.0f;
    bGameEnded = false;

    // Set default win distance
    WinDistance = SideRunnerGameInstanceConstants::DEFAULT_WIN_DISTANCE;

    UE_LOG(LogTemp, Log, TEXT("SideRunnerGameInstance initialized - Win distance: %.1f meters"), WinDistance);
}

void USideRunnerGameInstance::UpdateDistanceScore(float PlayerXPosition)
{
    // PERFORMANCE: Early exit if game has ended
    if (bGameEnded)
    {
        return;
    }

    // PERFORMANCE: Only count forward progress (positive X movement)
    if (PlayerXPosition > LastRecordedX)
    {
        // Calculate distance delta
        const float DeltaDistance = PlayerXPosition - LastRecordedX;
        DistanceTraveled += DeltaDistance;

        // PERFORMANCE: Convert distance to points (1 meter = 1 point)
        const int32 DistancePoints = ConvertDistanceToPoints(DeltaDistance);

        if (DistancePoints > 0)
        {
            CurrentScore += DistancePoints;
            OnScoreUpdated.Broadcast(CurrentScore);

#if UE_BUILD_DEVELOPMENT
            UE_LOG(LogTemp, VeryVerbose, TEXT("Distance score updated: +%d points | Total: %d | Distance: %.1fm"),
                DistancePoints, CurrentScore, DistanceTraveled / SideRunnerGameInstanceConstants::METERS_TO_UNREAL_UNITS);
#endif
        }

        // Update last recorded position
        LastRecordedX = PlayerXPosition;

        // Broadcast distance update for UI
        OnDistanceUpdated.Broadcast(DistanceTraveled / SideRunnerGameInstanceConstants::METERS_TO_UNREAL_UNITS);

        // PERFORMANCE: Check win condition after each update
        CheckWinCondition();
    }
}

void USideRunnerGameInstance::AddCoinBonus(int32 CoinValue)
{
    // PERFORMANCE: Early exit if game has ended
    if (bGameEnded)
    {
        return;
    }

    // Validate coin value
    if (CoinValue <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid coin value: %d"), CoinValue);
        return;
    }

    // Add bonus to score
    CurrentScore += CoinValue;
    OnScoreUpdated.Broadcast(CurrentScore);

#if UE_BUILD_DEVELOPMENT
    UE_LOG(LogTemp, VeryVerbose, TEXT("Coin bonus added: +%d points | Total score: %d"), CoinValue, CurrentScore);
#endif
}

void USideRunnerGameInstance::AddEnemyKillBonus(int32 BonusValue)
{
    // PERFORMANCE: Early exit if game has ended
    if (bGameEnded)
    {
        return;
    }

    // Validate bonus value
    if (BonusValue <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid enemy kill bonus: %d"), BonusValue);
        return;
    }

    // Add bonus to score
    CurrentScore += BonusValue;
    OnScoreUpdated.Broadcast(CurrentScore);

#if UE_BUILD_DEVELOPMENT
    UE_LOG(LogTemp, Log, TEXT("Enemy kill bonus added: +%d points | Total score: %d"), BonusValue, CurrentScore);
#endif
}

void USideRunnerGameInstance::CheckWinCondition()
{
    // PERFORMANCE: Early exit if already ended
    if (bGameEnded)
    {
        return;
    }

    // Convert win distance to Unreal units for comparison
    const float WinDistanceUnrealUnits = WinDistance * SideRunnerGameInstanceConstants::METERS_TO_UNREAL_UNITS;

    // Check if player has reached or exceeded the win distance
    if (DistanceTraveled >= WinDistanceUnrealUnits)
    {
        TriggerGameOver(true);
    }
}

void USideRunnerGameInstance::TriggerGameOver(bool bWon)
{
    // PERFORMANCE: Prevent duplicate game over processing
    if (bGameEnded)
    {
        return;
    }

    // Mark game as ended
    bGameEnded = true;

    // Update high score
    UpdateHighScore();

    // Broadcast appropriate event
    if (bWon)
    {
        OnGameWon.Broadcast();

        UE_LOG(LogTemp, Warning, TEXT("=== GAME WON! ==="));
        UE_LOG(LogTemp, Warning, TEXT("Distance: %.1f meters"),
            DistanceTraveled / SideRunnerGameInstanceConstants::METERS_TO_UNREAL_UNITS);
        UE_LOG(LogTemp, Warning, TEXT("Final Score: %d"), CurrentScore);
        UE_LOG(LogTemp, Warning, TEXT("High Score: %d"), HighScore);

        // Display on-screen message if available
#if !UE_BUILD_SHIPPING
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green,
                FString::Printf(TEXT("YOU WIN! Score: %d | Distance: %.1fm"),
                    CurrentScore, DistanceTraveled / SideRunnerGameInstanceConstants::METERS_TO_UNREAL_UNITS));
        }
#endif
    }
    else
    {
        OnGameLost.Broadcast();

        UE_LOG(LogTemp, Warning, TEXT("=== GAME OVER ==="));
        UE_LOG(LogTemp, Warning, TEXT("Distance: %.1f meters"),
            DistanceTraveled / SideRunnerGameInstanceConstants::METERS_TO_UNREAL_UNITS);
        UE_LOG(LogTemp, Warning, TEXT("Final Score: %d"), CurrentScore);
        UE_LOG(LogTemp, Warning, TEXT("High Score: %d"), HighScore);

        // Display on-screen message if available
#if !UE_BUILD_SHIPPING
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red,
                FString::Printf(TEXT("GAME OVER! Score: %d | Distance: %.1fm"),
                    CurrentScore, DistanceTraveled / SideRunnerGameInstanceConstants::METERS_TO_UNREAL_UNITS));
        }
#endif
    }
}

void USideRunnerGameInstance::ResetGameSession()
{
    // Reset scoring state
    CurrentScore = 0;
    DistanceTraveled = 0.0f;
    LastRecordedX = 0.0f;
    bGameEnded = false;

    // Note: HighScore is intentionally NOT reset

    UE_LOG(LogTemp, Log, TEXT("Game session reset - High score preserved: %d"), HighScore);

    // Broadcast reset events
    OnScoreUpdated.Broadcast(CurrentScore);
    OnDistanceUpdated.Broadcast(0.0f);
}
