#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SideRunnerGameMode.generated.h"

/**
 * ChromaRunner Game Mode - Manages UI lifecycle and game state flow.
 *
 * Responsibilities:
 * - Creates and displays GameHUDWidget at game start
 * - Listens to GameInstance delegates (OnGameWon, OnGameLost)
 * - Shows GameOverWidget when game ends
 * - Manages input mode transitions (game <-> UI)
 *
 * Blueprint Setup:
 * - Extend this class as BP_SideRunnerGameMode
 * - Set GameHUDWidgetClass and GameOverWidgetClass to Blueprint widgets
 * - Assign to Project Settings > Default GameMode
 */
UCLASS()
class SIDERUNNER_API ASideRunnerGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ASideRunnerGameMode();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// ======================================================================
	// Widget Configuration (Set in Blueprint)
	// ======================================================================

	/** Widget class for in-game HUD (set to WBP_GameHUD in Blueprint) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<class UGameHUDWidget> GameHUDWidgetClass;

	/** Widget class for game over screen (set to WBP_GameOver in Blueprint) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<class UGameOverWidget> GameOverWidgetClass;

	// ======================================================================
	// Widget Instance Management
	// ======================================================================

	/** Active HUD widget instance (weak reference prevents GC issues) */
	UPROPERTY()
	TWeakObjectPtr<class UGameHUDWidget> ActiveHUDWidget;

	/** Active game over widget instance (weak reference) */
	UPROPERTY()
	TWeakObjectPtr<class UGameOverWidget> ActiveGameOverWidget;

private:
	// ======================================================================
	// GameInstance Integration
	// ======================================================================

	/** Cached game instance reference for delegate binding */
	UPROPERTY()
	class USideRunnerGameInstance* CachedGameInstance;

	/** Flag to prevent duplicate game over processing */
	bool bGameOverActive;

	// ======================================================================
	// UI Lifecycle Functions
	// ======================================================================

	/**
	 * Creates and displays the in-game HUD.
	 * Called at BeginPlay.
	 */
	void CreateGameHUD();

	/**
	 * Shows the game over screen with final statistics.
	 * @param bWon - True if player won, false if player lost
	 */
	void ShowGameOverScreen(bool bWon);

	/**
	 * Removes the game HUD from viewport.
	 * Called when game over screen appears.
	 */
	void HideGameHUD();

	// ======================================================================
	// Delegate Handlers (bound to GameInstance events)
	// ======================================================================

	/** Called when player wins (reaches target distance) */
	UFUNCTION()
	void OnGameWonHandler();

	/** Called when player loses (runs out of lives) */
	UFUNCTION()
	void OnGameLostHandler();

	// ======================================================================
	// Input Mode Management
	// ======================================================================

	/**
	 * Switches input mode to UI for game over screen interaction.
	 * Shows mouse cursor and enables click events.
	 */
	void SetInputModeUI();

	/**
	 * Switches input mode back to game.
	 * Hides mouse cursor and disables UI input.
	 */
	void SetInputModeGame();
};
