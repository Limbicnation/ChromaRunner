#pragma once

#include "GameFramework/GameModeBase.h"
#include "SideRunnerGameMode.generated.h"

class UUserWidget;

/**
 * Chroma Runner Game Mode.
 *
 * Manages the overall game state: spawning players, tracking score,
 * and displaying HUD / Game Over widgets.
 *
 * Widget classes are assigned as Blueprintable UPROPERTY so that
 * BP_SideRunnerGameMode (a Blueprint subclass) can configure them
 * without modifying C++ code.
 */
UCLASS(Abstract, Blueprintable)
class SIDERUNNER_API ASideRunnerGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    ASideRunnerGameMode();

    // ── Widget Class Properties ────────────────────────────────────────────────
    // Assign these in BP_SideRunnerGameMode → Class Defaults.
    // WBP_GameHUD: displayed during gameplay (health, score, coins)
    // WBP_GameOver: displayed when the player dies

    /** Widget displayed on-screen during gameplay (health, score, coins). */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
    TSubclassOf<UUserWidget> GameHUDWidgetClass;

    /** Widget displayed on the Game Over screen with score and restart button. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
    TSubclassOf<UUserWidget> GameOverWidgetClass;

    // ── Blueprint Events ────────────────────────────────────────────────────────

    /** Called when the game should display the Game Over screen. Override in BP. */
    UFUNCTION(BlueprintImplementableEvent, Category = "Game|Events")
    void HandleGameOver();

    /** Called when the player reaches a win condition (e.g. 5000m). Override in BP. */
    UFUNCTION(BlueprintImplementableEvent, Category = "Game|Events")
    void HandleGameWon();

    // ── API ──────────────────────────────────────────────────────────────────

    /** Add Points to the current score. */
    UFUNCTION(BlueprintCallable, Category = "Game|Score")
    void AddScore(int32 Points);

    /** Get the current score. */
    UFUNCTION(BlueprintPure, Category = "Game|Score")
    int32 GetScore() const;

    UFUNCTION(BlueprintPure, Category = "Game|Score")
    int32 GetHighScore() const;

protected:
    // ── AGameModeBase overrides ──────────────────────────────────────────────

    virtual void BeginPlay() override;

    // ── Actor lifecycle ─────────────────────────────────────────────────────

    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
    int32 Score = 0;
    int32 HighScore = 0;

    /** Widget instance currently on screen (HUD or GameOver). */
    UPROPERTY()
    UUserWidget* CurrentWidget = nullptr;

    /** Cached reference to the player character — used for delegate binding. */
    UFUNCTION()
    void OnPlayerDeath();

    UFUNCTION()
    void OnPlayerWon();
};
