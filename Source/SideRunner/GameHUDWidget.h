#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameHUDWidget.generated.h"

/**
 * In-game HUD Widget - Displays real-time game stats during gameplay.
 * Shows current lives, score, and distance traveled.
 *
 * Blueprint Setup Required:
 * - Create WBP_GameHUD based on this class
 * - Add TextBlocks: LivesText, ScoreText, DistanceText
 * - Bind widgets using "Is Variable" and matching names
 * - Add to viewport at game start via GameMode or PlayerController
 *
 * Performance: Updates only when GameInstance delegates fire (event-driven, not Tick-based).
 */
UCLASS()
class SIDERUNNER_API UGameHUDWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    /**
     * Updates the lives display.
     * Called automatically when GameInstance fires OnLivesUpdated.
     *
     * @param CurrentLives - Current remaining lives
     * @param MaxLives - Maximum lives capacity
     */
    UFUNCTION(BlueprintCallable, Category = "UI")
    void UpdateLivesDisplay(int32 CurrentLives, int32 MaxLives);

    /**
     * Updates the score display.
     * Called automatically when GameInstance fires OnScoreUpdated.
     *
     * @param CurrentScore - Current total score
     */
    UFUNCTION(BlueprintCallable, Category = "UI")
    void UpdateScoreDisplay(int32 CurrentScore);

    /**
     * Updates the distance display.
     * Called automatically when GameInstance fires OnDistanceUpdated.
     *
     * @param DistanceMeters - Distance traveled in meters
     */
    UFUNCTION(BlueprintCallable, Category = "UI")
    void UpdateDistanceDisplay(float DistanceMeters);

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    // ======================================================================
    // Widget Bindings (must match UMG widget names exactly)
    // ======================================================================

    /** Displays current lives (e.g., "Lives: 3/3") */
    UPROPERTY(meta = (BindWidgetOptional))
    class UTextBlock* LivesText;

    /** Displays current score */
    UPROPERTY(meta = (BindWidgetOptional))
    class UTextBlock* ScoreText;

    /** Displays distance traveled */
    UPROPERTY(meta = (BindWidgetOptional))
    class UTextBlock* DistanceText;

private:
    /** Cached game instance reference for delegate binding */
    UPROPERTY()
    class USideRunnerGameInstance* CachedGameInstance;

    // ======================================================================
    // Delegate Handlers (called by GameInstance events)
    // ======================================================================

    UFUNCTION()
    void OnLivesUpdatedHandler(int32 CurrentLives, int32 MaxLives);

    UFUNCTION()
    void OnScoreUpdatedHandler(int32 NewScore);

    UFUNCTION()
    void OnDistanceUpdatedHandler(float NewDistance);
};
