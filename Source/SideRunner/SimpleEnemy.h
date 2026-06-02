// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GroundPatrol.h"
#include "SimpleEnemy.generated.h"

// Forward declarations for optimized compilation
class UBoxComponent;
class UStaticMeshComponent;
class ARunnerCharacter;
class UPlayerHealth;
class UGroundPatrolComponent;

/**
 * Simple patrol-based enemy for ChromaRunner 2.5D platformer.
 *
 * Features:
 * - Delegates patrol movement to UGroundPatrolComponent (data-driven config)
 * - Collision-based contact damage with cooldown system
 * - Auto-cleanup when behind player for performance optimization
 * - Blueprint-friendly with exposed properties for level design
 *
 * Performance Considerations:
 * - Uses simple 2D distance calculations (Dist2D)
 * - Timer-based damage cooldown (not tick-based)
 * - Automatic cleanup prevents memory leaks
 * - Cache-friendly member layout
 *
 * Integration:
 * - Deals damage via UPlayerHealth::TakeDamage()
 * - Uses EDamageType::EnemyMelee for proper damage categorization
 * - Respects player invulnerability frames
 */
UCLASS()
class SIDERUNNER_API ASimpleEnemy : public AActor
{
	GENERATED_BODY()

public:
	/**
	 * Constructor - Sets default values and initializes components.
	 * - Creates collision box and mesh components
	 * - Configures collision channels for player overlap
	 * - Enables tick for patrol movement
	 */
	ASimpleEnemy();

protected:
	/**
	 * Called when the game starts or when spawned.
	 * - Caches player reference for performance
	 * - Stores initial location for patrol range
	 * - Sets up collision event bindings
	 */
	virtual void BeginPlay() override;

public:
	/**
	 * Called every frame.
	 * - Executes patrol movement if enabled
	 * - Performs cleanup check for optimization
	 *
	 * @param DeltaTime Time since last frame in seconds
	 */
	virtual void Tick(float DeltaTime) override;

	// ========================================
	// COMPONENTS
	// ========================================

	/**
	 * Collision box for overlap detection with player.
	 * RootComponent for the enemy actor.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* CollisionBox;

	/**
	 * Visual mesh representation of the enemy.
	 * Designers can assign custom meshes in Blueprint.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* EnemyMesh;

	/**
	 * Reusable patrol component — drives back-and-forth movement.
	 * Configure speed, distance, detection in the Details panel.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UGroundPatrolComponent* PatrolComponent;

	// ========================================
	// GAMEPLAY PROPERTIES - BLUEPRINT EXPOSED
	// ========================================

	/**
	 * Damage dealt to player on contact.
	 * Range: 10-100 HP (default: 25 = 4 hits to kill at 100 HP)
	 * Configurable per enemy for difficulty scaling.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Combat", meta = (ClampMin = "10", ClampMax = "100"))
	int32 ContactDamage = 25;

	/**
	 * Distance in units behind player before auto-cleanup.
	 * Default: 2000 units
	 * Prevents off-screen enemies from consuming resources.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Optimization", meta = (ClampMin = "500.0", ClampMax = "5000.0"))
	float CleanupDistance = 2000.0f;

	// ========================================
	// GAMEPLAY FUNCTIONS
	// ========================================

	/**
	 * Gets the current patrol direction.
	 * Delegates to UGroundPatrolComponent.
	 */
	UFUNCTION(BlueprintPure, Category = "Enemy|Movement")
	EPatrolDirection GetPatrolDirection() const;

	/**
	 * Gets the spawn location used for patrol range calculation.
	 * @return Initial world location when enemy was spawned
	 */
	UFUNCTION(BlueprintPure, Category = "Enemy|Movement")
	FVector GetStartLocation() const { return StartLocation; }

	/**
	 * Checks if enemy has recently dealt damage (cooldown active).
	 * @return True if cooldown is active, false if can deal damage again
	 */
	UFUNCTION(BlueprintPure, Category = "Enemy|Combat")
	bool HasRecentlyDealtDamage() const { return bHasDealtDamage; }

protected:
	// ========================================
	// CLEANUP
	// ========================================

	/**
	 * Destroys enemy if it falls too far behind player.
	 * Performance optimization — prevents accumulation of off-screen enemies.
	 */
	void CleanupIfBehindPlayer();

	// ========================================
	// COLLISION HANDLING
	// ========================================

	/**
	 * Handles overlap begin event with collision box.
	 *
	 * Damage Dealing Logic:
	 * 1. Verify overlapping actor is player
	 * 2. Check damage cooldown flag
	 * 3. Call UPlayerHealth::TakeDamage() with EnemyMelee type
	 * 4. Set cooldown flag to prevent multi-hit
	 * 5. Start timer to reset cooldown after invulnerability
	 *
	 * Cooldown Duration: 1.5 seconds (matches typical invulnerability frame duration)
	 *
	 * @param OverlappedComponent The collision box component
	 * @param OtherActor The actor entering overlap (should be player)
	 * @param OtherComp The other actor's component
	 * @param OtherBodyIndex Body index for multi-body actors
	 * @param bFromSweep True if from sweep operation
	 * @param SweepResult Sweep trace result data
	 */
	UFUNCTION()
	void OnOverlapBegin(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

private:
	// ========================================
	// INTERNAL STATE - NOT BLUEPRINT EXPOSED
	// ========================================

	/**
	 * Cached reference to player character.
	 * Performance: Avoids repeated GetPlayerCharacter() calls.
	 * Memory: Stored as UPROPERTY for GC safety (TWeakObjectPtr alternative).
	 */
	UPROPERTY()
	ARunnerCharacter* PlayerRef;

	/**
	 * World location where enemy spawned.
	 * Used as center point for patrol range calculation.
	 */
	FVector StartLocation;

	/**
	 * Multi-hit prevention flag.
	 * True: Damage cooldown active, cannot deal damage
	 * False: Can deal damage on next overlap
	 * Reset via timer after 1.5 seconds.
	 */
	bool bHasDealtDamage = false;

	/**
	 * Timer handle for damage cooldown reset.
	 * Allows lambda-based timer callback for clean cooldown logic.
	 */
	FTimerHandle DamageCooldownTimer;
};
