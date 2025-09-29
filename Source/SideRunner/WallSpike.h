// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Spikes.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"
#include "WallSpike.generated.h"

// Forward declarations
class ARunnerCharacter;
class UPlayerHealthComponent;

/**
 * Performance-optimized wall spike that chases the player with configurable behavior.
 * Unlike regular Spikes which deal incremental damage, WallSpikes are lethal moving traps
 * that actively pursue the player with directional bias for constant pressure.
 * Features instant-death mechanics and intelligent audio management.
 */
UCLASS()
class SIDERUNNER_API AWallSpike : public ASpikes
{
	GENERATED_BODY()

public:
	AWallSpike();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	
	// Collision detection overrides
	virtual void NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp,
		bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;

protected:
	// PERFORMANCE: Chasing behavior properties
	
	/** Base speed for chasing the player */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chase Behavior", meta = (ClampMin = "100.0", ClampMax = "2000.0"))
	float ChaseSpeed = 400.0f;
	
	/** How strongly the spike moves in primary direction (0.0 = pure chase, 1.0 = pure directional movement) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chase Behavior", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DirectionalBias = 0.4f;
	
	/** Maximum distance to start chasing the player */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chase Behavior", meta = (ClampMin = "0.0", ClampMax = "5000.0"))
	float ChaseRange = 1500.0f;
	
	/** How quickly the spike can change direction (higher = more responsive) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chase Behavior", meta = (ClampMin = "0.1", ClampMax = "10.0"))
	float DirectionChangeRate = 2.0f;

	// PERFORMANCE: Acceleration properties
	/** Whether the spike should accelerate when close to the player */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chase Behavior")
	bool bAccelerateWhenClose = true;
	
	/** Distance at which the spike starts accelerating */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chase Behavior", meta = (EditCondition = "bAccelerateWhenClose", ClampMin = "0.0", ClampMax = "1000.0"))
	float AccelerationRange = 500.0f;
	
	/** Maximum speed multiplier when accelerating */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chase Behavior", meta = (EditCondition = "bAccelerateWhenClose", ClampMin = "1.0", ClampMax = "5.0"))
	float MaxSpeedMultiplier = 2.0f;

	// PERFORMANCE: Directional behavior
	/** Whether to use preset directions or custom direction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Direction")
	bool bUsePresetDirections = true;
	
	/** Preset direction selection: 0=Forward(+Y), 1=Backward(-Y), 2=Right(+X), 3=Left(-X), 4=Up(+Z), 5=Down(-Z) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Direction", meta = (EditCondition = "bUsePresetDirections", ClampMin = "0", ClampMax = "5"))
	int32 PresetDirectionIndex = 0; // 0 = Forward (+Y)
	
	/** Custom direction vector (only used when bUsePresetDirections is false) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Direction", meta = (EditCondition = "!bUsePresetDirections"))
	FVector CustomDirection = FVector(0.0f, 1.0f, 0.0f);

	// PERFORMANCE: Lifetime management
	/** Time to wait after player death before destroying self */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lifetime", meta = (ClampMin = "0.1", ClampMax = "10.0"))
	float DeathCleanupDelay = 3.0f;
	
	/** Maximum distance behind player before self-destruction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lifetime", meta = (ClampMin = "500.0", ClampMax = "10000.0"))
	float MaxDistanceBehindPlayer = 2000.0f;
	
	/** Maximum time to spend far behind player before giving up */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lifetime", meta = (ClampMin = "1.0", ClampMax = "30.0"))
	float MaxTimeBehindPlayer = 10.0f;

	// PERFORMANCE: Audio properties
	/** Sound to play when WallSpike starts chasing the player (inherited from base class: CollisionSound for death sound) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	USoundBase* ChaseStartSound;
	
	/** Sound to play continuously while chasing (looping sound) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	USoundBase* ChaseLoopSound;
	
	/** Volume multiplier for chase sounds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float ChaseVolumeMultiplier = 1.0f;
	
	/** Pitch multiplier for chase sounds (higher = more frantic) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (ClampMin = "0.5", ClampMax = "2.0"))
	float ChasePitchMultiplier = 1.0f;

private:
	// PERFORMANCE: Cached references and state
	
	/** Cached reference to the player character for performance */
	UPROPERTY()
	ARunnerCharacter* TargetPlayer;
	
	/** Primary movement direction for the wall spike */
	FVector PrimaryDirection;
	
	/** Current movement direction (smoothed) */
	FVector CurrentDirection;
	
	// PERFORMANCE: Optimized timing variables
	/** Time until next player search (optimization) */
	float PlayerSearchTimer;
	
	/** How often to search for the player (in seconds) */
	float PlayerSearchInterval = 0.5f; // Reduced from frequent updates
	
	/** Timer for cleanup after player death - INSTANCE VARIABLE (not static) */
	float PlayerDeathTimer;
	
	/** Timer tracking how long we've been behind the player */
	float TimeBehindPlayer;
	
	// PERFORMANCE: State flags
	/** Whether we've successfully locked onto a player */
	bool bHasTarget;
	
	/** Flag to prevent multiple kills from same wall spike */
	bool bHasKilledPlayer;
	
	/** Whether we're tracking player death for cleanup */
	bool bTrackingPlayerDeath;
	
	// PERFORMANCE: Audio management
	/** Audio component for playing chase loop sound */
	UPROPERTY()
	UAudioComponent* ChaseAudioComponent;

	// PERFORMANCE: Core functionality methods
	
	/** Get the current primary direction based on settings */
	FVector GetPrimaryDirection() const;
	
	/** Find and cache the nearest player */
	void UpdateTargetPlayer();
	
	/** Calculate the desired movement direction based on player position and directional bias */
	FVector CalculateChaseDirection() const;
	
	/** Update the spike's movement with smooth direction changes */
	void UpdateChaseMovement(float DeltaTime);
	
	/** Check if the spike should be destroyed (too far behind player, etc.) */
	void CheckLifetimeAndCleanup();
	
	/** Apply instant death to the player character */
	void ApplyInstantDeathToPlayer(ARunnerCharacter* Player, FVector HitLocation);
	
	void CheckProximityCollision();
	
	// PERFORMANCE: Helper methods for cleaner code
	void HandlePlayerDeathOrLoss();
	void HandlePlayerOutOfRange();
	void HandleChaseAudioStart(bool bWasHasTarget);
	void StopChaseAudio();
	
#if WITH_EDITOR
	void DrawDebugVisualization();
#endif

	// PERFORMANCE: Collision event handlers
	/** Handle overlap events for collision detection */
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
					   UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
					   bool bFromSweep, const FHitResult& SweepResult);
	
	/** Handle hit events for collision detection */
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, 
			   UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
};