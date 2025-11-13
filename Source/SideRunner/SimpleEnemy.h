// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SimpleEnemy.generated.h"

// Forward declarations for optimized compilation
class UBoxComponent;
class UStaticMeshComponent;
class ARunnerCharacter;
class UPlayerHealthComponent;

/**
 * Simple patrol-based enemy for ChromaRunner 2.5D platformer.
 *
 * Features:
 * - Configurable back-and-forth patrol along Y-axis
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
 * - Deals damage via PlayerHealthComponent::TakeDamage()
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

	// ========================================
	// GAMEPLAY PROPERTIES - BLUEPRINT EXPOSED
	// ========================================

	/**
	 * Movement speed in units per second.
	 * Range: 100-800 units/s (default: 300)
	 * Higher values create faster, more aggressive enemies.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Movement", meta = (ClampMin = "100.0", ClampMax = "800.0"))
	float MoveSpeed = 300.0f;

	/**
	 * Damage dealt to player on contact.
	 * Range: 10-100 HP (default: 25 = 4 hits to kill at 100 HP)
	 * Configurable per enemy for difficulty scaling.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Combat", meta = (ClampMin = "10", ClampMax = "100"))
	int32 ContactDamage = 25;

	/**
	 * Enable or disable patrol behavior.
	 * If false, enemy remains stationary at spawn location.
	 * Useful for creating guard-type enemies.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Movement")
	bool bPatrolMode = true;

	/**
	 * Maximum distance enemy patrols from spawn point.
	 * Range: 100-1000 units (default: 400)
	 * Patrol covers PatrolDistance in each direction (total range = 2x).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Movement", meta = (ClampMin = "100.0", ClampMax = "1000.0"))
	float PatrolDistance = 400.0f;

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
	 * @return 1 for forward, -1 for backward
	 */
	UFUNCTION(BlueprintPure, Category = "Enemy|Movement")
	int32 GetPatrolDirection() const { return PatrolDirection; }

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
	// MOVEMENT IMPLEMENTATION
	// ========================================

	/**
	 * Executes simple back-and-forth patrol movement.
	 *
	 * Algorithm:
	 * 1. Calculate 2D distance from spawn point
	 * 2. If distance exceeds PatrolDistance, reverse direction
	 * 3. Move along Y-axis (side-scrolling direction)
	 *
	 * Performance: Uses Dist2D for 2D-only calculation (faster than 3D)
	 *
	 * @param DeltaTime Time since last frame for frame-rate independent movement
	 */
	void SimplePatrolMovement(float DeltaTime);

	/**
	 * Destroys enemy if it falls too far behind player.
	 *
	 * Purpose: Performance optimization
	 * - Prevents accumulation of off-screen enemies
	 * - Reduces tick overhead for unseen actors
	 * - Frees memory for new enemy spawns
	 *
	 * Trigger: Enemy X position < (Player X - CleanupDistance)
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
	 * 3. Call PlayerHealthComponent::TakeDamage() with EnemyMelee type
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
	 * Current patrol direction multiplier.
	 * Values: +1 (forward along Y) or -1 (backward along Y)
	 * Flips when patrol distance limit is reached.
	 */
	int32 PatrolDirection = 1;

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
