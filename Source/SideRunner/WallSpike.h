// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Spikes.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"
#include "WallSpike.generated.h"

class ARunnerCharacter;
class UPlayerHealthComponent;

/**
 * Performance-optimized wall spike that chases the player with configurable behavior.
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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chase Behavior", meta = (ClampMin = "0.0", ClampMax = "2000.0"))
	float ChaseSpeed = 400.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chase Behavior", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DirectionalBias = 0.4f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chase Behavior", meta = (ClampMin = "0.0", ClampMax = "5000.0"))
	float ChaseRange = 1500.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chase Behavior", meta = (ClampMin = "0.1", ClampMax = "10.0"))
	float DirectionChangeRate = 2.0f;

	// PERFORMANCE: Acceleration properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chase Behavior")
	bool bAccelerateWhenClose = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chase Behavior", meta = (EditCondition = "bAccelerateWhenClose", ClampMin = "0.0", ClampMax = "1000.0"))
	float AccelerationRange = 500.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chase Behavior", meta = (EditCondition = "bAccelerateWhenClose", ClampMin = "1.0", ClampMax = "5.0"))
	float MaxSpeedMultiplier = 2.0f;

	// PERFORMANCE: Directional behavior
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Direction")
	bool bUsePresetDirections = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Direction", meta = (EditCondition = "bUsePresetDirections", ClampMin = "0", ClampMax = "5"))
	int32 PresetDirectionIndex = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Direction", meta = (EditCondition = "!bUsePresetDirections"))
	FVector CustomDirection = FVector(0.0f, 1.0f, 0.0f);

	// PERFORMANCE: Lifetime management
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lifetime", meta = (ClampMin = "0.1", ClampMax = "10.0"))
	float DeathCleanupDelay = 3.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lifetime", meta = (ClampMin = "500.0", ClampMax = "10000.0"))
	float MaxDistanceBehindPlayer = 2000.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lifetime", meta = (ClampMin = "1.0", ClampMax = "30.0"))
	float MaxTimeBehindPlayer = 10.0f;

	/** Distance behind player to position WallSpike when player respawns */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lifetime", meta = (ClampMin = "500.0", ClampMax = "5000.0"))
	float RespawnDistanceBehind = 1500.0f;

	// PERFORMANCE: Audio properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	USoundBase* ChaseStartSound;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	USoundBase* ChaseLoopSound;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float ChaseVolumeMultiplier = 1.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (ClampMin = "0.5", ClampMax = "2.0"))
	float ChasePitchMultiplier = 1.0f;

private:
	// PERFORMANCE: Cached references and state
	UPROPERTY()
	ARunnerCharacter* TargetPlayer;
	
	FVector PrimaryDirection;
	FVector CurrentDirection;
	
	// PERFORMANCE: Optimized timing variables
	float PlayerSearchTimer;
	float PlayerSearchInterval = 0.5f; // Reduced from frequent updates
	float PlayerDeathTimer;
	float TimeBehindPlayer;
	
	// PERFORMANCE: State flags
	bool bHasTarget;
	bool bHasKilledPlayer;
	bool bTrackingPlayerDeath;
	
	// PERFORMANCE: Audio management
	UPROPERTY()
	UAudioComponent* ChaseAudioComponent;

	// PERFORMANCE: Core functionality methods
	FVector GetPrimaryDirection() const;
	void UpdateTargetPlayer();
	FVector CalculateChaseDirection() const;
	void UpdateChaseMovement(float DeltaTime);
	void CheckLifetimeAndCleanup();
	void ApplyInstantDeathToPlayer(ARunnerCharacter* Player, FVector HitLocation);
	void CheckProximityCollision();
	
	// PERFORMANCE: Helper methods for cleaner code
	void HandlePlayerDeathOrLoss();
	void HandlePlayerOutOfRange();
	void HandleChaseAudioStart(bool bWasHasTarget);
	void StopChaseAudio();
	void ResetPositionBehindPlayer(ARunnerCharacter* Player);
	
#if WITH_EDITOR
	void DrawDebugVisualization();
#endif

	// PERFORMANCE: Collision event handlers
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
					   UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
					   bool bFromSweep, const FHitResult& SweepResult);
	
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, 
			   UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
};
