// Fill out your copyright notice in the Description page of Project Settings.

#include "SimpleEnemy.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "RunnerCharacter.h"
#include "PlayerHealthComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

// Performance-critical constants - evaluated at compile time
namespace EnemyConstants
{
	/** Collision box half-extents (X, Y, Z) in centimeters */
	constexpr FVector CollisionBoxExtent(50.0f, 50.0f, 100.0f);

	/** Damage cooldown duration in seconds (matches typical invulnerability frames) */
	constexpr float DamageCooldownDuration = 1.5f;

	/** Collision channel name for player detection */
	static const FName PlayerCollisionProfile(TEXT("OverlapAllDynamic"));
}

/**
 * Constructor - Initializes components and default properties.
 *
 * Component Hierarchy:
 *   CollisionBox (RootComponent)
 *   └─ EnemyMesh (attached child)
 *
 * Collision Setup:
 * - CollisionBox: Query-only (overlap), blocks nothing
 * - Generates overlap events for player detection
 * - Uses OverlapAllDynamic profile for maximum compatibility
 *
 * Performance:
 * - Tick enabled by default (required for patrol)
 * - Can be disabled in Blueprint for stationary enemies
 */
ASimpleEnemy::ASimpleEnemy()
{
	// Enable tick for patrol movement
	PrimaryActorTick.bCanEverTick = true;

	// ========================================
	// COLLISION BOX SETUP
	// ========================================

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	RootComponent = CollisionBox;

	// Set collision box dimensions
	CollisionBox->SetBoxExtent(EnemyConstants::CollisionBoxExtent);

	// Configure collision profile for player overlap detection
	// Query-only: No physics simulation, just overlap detection
	CollisionBox->SetCollisionProfileName(EnemyConstants::PlayerCollisionProfile);
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	// Enable overlap events for damage dealing
	CollisionBox->SetGenerateOverlapEvents(true);

	// ========================================
	// MESH COMPONENT SETUP
	// ========================================

	EnemyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EnemyMesh"));
	EnemyMesh->SetupAttachment(CollisionBox);

	// Mesh is visual only - no collision
	EnemyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	EnemyMesh->SetGenerateOverlapEvents(false);

	// Allow Blueprint designers to override mesh appearance
	EnemyMesh->SetIsReplicated(false); // Single-player game, no replication needed
}

/**
 * BeginPlay - Initialization when spawned into world.
 *
 * Initialization Steps:
 * 1. Cache player reference for performance (avoid repeated searches)
 * 2. Store spawn location for patrol range calculation
 * 3. Bind collision overlap event for damage dealing
 *
 * Error Handling:
 * - Logs warning if player not found (shouldn't happen in gameplay)
 * - Enemy still spawns but won't deal damage or cleanup
 */
void ASimpleEnemy::BeginPlay()
{
	Super::BeginPlay();

	// ========================================
	// CACHE PLAYER REFERENCE
	// ========================================

	// Get player character (index 0 = first player controller)
	PlayerRef = Cast<ARunnerCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	if (!PlayerRef)
	{
		UE_LOG(LogTemp, Warning, TEXT("SimpleEnemy: Failed to find player character at BeginPlay. Damage and cleanup disabled."));
	}

	// ========================================
	// STORE PATROL START LOCATION
	// ========================================

	// Cache spawn location for patrol range calculation
	StartLocation = GetActorLocation();

	// Initialize patrol direction (could be randomized for variety)
	PatrolDirection = 1; // Start moving forward (+Y direction)

	// ========================================
	// BIND COLLISION EVENTS
	// ========================================

	// Register overlap event for damage dealing
	if (CollisionBox)
	{
		CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &ASimpleEnemy::OnOverlapBegin);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("SimpleEnemy: CollisionBox is null at BeginPlay!"));
	}
}

/**
 * Tick - Called every frame for patrol movement and cleanup.
 *
 * Frame Budget:
 * - Patrol movement: ~0.01ms per enemy
 * - Cleanup check: ~0.005ms per enemy
 * - Target: <0.1ms for 10 enemies on screen
 *
 * Optimization Notes:
 * - Uses simple vector math (no raycasts or expensive checks)
 * - Cleanup check uses 1D comparison (X-axis only)
 * - Can be optimized further with time-sliced updates if needed
 *
 * @param DeltaTime Frame time in seconds (for frame-rate independence)
 */
void ASimpleEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Execute patrol movement if enabled
	if (bPatrolMode)
	{
		SimplePatrolMovement(DeltaTime);
	}

	// Perform cleanup check for performance optimization
	CleanupIfBehindPlayer();
}

/**
 * SimplePatrolMovement - Executes back-and-forth patrol along Y-axis.
 *
 * Movement Algorithm:
 * 1. Get current position and calculate 2D distance from spawn
 * 2. If distance >= PatrolDistance, reverse direction
 * 3. Apply movement along Y-axis based on speed and delta time
 *
 * Coordinate System (2.5D side-scroller):
 * - X: Forward progression (player moves right continuously)
 * - Y: Lateral movement (side-to-side, patrol axis)
 * - Z: Vertical (gravity/jumping)
 *
 * Performance:
 * - Uses FVector::Dist2D for 2D-only distance (skips Z-axis, faster than Dist)
 * - Direct SetActorLocation (no physics simulation overhead)
 * - Frame-rate independent via DeltaTime multiplication
 *
 * Edge Cases:
 * - If PatrolDistance is very small, may oscillate rapidly
 * - Movement is linear (no acceleration/deceleration)
 *
 * @param DeltaTime Frame time for smooth, frame-rate independent movement
 */
void ASimpleEnemy::SimplePatrolMovement(float DeltaTime)
{
	// Get current world position
	const FVector CurrentPos = GetActorLocation();

	// Calculate 2D distance from spawn point (ignores Z for performance)
	const float DistanceFromStart = FVector::Dist2D(CurrentPos, StartLocation);

	// Reverse direction when patrol limit reached
	if (DistanceFromStart >= PatrolDistance)
	{
		PatrolDirection *= -1; // Flip: +1 becomes -1, -1 becomes +1
	}

	// Calculate frame-rate independent movement delta
	// Movement is along Y-axis (side-to-side) for 2.5D gameplay
	const FVector Movement = FVector(0.0f, PatrolDirection * MoveSpeed * DeltaTime, 0.0f);

	// Apply movement (direct location update, no physics)
	SetActorLocation(CurrentPos + Movement);
}

/**
 * CleanupIfBehindPlayer - Destroys enemy when too far behind player.
 *
 * Purpose: Performance optimization
 * - Prevents accumulation of dozens of off-screen enemies
 * - Reduces overall tick overhead
 * - Frees memory for new spawns ahead of player
 *
 * Algorithm:
 * 1. Check if player reference is valid
 * 2. Compare X positions (forward axis in 2.5D side-scroller)
 * 3. If enemy is CleanupDistance behind player, destroy self
 *
 * Performance:
 * - Single 1D comparison (X-axis only)
 * - No expensive distance calculations
 * - Executes every frame but negligible cost (~0.002ms)
 *
 * Typical Values:
 * - CleanupDistance: 2000 units (default)
 * - At 300 units/s speed, enemy exists ~6.6 seconds after passing
 *
 * Edge Cases:
 * - If player moves backward, enemy won't cleanup (by design)
 * - If PlayerRef is invalid, no cleanup occurs (safe fallback)
 */
void ASimpleEnemy::CleanupIfBehindPlayer()
{
	// Verify player reference is valid
	if (!PlayerRef)
	{
		return; // No player = no cleanup check (rare edge case)
	}

	// Get X positions (forward axis for side-scrolling)
	const float PlayerX = PlayerRef->GetActorLocation().X;
	const float EnemyX = GetActorLocation().X;

	// Destroy if enemy is cleanup distance behind player
	if (EnemyX < (PlayerX - CleanupDistance))
	{
		// Safe destruction (removes from world, triggers garbage collection)
		Destroy();

		// Debug logging (disable in shipping builds for performance)
		#if !UE_BUILD_SHIPPING
		UE_LOG(LogTemp, Verbose, TEXT("SimpleEnemy: Cleaned up at X=%.1f (Player at X=%.1f)"), EnemyX, PlayerX);
		#endif
	}
}

/**
 * OnOverlapBegin - Handles collision with player for damage dealing.
 *
 * Damage Dealing Flow:
 * 1. Verify overlapping actor is the player (Cast<ARunnerCharacter>)
 * 2. Check multi-hit prevention flag (bHasDealtDamage)
 * 3. Get player's health component
 * 4. Call TakeDamage() with ContactDamage and EnemyMelee type
 * 5. Set cooldown flag and start timer for reset
 *
 * Cooldown System:
 * - Uses timer with lambda for clean reset logic
 * - Duration: 1.5 seconds (EnemyConstants::DamageCooldownDuration)
 * - Prevents rapid multi-hit during prolonged contact
 * - Respects player's invulnerability frames automatically
 *
 * Integration with PlayerHealthComponent:
 * - Uses EDamageType::EnemyMelee for proper categorization
 * - PlayerHealthComponent handles invulnerability logic
 * - Triggers health changed delegate for UI updates
 *
 * Performance:
 * - Event-driven (not polled), only executes on overlap
 * - Lambda capture for timer callback (modern C++ pattern)
 * - Minimal overhead per collision
 *
 * Error Handling:
 * - Null checks for player, health component
 * - Logs warnings in development builds
 * - Fails gracefully (no damage) if validation fails
 *
 * @param OverlappedComponent The collision box that detected overlap
 * @param OtherActor The actor entering the collision box
 * @param OtherComp The component of the other actor
 * @param OtherBodyIndex Body index for multi-body meshes
 * @param bFromSweep True if overlap detected during sweep trace
 * @param SweepResult Hit result data if from sweep
 */
void ASimpleEnemy::OnOverlapBegin(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	// ========================================
	// VALIDATION: PLAYER CHECK
	// ========================================

	// Attempt to cast overlapping actor to player character
	ARunnerCharacter* Player = Cast<ARunnerCharacter>(OtherActor);

	if (!Player)
	{
		// Not the player - could be another enemy, projectile, etc.
		return;
	}

	// ========================================
	// COOLDOWN CHECK
	// ========================================

	// Prevent multi-hit during cooldown period
	if (bHasDealtDamage)
	{
		return; // Cooldown active, skip damage
	}

	// ========================================
	// DAMAGE DEALING
	// ========================================

	// Get player's health component
	UPlayerHealthComponent* HealthComp = Player->HealthComponent;

	if (!HealthComp)
	{
		// Player exists but health component missing (shouldn't happen)
		UE_LOG(LogTemp, Warning, TEXT("SimpleEnemy: Player has no HealthComponent!"));
		return;
	}

	// Deal damage via health component (type: EnemyMelee)
	HealthComp->TakeDamage(ContactDamage, EDamageType::EnemyMelee);

	// Log damage in development builds
	#if !UE_BUILD_SHIPPING
	UE_LOG(LogTemp, Log, TEXT("SimpleEnemy: Dealt %d damage to player (Type: EnemyMelee)"), ContactDamage);
	#endif

	// ========================================
	// COOLDOWN ACTIVATION
	// ========================================

	// Set flag to prevent multi-hit
	bHasDealtDamage = true;

	// Clear any existing cooldown timer (safety measure)
	if (GetWorldTimerManager().IsTimerActive(DamageCooldownTimer))
	{
		GetWorldTimerManager().ClearTimer(DamageCooldownTimer);
	}

	// Start cooldown timer with lambda callback
	// Lambda captures 'this' to reset member variable
	GetWorldTimerManager().SetTimer(
		DamageCooldownTimer,
		[this]()
		{
			// Reset damage flag after cooldown expires
			bHasDealtDamage = false;

			#if !UE_BUILD_SHIPPING
			UE_LOG(LogTemp, Verbose, TEXT("SimpleEnemy: Damage cooldown reset, can deal damage again"));
			#endif
		},
		EnemyConstants::DamageCooldownDuration,
		false // Non-looping (one-shot timer)
	);
}
