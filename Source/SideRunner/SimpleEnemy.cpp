// Fill out your copyright notice in the Description page of Project Settings.

#include "SimpleEnemy.h"
#include "Components/BoxComponent.h"
#include "SideRunner.h" // Custom log categories
#include "Components/StaticMeshComponent.h"
#include "RunnerCharacter.h"
#include "PlayerHealth.h"
#include "GroundPatrol.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

// Performance-critical constants - evaluated at compile time
namespace EnemyConstants
{
	/** Collision box half-extents (X, Y, Z) in centimeters */
	static const FVector CollisionBoxExtent{50.0f, 50.0f, 100.0f};

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

	// ========================================
	// PATROL COMPONENT SETUP
	// ========================================

	PatrolComponent = CreateDefaultSubobject<UGroundPatrolComponent>(TEXT("PatrolComponent"));
}

EPatrolDirection ASimpleEnemy::GetPatrolDirection() const
{
	if (PatrolComponent)
	{
		return PatrolComponent->GetPatrolDirection();
	}
	return EPatrolDirection::Forward;
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
		UE_LOG(LogSideRunnerCombat, Warning, TEXT("SimpleEnemy: Failed to find player character at BeginPlay. Damage and cleanup disabled."));
	}

	// ========================================
	// STORE PATROL START LOCATION
	// ========================================

	// Cache spawn location for cleanup distance calculation
	StartLocation = GetActorLocation();

	// Enable patrol movement via the component
	if (PatrolComponent)
	{
		PatrolComponent->SetPatrolEnabled(true);
	}

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
		UE_LOG(LogSideRunnerCombat, Error, TEXT("SimpleEnemy: CollisionBox is null at BeginPlay!"));
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

	// Patrol movement is now handled by UGroundPatrolComponent (TickComponent)

	// Perform cleanup check for performance optimization
	CleanupIfBehindPlayer();
}

/**
 * CleanupIfBehindPlayer - Destroys enemy when too far behind player.
 * Performance optimization — prevents accumulation of off-screen enemies.
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
		UE_LOG(LogSideRunnerCombat, Verbose, TEXT("SimpleEnemy: Cleaned up at X=%.1f (Player at X=%.1f)"), EnemyX, PlayerX);
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
	UPlayerHealth* HealthComp = Player->HealthComponent;

	if (!HealthComp)
	{
		// Player exists but health component missing (shouldn't happen)
		UE_LOG(LogSideRunnerCombat, Warning, TEXT("SimpleEnemy: Player has no HealthComponent!"));
		return;
	}

	// Deal damage via health component (type: EnemyMelee)
	HealthComp->TakeDamage(ContactDamage, EDamageType::EnemyMelee);

	// Log damage in development builds
	#if !UE_BUILD_SHIPPING
	UE_LOG(LogSideRunnerCombat, Log, TEXT("SimpleEnemy: Dealt %d damage to player (Type: EnemyMelee)"), ContactDamage);
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
			UE_LOG(LogSideRunnerCombat, Verbose, TEXT("SimpleEnemy: Damage cooldown reset, can deal damage again"));
			#endif
		},
		EnemyConstants::DamageCooldownDuration,
		false // Non-looping (one-shot timer)
	);
}
