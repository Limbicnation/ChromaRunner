#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameOverWidget.generated.h"

/**
 * Game Over Widget - Displays win/loss state, score, distance, and lives used.
 * Provides restart and quit functionality.
 *
 * Blueprint Setup Required:
 * - Create WBP_GameOver based on this class
 * - Add TextBlocks: GameOverText, ScoreText, DistanceText, HighScoreText, LivesText
 * - Add Buttons: RestartButton, QuitButton
 * - Bind widgets using "Is Variable" and matching names
 *
 * Performance: Widgets are created on-demand and destroyed after use.
 */
UCLASS()
class SIDERUNNER_API UGameOverWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    /**
     * Configures the game over display with final game statistics.
     *
     * @param bWon - True if player won, false if player lost
     * @param FinalScore - Final score achieved
     * @param DistanceMeters - Distance traveled in meters
     * @param HighScore - Current high score
     * @param LivesUsed - Total lives consumed (e.g., started with 3, ended with 0 = 3 used)
     */
    UFUNCTION(BlueprintCallable, Category = "UI")
    void SetupGameOverDisplay(bool bWon, int32 FinalScore, float DistanceMeters, int32 HighScore, int32 LivesUsed);

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    // ======================================================================
    // Widget Bindings (must match UMG widget names exactly)
    // ======================================================================

    /** Main game over message text ("YOU WIN!" / "GAME OVER") */
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* GameOverText;

    /** Displays final score */
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* ScoreText;

    /** Displays distance traveled */
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* DistanceText;

    /** Displays high score */
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* HighScoreText;

    /** Displays lives used in this attempt */
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* LivesText;

    /** Restart button */
    UPROPERTY(meta = (BindWidget))
    class UButton* RestartButton;

    /** Quit button */
    UPROPERTY(meta = (BindWidget))
    class UButton* QuitButton;

    // ======================================================================
    // Button Click Handlers
    // ======================================================================

    /** Called when restart button is clicked */
    UFUNCTION()
    void OnRestartClicked();

    /** Called when quit button is clicked */
    UFUNCTION()
    void OnQuitClicked();

private:
    /** Cached game instance reference for restart functionality */
    UPROPERTY()
    class USideRunnerGameInstance* CachedGameInstance;
};
