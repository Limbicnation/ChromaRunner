#include "SideRunnerGameInstance.h"
#include "Engine/Engine.h"
#include "SideRunner.h" // Custom log categories

void USideRunnerGameInstance::Init()
{
    Super::Init();

    // Initialize scoring state
    CurrentScore = 0;
    DistanceTraveled = 0.0f;
    HighScore = 0;
    LastRecordedY = 0.0f;
    bGameEnded = false;
    LastMilestone = 0;

    // Set default win distance
    WinDistance = SideRunnerGameInstanceConstants::DEFAULT_WIN_DISTANCE;

    // Initialize lives state
    MaxLives = SideRunnerGameInstanceConstants::DEFAULT_MAX_LIVES;
    CurrentLives = MaxLives;
    LastRespawnLocation = FVector::ZeroVector;

    UE_LOG(LogSideRunnerScoring, Log, TEXT("SideRunnerGameInstance initialized - Win distance: %.1f meters, Lives: %d"), WinDistance, MaxLives);
}

void USideRunnerGameInstance::UpdateDistanceScore(float PlayerYPosition)
{
    // PERFORMANCE: Early exit if game has ended
    if (bGameEnded)
    {
        return;
    }

    // PERFORMANCE: Only count forward progress (positive Y movement)
    if (PlayerYPosition > LastRecordedY)
    {
        // Calculate distance delta
        const float DeltaDistance = PlayerYPosition - LastRecordedY;
        DistanceTraveled += DeltaDistance;

        // PERFORMANCE: Convert distance to points (1 meter = 1 point)
        const int32 DistancePoints = ConvertDistanceToPoints(DeltaDistance);

        if (DistancePoints > 0)
        {
            CurrentScore += DistancePoints;
            OnScoreUpdated.Broadcast(CurrentScore);

#if UE_BUILD_DEVELOPMENT
            UE_LOG(LogSideRunnerScoring, VeryVerbose, TEXT("Distance score updated: +%d points | Total: %d | Distance: %.1fm"),
                DistancePoints, CurrentScore, DistanceTraveled / SideRunnerGameInstanceConstants::METERS_TO_UNREAL_UNITS);
#endif
        }

        // Update last recorded position
        LastRecordedY = PlayerYPosition;

        // Broadcast distance update for UI
        OnDistanceUpdated.Broadcast(DistanceTraveled / SideRunnerGameInstanceConstants::METERS_TO_UNREAL_UNITS);

        // PERFORMANCE: Check win condition after each update
        CheckWinCondition();

        // Check for milestone (every 1000m)
        CheckMilestone();
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
        UE_LOG(LogSideRunnerScoring, Warning, TEXT("Invalid coin value: %d"), CoinValue);
        return;
    }

    // Add bonus to score
    CurrentScore += CoinValue;
    OnScoreUpdated.Broadcast(CurrentScore);

#if UE_BUILD_DEVELOPMENT
    UE_LOG(LogSideRunnerScoring, VeryVerbose, TEXT("Coin bonus added: +%d points | Total score: %d"), CoinValue, CurrentScore);
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
        UE_LOG(LogSideRunnerScoring, Warning, TEXT("Invalid enemy kill bonus: %d"), BonusValue);
        return;
    }

    // Add bonus to score
    CurrentScore += BonusValue;
    OnScoreUpdated.Broadcast(CurrentScore);

#if UE_BUILD_DEVELOPMENT
    UE_LOG(LogSideRunnerScoring, Log, TEXT("Enemy kill bonus added: +%d points | Total score: %d"), BonusValue, CurrentScore);
#endif
}

void USideRunnerGameInstance::CheckWinCondition()
{
    // PERFORMANCE: Early exit if already ended
    if (bGameEnded)
    {
        return;
    }

    // In endless mode, the win condition is disabled
    if (bEndlessMode)
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

    // Only allow game over if no lives remain or player won
    if (!bWon && CurrentLives > 0)
    {
        UE_LOG(LogSideRunnerScoring, Warning, TEXT("TriggerGameOver called but player has %d lives remaining"), CurrentLives);
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

        UE_LOG(LogSideRunnerScoring, Log, TEXT("=== GAME WON! ==="));
        UE_LOG(LogSideRunnerScoring, Log, TEXT("Distance: %.1f meters"),
            DistanceTraveled / SideRunnerGameInstanceConstants::METERS_TO_UNREAL_UNITS);
        UE_LOG(LogSideRunnerScoring, Log, TEXT("Final Score: %d"), CurrentScore);
        UE_LOG(LogSideRunnerScoring, Log, TEXT("High Score: %d"), HighScore);

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

        UE_LOG(LogSideRunnerScoring, Log, TEXT("=== GAME OVER ==="));
        UE_LOG(LogSideRunnerScoring, Log, TEXT("Distance: %.1f meters"),
            DistanceTraveled / SideRunnerGameInstanceConstants::METERS_TO_UNREAL_UNITS);
        UE_LOG(LogSideRunnerScoring, Log, TEXT("Final Score: %d"), CurrentScore);
        UE_LOG(LogSideRunnerScoring, Log, TEXT("High Score: %d"), HighScore);

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
    LastRecordedY = 0.0f;
    bGameEnded = false;
    LastMilestone = 0;

    // Reset lives
    ResetLives();

    // Note: HighScore is intentionally NOT reset

    UE_LOG(LogSideRunnerScoring, Log, TEXT("Game session reset - High score preserved: %d"), HighScore);

    // Broadcast reset events
    OnScoreUpdated.Broadcast(CurrentScore);
    OnDistanceUpdated.Broadcast(0.0f);
}

bool USideRunnerGameInstance::DecrementLives()
{
    if (CurrentLives <= 0)
    {
        UE_LOG(LogSideRunnerScoring, Warning, TEXT("DecrementLives called but lives already at 0"));
        return false;
    }

    CurrentLives--;
    OnLivesUpdated.Broadcast(CurrentLives, MaxLives);

    UE_LOG(LogSideRunnerScoring, Log, TEXT("Lives decremented - Remaining: %d/%d"), CurrentLives, MaxLives);

    // Trigger game over only if no lives remain
    if (CurrentLives <= 0)
    {
        TriggerGameOver(false);
        return false;
    }

    return true;
}

void USideRunnerGameInstance::ResetLives()
{
    CurrentLives = MaxLives;
    OnLivesUpdated.Broadcast(CurrentLives, MaxLives);

    UE_LOG(LogSideRunnerScoring, Log, TEXT("Lives reset to %d/%d"), CurrentLives, MaxLives);
}

void USideRunnerGameInstance::SetRespawnLocation(const FVector& RespawnLocation)
{
    LastRespawnLocation = RespawnLocation;
    UE_LOG(LogSideRunner, VeryVerbose, TEXT("Respawn location set to: %s"), *RespawnLocation.ToString());
}

void USideRunnerGameInstance::InitializeDistanceTracking(float StartingYPosition)
{
    LastRecordedY = StartingYPosition;
    UE_LOG(LogSideRunnerScoring, Log, TEXT("Distance tracking initialized at Y=%.1f"), StartingYPosition);
}

// Note: Debug console commands have been moved to ASideRunnerPlayerController
// for proper Exec function support in UE5.5 (Exec only works in PlayerController)

void USideRunnerGameInstance::CheckMilestone()
{
    // Only relevant in endless mode
    if (!bEndlessMode)
    {
        return;
    }

    const float DistanceMeters = DistanceTraveled / SideRunnerGameInstanceConstants::METERS_TO_UNREAL_UNITS;
    const int32 CurrentMilestone = FMath::FloorToInt(DistanceMeters / 1000.0f);

    if (CurrentMilestone > LastMilestone)
    {
        LastMilestone = CurrentMilestone;
        OnMilestoneReached.Broadcast(LastMilestone);

        UE_LOG(LogSideRunnerScoring, Log, TEXT("Milestone reached: %d (Distance: %.0fm)"),
               LastMilestone, DistanceMeters);
    }
}
