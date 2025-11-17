// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SideRunnerPlayerController.generated.h"

/**
 * ChromaRunner Player Controller - Handles debug commands and player input routing.
 *
 * This class centralizes all console debug commands for easier testing and debugging.
 * UFUNCTION(Exec) commands only work in PlayerController classes (not GameInstance).
 */
UCLASS()
class SIDERUNNER_API ASideRunnerPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ASideRunnerPlayerController();

protected:
	virtual void BeginPlay() override;

public:
	// ======================================================================
	// Debug Console Commands (Development/Editor builds only)
	// ======================================================================

	/**
	 * Debug console command to trigger game over for testing.
	 * Usage: DebugTriggerGameOver
	 * @note Only available in non-shipping builds
	 */
	UFUNCTION(Exec, Category = "Debug")
	void DebugTriggerGameOver();

	/**
	 * Debug console command to set score for testing.
	 * Usage: DebugSetScore 1000
	 * @param NewScore - Score value to set
	 * @note Only available in non-shipping builds
	 */
	UFUNCTION(Exec, Category = "Debug")
	void DebugSetScore(int32 NewScore);

	/**
	 * Debug console command to add lives for testing.
	 * Usage: DebugAddLives 5
	 * @param LivestoAdd - Number of lives to add
	 * @note Only available in non-shipping builds
	 */
	UFUNCTION(Exec, Category = "Debug")
	void DebugAddLives(int32 LivestoAdd);

	/**
	 * Teleports player to specific distance (meters) for testing win condition.
	 * Usage: TeleportToDistance 5000
	 * @param DistanceMeters - Target distance in meters
	 * @note Only functional in non-shipping builds
	 */
	UFUNCTION(Exec, Category = "Debug")
	void TeleportToDistance(float DistanceMeters);

	/**
	 * Instantly kills the player for testing death/game over flow.
	 * Usage: KillPlayer
	 * @note Only functional in non-shipping builds
	 */
	UFUNCTION(Exec, Category = "Debug")
	void KillPlayer();

private:
	/** Cached reference to game instance for debug commands */
	UPROPERTY()
	class USideRunnerGameInstance* CachedGameInstance;
};
