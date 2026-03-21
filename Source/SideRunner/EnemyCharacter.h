// Copyright ChromaRunner. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnemyCharacter.generated.h"

class UBoxComponent;
class UPaperFlipbookComponent;
class UPaperFlipbook;

DECLARE_LOG_CATEGORY_EXTERN(LogSideRunnerEnemy, Log, All);

/**
 * AEnemyCharacter — Base enemy for ChromaRunner endless runner.
 *
 * Architecture:
 *   - Timer-driven patrol (NO Tick) for perf on mobile/low-end
 *   - DamageZone (body) hurts player on overlap
 *   - StompZone (head) kills enemy when player lands on it
 *   - PaperFlipbook visual, facing locked to 2.5D plane
 *   - Designed to be spawned by BP_Level segments via SpawnActor, NOT placed in World Partition grid
 *
 * Usage:
 *   Create a Blueprint child (BP_EnemyGroundPatrol) and assign flipbooks in defaults.
 */
UCLASS(Blueprintable, BlueprintType)
class SIDERUNNER_API AEnemyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AEnemyCharacter();

	// --- Components ---

	/** Body collision — damages player on overlap */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UBoxComponent> DamageZone;

	/** Head collision — player defeats enemy by landing on this */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UBoxComponent> StompZone;

	/** 2D sprite visual */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visual")
	TObjectPtr<UPaperFlipbookComponent> EnemySprite;

	// --- Tuning (Data-Driven, editable per-instance) ---

	/** Distance to patrol in each direction from spawn point (Y-axis for side-scroller) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol", meta = (ClampMin = "50.0"))
	float PatrolDistance = 300.0f;

	/** Movement speed during patrol */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol", meta = (ClampMin = "10.0"))
	float PatrolSpeed = 150.0f;

	/** Pause duration at each patrol endpoint before reversing */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol", meta = (ClampMin = "0.0"))
	float PatrolPauseTime = 0.5f;

	/** Damage dealt to player on contact */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (ClampMin = "1"))
	int32 ContactDamage = 1;

	/** Flipbook for walking animation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	TObjectPtr<UPaperFlipbook> WalkFlipbook;

	/** Flipbook for idle/pause animation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	TObjectPtr<UPaperFlipbook> IdleFlipbook;

	/** Flipbook for death animation (optional — destroys after playing) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	TObjectPtr<UPaperFlipbook> DeathFlipbook;

	// --- Blueprint Events ---

	/** Called when enemy is defeated (stomp or attack). Override in BP for FX/audio. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
	void OnEnemyDefeated();

	/** Called when enemy damages the player. Override in BP for hit feedback FX. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
	void OnDamagedPlayer(AActor* Player);

	/** Kill this enemy — plays death anim then destroys */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void DefeatEnemy();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	// --- Patrol State ---

	FVector PatrolOrigin;
	float PatrolDirection = 1.0f; // 1.0 = forward, -1.0 = backward (along Y)
	bool bIsPatrolling = false;
	bool bIsDead = false;

	FTimerHandle PatrolTimerHandle;
	FTimerHandle PauseTimerHandle;
	FTimerHandle DeathTimerHandle;

	// --- Patrol Logic (Timer-driven, NOT Tick) ---

	void StartPatrol();
	void PatrolStep();
	void PauseAtEndpoint();
	void ResumePatrol();

	// --- Overlap Callbacks ---

	UFUNCTION()
	void OnDamageZoneOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	void OnStompZoneOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	void UpdateSpriteDirection();
};
