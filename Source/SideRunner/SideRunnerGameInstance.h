#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "SideRunnerGameInstance.generated.h"

/**
 * Delegate fired when the player's score changes
 * @param NewScore - The updated total score
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnScoreUpdated, int32, NewScore);

/**
 * Delegate fired when the distance traveled changes
 * @param NewDistance - The updated distance in meters
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDistanceUpdated, float, NewDistance);

/**
 * Delegate fired when the player wins (reaches target distance)
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameWon);

/**
 * Delegate fired when the player loses (dies before reaching target)
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameLost);

/**
 * Delegate fired when lives count changes
 * @param CurrentLives - Current remaining lives
 * @param MaxLives - Maximum lives capacity
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLivesUpdated, int32, CurrentLives, int32, MaxLives);

/**
 * Performance constants for game instance calculations
 * Centralized to ensure consistency and improve maintainability
 */
namespace SideRunnerGameInstanceConstants
{
    /** Conversion factor: Unreal units to meters (100 units = 1 meter) */
    constexpr float METERS_TO_UNREAL_UNITS = 100.0f;

    /** Default coin bonus points */
    constexpr int32 DEFAULT_COIN_BONUS = 10;

    /** Default enemy kill bonus points */
    constexpr int32 DEFAULT_ENEMY_KILL_BONUS = 50;

    /** Default win distance in meters */
    constexpr float DEFAULT_WIN_DISTANCE = 5000.0f;

    /** Default starting lives count */
    constexpr int32 DEFAULT_MAX_LIVES = 3;
}

/**
 * ChromaRunner Game Instance - Persistent game state and scoring system.
 *
 * Features:
 * - Distance-based scoring (1 point per meter traveled)
 * - Coin bonus system (configurable points per coin)
 * - Enemy kill bonuses (configurable points per kill)
 * - Win condition at configurable distance (default: 5000m)
 * - High score tracking across game sessions
 * - Event delegates for UI integration
 *
 * Performance Optimizations:
 * - Distance updates only count forward progress (no negative scoring)
 * - Score calculations use integer math for cache efficiency
 * - State flags prevent redundant processing after game end
 *
 * Thread Safety: All methods should be called from the game thread.
 */
UCLASS()
class SIDERUNNER_API USideRunnerGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    /** Called when the GameInstance is initialized */
    virtual void Init() override;

    // ======================================================================
    // Score Management
    // ======================================================================

    /**
     * Updates the player's distance score based on current X position.
     * Only counts forward progress (positive X movement).
     * Awards 1 point per meter traveled (100 Unreal units).
     * Automatically checks win condition after each update.
     *
     * @param PlayerXPosition - Current X coordinate of the player in world space
     */
    UFUNCTION(BlueprintCallable, Category = "Score")
    void UpdateDistanceScore(float PlayerXPosition);

    /**
     * Adds bonus points for collecting a coin.
     * Default value is 10 points per coin.
     *
     * @param CoinValue - Bonus points to add (default: 10)
     */
    UFUNCTION(BlueprintCallable, Category = "Score")
    void AddCoinBonus(int32 CoinValue = 10);

    /**
     * Adds bonus points for killing an enemy.
     * Intended for future enemy system integration.
     *
     * @param BonusValue - Bonus points to add (default: 50)
     */
    UFUNCTION(BlueprintCallable, Category = "Score")
    void AddEnemyKillBonus(int32 BonusValue = 50);

    /**
     * Returns the current total score (distance + bonuses).
     *
     * @return Current score value
     */
    UFUNCTION(BlueprintPure, Category = "Score")
    int32 GetCurrentScore() const { return CurrentScore; }

    /**
     * Returns the total distance traveled in meters.
     *
     * @return Distance traveled in meters
     */
    UFUNCTION(BlueprintPure, Category = "Score")
    float GetDistanceTraveled() const { return DistanceTraveled / SideRunnerGameInstanceConstants::METERS_TO_UNREAL_UNITS; }

    /**
     * Returns the high score achieved in any session.
     *
     * @return High score value
     */
    UFUNCTION(BlueprintPure, Category = "Score")
    int32 GetHighScore() const { return HighScore; }

    /**
     * Returns the raw distance traveled in Unreal units.
     * Used internally for precise calculations.
     *
     * @return Distance in Unreal units
     */
    UFUNCTION(BlueprintPure, Category = "Score")
    float GetRawDistanceTraveled() const { return DistanceTraveled; }

    // ======================================================================
    // Game State Management
    // ======================================================================

    /**
     * Checks if the player has reached the win distance.
     * Called automatically after each distance update.
     * Triggers OnGameWon event if win condition is met.
     */
    UFUNCTION(BlueprintCallable, Category = "Game")
    void CheckWinCondition();

    /**
     * Triggers the game over sequence.
     * Updates high score, broadcasts appropriate event, and marks game as ended.
     *
     * @param bWon - True if player won, false if player died
     */
    UFUNCTION(BlueprintCallable, Category = "Game")
    void TriggerGameOver(bool bWon);

    /**
     * Resets all game state for a new session.
     * Clears score, distance, and game-ended flag.
     * Preserves high score.
     */
    UFUNCTION(BlueprintCallable, Category = "Game")
    void ResetGameSession();

    /**
     * Returns whether the game has ended (win or lose).
     *
     * @return True if game has ended
     */
    UFUNCTION(BlueprintPure, Category = "Game")
    bool HasGameEnded() const { return bGameEnded; }

    // ======================================================================
    // Lives Management
    // ======================================================================

    /**
     * Decrements the lives counter and broadcasts update.
     * Returns true if lives remain, false if game over.
     *
     * @return True if player has lives remaining, false if game over
     */
    UFUNCTION(BlueprintCallable, Category = "Lives")
    bool DecrementLives();

    /**
     * Resets lives to maximum value.
     * Called at game start and after restart from game over.
     */
    UFUNCTION(BlueprintCallable, Category = "Lives")
    void ResetLives();

    /**
     * Returns current remaining lives.
     *
     * @return Current lives count
     */
    UFUNCTION(BlueprintPure, Category = "Lives")
    int32 GetCurrentLives() const { return CurrentLives; }

    /**
     * Returns maximum lives capacity.
     *
     * @return Maximum lives value
     */
    UFUNCTION(BlueprintPure, Category = "Lives")
    int32 GetMaxLives() const { return MaxLives; }

    /**
     * Returns whether player has any lives remaining.
     *
     * @return True if CurrentLives > 0
     */
    UFUNCTION(BlueprintPure, Category = "Lives")
    bool HasLivesRemaining() const { return CurrentLives > 0; }

    /**
     * Stores the current respawn position (for future checkpoint system).
     *
     * @param RespawnLocation - World location to respawn at
     */
    UFUNCTION(BlueprintCallable, Category = "Lives")
    void SetRespawnLocation(const FVector& RespawnLocation);

    /**
     * Gets the stored respawn location.
     *
     * @return Stored respawn world location
     */
    UFUNCTION(BlueprintPure, Category = "Lives")
    FVector GetRespawnLocation() const { return LastRespawnLocation; }

    /**
     * Initializes the distance tracking from player's starting position.
     * Should be called once at game start to ensure accurate score calculation.
     *
     * @param StartingXPosition - Player's initial X coordinate in world space
     */
    UFUNCTION(BlueprintCallable, Category = "Score")
    void InitializeDistanceTracking(float StartingXPosition);

    // ======================================================================
    // Debug Commands (Development/Editor builds only)
    // ======================================================================

#if !UE_BUILD_SHIPPING
    /**
     * Debug console command to trigger game over for testing.
     * Usage: TriggerGameOver
     * @note Only available in non-shipping builds
     */
    UFUNCTION(Exec, Category = "Debug")
    void DebugTriggerGameOver();

    /**
     * Debug console command to set score for testing.
     * Usage: SetScore 1000
     * @param NewScore - Score value to set
     * @note Only available in non-shipping builds
     */
    UFUNCTION(Exec, Category = "Debug")
    void DebugSetScore(int32 NewScore);

    /**
     * Debug console command to add lives for testing.
     * Usage: AddLives 5
     * @param LivestoAdd - Number of lives to add
     * @note Only available in non-shipping builds
     */
    UFUNCTION(Exec, Category = "Debug")
    void DebugAddLives(int32 LivestoAdd);
#endif

    // ======================================================================
    // Events for UI Integration
    // ======================================================================

    /** Broadcast when score changes - bind to update score UI */
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnScoreUpdated OnScoreUpdated;

    /** Broadcast when distance changes - bind to update distance UI */
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnDistanceUpdated OnDistanceUpdated;

    /** Broadcast when player wins - bind to show victory screen */
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnGameWon OnGameWon;

    /** Broadcast when player loses - bind to show game over screen */
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnGameLost OnGameLost;

    /** Broadcast when lives count changes - bind to update lives UI */
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnLivesUpdated OnLivesUpdated;

protected:
    // ======================================================================
    // Scoring State
    // ======================================================================

    /** Current total score (distance points + bonuses) */
    UPROPERTY(BlueprintReadOnly, Category = "Score")
    int32 CurrentScore;

    /** Total distance traveled in Unreal units (divide by 100 for meters) */
    UPROPERTY(BlueprintReadOnly, Category = "Score")
    float DistanceTraveled;

    /** Highest score achieved across all sessions */
    UPROPERTY(BlueprintReadOnly, Category = "Score")
    int32 HighScore;

    /** Distance required to win in meters (default: 5000m) */
    UPROPERTY(EditDefaultsOnly, Category = "Game", meta = (ClampMin = "1000.0", ClampMax = "10000.0"))
    float WinDistance;

    // ======================================================================
    // Lives State
    // ======================================================================

    /** Maximum lives capacity (default: 3) */
    UPROPERTY(EditDefaultsOnly, Category = "Lives", meta = (ClampMin = "1", ClampMax = "10"))
    int32 MaxLives;

    /** Current remaining lives */
    UPROPERTY(BlueprintReadOnly, Category = "Lives")
    int32 CurrentLives;

    /** Last respawn location (for checkpoint system) */
    UPROPERTY(BlueprintReadOnly, Category = "Lives")
    FVector LastRespawnLocation;

private:
    // ======================================================================
    // Internal State
    // ======================================================================

    /** Last recorded X position - used to calculate forward progress only */
    float LastRecordedX;

    /** Flag to prevent processing after game ends */
    bool bGameEnded;

    // ======================================================================
    // Helper Functions
    // ======================================================================

    /**
     * Converts distance delta to score points.
     * 1 meter (100 Unreal units) = 1 point
     *
     * @param DeltaDistance - Distance moved in Unreal units
     * @return Score points earned from distance
     */
    FORCEINLINE int32 ConvertDistanceToPoints(float DeltaDistance) const
    {
        return FMath::FloorToInt(DeltaDistance / SideRunnerGameInstanceConstants::METERS_TO_UNREAL_UNITS);
    }

    /**
     * Updates the high score if current score exceeds it.
     */
    FORCEINLINE void UpdateHighScore()
    {
        if (CurrentScore > HighScore)
        {
            HighScore = CurrentScore;
        }
    }
};
