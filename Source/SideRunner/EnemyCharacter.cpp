// Copyright ChromaRunner. All Rights Reserved.

#include "EnemyCharacter.h"

#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PaperFlipbookComponent.h"
#include "PaperFlipbook.h"
#include "TimerManager.h"
#include "Engine/DamageEvents.h"
#include "Algo/Accumulate.h"

DEFINE_LOG_CATEGORY(LogSideRunnerEnemy);

// Patrol timer frequency — 60Hz movement update without using Tick
static constexpr float PATROL_STEP_INTERVAL = 1.0f / 60.0f;

// Default distance threshold to consider a waypoint "reached" (legacy constant for backward compatibility)
static constexpr float WAYPOINT_ARRIVAL_THRESHOLD = 10.0f;

AEnemyCharacter::AEnemyCharacter()
{
	// PERF: No tick needed — patrol driven by timer
	PrimaryActorTick.bCanEverTick = false;

	// --- Capsule (root collision) ---
	GetCapsuleComponent()->InitCapsuleSize(34.0f, 44.0f);
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));

	// --- Movement ---
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (MoveComp)
	{
		MoveComp->MaxWalkSpeed = PatrolSpeed;
		MoveComp->GravityScale = 1.0f;
		MoveComp->bOrientRotationToMovement = false; // 2D — we flip sprite manually
		MoveComp->bConstrainToPlane = true;
		MoveComp->SetPlaneConstraintAxisSetting(EPlaneConstraintAxisSetting::X); // Lock X for 2.5D
		MoveComp->SetPlaneConstraintEnabled(true);
	}

	// --- Sprite Visual ---
	EnemySprite = CreateDefaultSubobject<UPaperFlipbookComponent>(TEXT("EnemySprite"));
	if (EnemySprite)
	{
		EnemySprite->SetupAttachment(GetCapsuleComponent());
		EnemySprite->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
		// Ensure sprite renders in front of background
		EnemySprite->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
	}

	// --- Damage Zone (body — hurts player) ---
	DamageZone = CreateDefaultSubobject<UBoxComponent>(TEXT("DamageZone"));
	if (DamageZone)
	{
		DamageZone->SetupAttachment(GetCapsuleComponent());
		DamageZone->SetBoxExtent(FVector(32.0f, 32.0f, 36.0f));
		DamageZone->SetRelativeLocation(FVector(0.0f, 0.0f, -8.0f)); // Offset down from center
		DamageZone->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
		DamageZone->SetGenerateOverlapEvents(true);
		DamageZone->ComponentTags.Add(TEXT("EnemyDamage"));
	}

	// --- Stomp Zone (head — player defeats enemy by landing here) ---
	StompZone = CreateDefaultSubobject<UBoxComponent>(TEXT("StompZone"));
	if (StompZone)
	{
		StompZone->SetupAttachment(GetCapsuleComponent());
		StompZone->SetBoxExtent(FVector(28.0f, 28.0f, 12.0f));
		StompZone->SetRelativeLocation(FVector(0.0f, 0.0f, 38.0f)); // Top of capsule
		StompZone->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
		StompZone->SetGenerateOverlapEvents(true);
		StompZone->ComponentTags.Add(TEXT("EnemyStomp"));
	}

	// --- Actor Config ---
	Tags.Add(TEXT("Enemy"));
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Store spawn origin for patrol bounds
	PatrolOrigin = GetActorLocation();

	// CONVERT LEGACY WORLD-SPACE WAYPOINTS TO LOCAL SPACE
	// If designer authored world-space waypoints, convert them to local offsets
	if (!bWaypointsAreLocalSpace && !PatrolWaypoints.IsEmpty())
	{
		for (FVector& Waypoint : PatrolWaypoints)
		{
			Waypoint = Waypoint - PatrolOrigin;
		}
		bWaypointsAreLocalSpace = true;

		UE_LOG(LogSideRunnerEnemy, Log,
			TEXT("Enemy [%s] converted %d legacy world-space waypoints to local offsets"),
			*GetName(), PatrolWaypoints.Num());
	}

	// Apply tuned speed to movement component
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = PatrolSpeed;
	}

	// Set initial flipbook
	if (EnemySprite && WalkFlipbook)
	{
		EnemySprite->SetFlipbook(WalkFlipbook);
		EnemySprite->SetLooping(true);
		EnemySprite->Play();
	}

	// Bind overlap delegates
	if (DamageZone)
	{
		DamageZone->OnComponentBeginOverlap.AddDynamic(this, &AEnemyCharacter::OnDamageZoneOverlap);
	}
	if (StompZone)
	{
		StompZone->OnComponentBeginOverlap.AddDynamic(this, &AEnemyCharacter::OnStompZoneOverlap);
	}

    UE_LOG(LogSideRunnerEnemy, Log,
        TEXT("Enemy [%s] spawned at %s, PatrolDist=%.0f, Speed=%.0f, Waypoints=%d, Mode=%s"),
        *GetName(), *PatrolOrigin.ToString(), PatrolDistance, PatrolSpeed,
        PatrolWaypoints.Num(),
        *UEnum::GetValueAsString(TraversalMode));

	// Start patrol on next frame (allow physics to settle)
	StartPatrolFromBeginning();
}

void AEnemyCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Clean up ALL timers to prevent dangling callbacks
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(PatrolTimerHandle);
		World->GetTimerManager().ClearTimer(PauseTimerHandle);
		World->GetTimerManager().ClearTimer(DeathTimerHandle);
	}

	// Unbind delegates
	if (DamageZone)
	{
		DamageZone->OnComponentBeginOverlap.RemoveDynamic(this, &AEnemyCharacter::OnDamageZoneOverlap);
	}
	if (StompZone)
	{
		StompZone->OnComponentBeginOverlap.RemoveDynamic(this, &AEnemyCharacter::OnStompZoneOverlap);
	}

	Super::EndPlay(EndPlayReason);
}

// ============================================================================
// Patrol Node System
// ============================================================================

int32 AEnemyCharacter::CalculatePatrolMetric()
{
	if (PatrolNodes.IsEmpty())
	{
		UE_LOG(LogSideRunnerEnemy, Log, TEXT("CalculatePatrolMetric: PatrolNodes is empty, returning 0"));
		TotalPatrolDuration = 0;
		return 0;
	}

	if (!ValidatePatrolArrays())
	{
		UE_LOG(LogSideRunnerEnemy, Warning,
			TEXT("CalculatePatrolMetric: PatrolNodes(%d) and PatrolWaypoints(%d) size mismatch"),
			PatrolNodes.Num(), PatrolWaypoints.Num());
	}

	TotalPatrolDuration = Algo::Accumulate(PatrolNodes, 0);

	UE_LOG(LogSideRunnerEnemy, Log,
		TEXT("CalculatePatrolMetric: %d nodes summed -> TotalPatrolDuration = %d"),
		PatrolNodes.Num(), TotalPatrolDuration);

	return TotalPatrolDuration;
}

bool AEnemyCharacter::MoveToPatrolNode(int32 NodeIndex)
{
	if (!ensureMsgf(!PatrolWaypoints.IsEmpty(), TEXT("MoveToPatrolNode: PatrolWaypoints is empty")))
	{
		UE_LOG(LogSideRunnerEnemy, Warning, TEXT("MoveToPatrolNode: PatrolWaypoints empty, cannot move"));
		return false;
	}

	if (!ensureMsgf(NodeIndex >= 0 && NodeIndex < PatrolWaypoints.Num(),
		TEXT("MoveToPatrolNode: NodeIndex %d out of range [0, %d)"), NodeIndex, PatrolWaypoints.Num()))
	{
		UE_LOG(LogSideRunnerEnemy, Warning,
			TEXT("MoveToPatrolNode: Invalid NodeIndex %d (valid: 0-%d)"),
			NodeIndex, PatrolWaypoints.Num() - 1);
		return false;
	}

	if (bIsDead)
	{
		UE_LOG(LogSideRunnerEnemy, Log, TEXT("MoveToPatrolNode: Enemy is dead, ignoring move request"));
		return false;
	}

	// Stop timer-driven patrol to prevent dual movement conflict
	StopPatrolTimer();

	// Set CurrentNodeIndex BEFORE calling GetCurrentPatrolTarget()
	CurrentNodeIndex = NodeIndex;

	const FVector CurrentLocation = GetActorLocation();
	const FVector TargetLocation = GetCurrentPatrolTarget();

	// Update PatrolDirection so UpdateSpriteDirection faces the correct way
	const float YDirection = TargetLocation.Y - CurrentLocation.Y;
	if (!FMath::IsNearlyZero(YDirection))
	{
		PatrolDirection = (YDirection > 0.0f) ? 1.0f : -1.0f;
	}

	// Direct movement toward waypoint (no navmesh needed for 2.5D side-scroller)
	const FVector Direction = (TargetLocation - CurrentLocation).GetSafeNormal();
	const FVector Movement(0.0f, Direction.Y * PatrolSpeed * 0.016f, 0.0f);
	const FVector NewLocation = CurrentLocation + Movement;
	
	// Use TeleportTo instead of SetActorLocation for Characters
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->TeleportTo(NewLocation);
	}
	else
	{
		SetActorLocation(NewLocation);
	}
	UpdateSpriteDirection();

	UE_LOG(LogSideRunnerEnemy, Log,
		TEXT("MoveToPatrolNode: Moving to node %d at %s"),
		NodeIndex, *TargetLocation.ToString());

	return true;
}

void AEnemyCharacter::SetTraversalMode(EPatrolTraversalMode NewMode)
{
    if (TraversalMode == NewMode)
    {
        return;
    }

    TraversalMode = NewMode;

    // Reset reverse flag when switching to Loop (prevents getting stuck)
    if (TraversalMode == EPatrolTraversalMode::Loop)
    {
        bWaypointReverse = false;
    }

    UE_LOG(LogSideRunnerEnemy, Log, TEXT("Traversal mode changed to %s"),
        *UEnum::GetValueAsString(TraversalMode));
}

// ============================================================================
// Patrol System (Timer-Driven — NO Tick)
// ============================================================================

void AEnemyCharacter::StopPatrolTimer()
{
	bIsPatrolling = false;
	GetWorldTimerManager().ClearTimer(PatrolTimerHandle);
	GetWorldTimerManager().ClearTimer(PauseTimerHandle);
}

bool AEnemyCharacter::ValidatePatrolArrays() const
{
	if (PatrolWaypoints.IsEmpty())
	{
		// PatrolNodes alone is valid — waypoint navigation is optional
		return true;
	}
	return PatrolNodes.Num() == PatrolWaypoints.Num();
}

void AEnemyCharacter::StartPatrol()
{
	if (bIsDead) return;

	bIsPatrolling = true;

	GetWorldTimerManager().SetTimer(
		PatrolTimerHandle,
		this,
		&AEnemyCharacter::PatrolStep,
		PATROL_STEP_INTERVAL,
		true // Looping
	);
}

void AEnemyCharacter::StartPatrolFromBeginning()
{
	if (bIsDead) return;

	bIsPatrolling = true;
	PatrolDirection = 1.0f;
	bWaypointReverse = false;
	CurrentNodeIndex = 0;

	StartPatrol();
}

void AEnemyCharacter::PatrolStep()
{
	if (bIsDead || !bIsPatrolling) return;

	// Branch: waypoint patrol vs. simple origin-based patrol
	if (!PatrolWaypoints.IsEmpty())
	{
		PatrolStepWaypoint();
	}
	else
	{
		PatrolStepSimple();
	}
}

// ----------------------------------------------------------------------------
// Simple Patrol (fallback when PatrolWaypoints is empty)
// ----------------------------------------------------------------------------

void AEnemyCharacter::PatrolStepSimple()
{
	const FVector CurrentLocation = GetActorLocation();
	const float DistanceFromOrigin = CurrentLocation.Y - PatrolOrigin.Y;

	// Check if we've reached a patrol boundary
	if (FMath::Abs(DistanceFromOrigin) >= PatrolDistance)
	{
		PauseAtEndpoint();
		return;
	}

	// Move along Y axis (side-scroller direction)
	const FVector PatrolMovement(0.0f, PatrolDirection * PatrolSpeed * PATROL_STEP_INTERVAL, 0.0f);
	const FVector NewLocation = GetActorLocation() + PatrolMovement;
	
	// Use TeleportTo instead of SetActorLocation for Characters
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->TeleportTo(NewLocation);
	}
	else
	{
		SetActorLocation(NewLocation);
	}

	UpdateSpriteDirection();
}

// ----------------------------------------------------------------------------
// Waypoint Patrol (uses PatrolWaypoints array with traversal mode)
// ----------------------------------------------------------------------------

void AEnemyCharacter::PatrolStepWaypoint()
{
	// VALIDATE: Ensure we have waypoints to patrol
	if (PatrolWaypoints.IsEmpty())
	{
		UE_LOG(LogSideRunnerEnemy, Warning, TEXT("PatrolStepWaypoint: No waypoints configured, falling back to simple patrol"));
		PatrolStepSimple();
		return;
	}

	// VALIDATE: Ensure current index is within bounds (handles runtime array modification)
	if (!PatrolWaypoints.IsValidIndex(CurrentNodeIndex))
	{
		UE_LOG(LogSideRunnerEnemy, Warning, TEXT("PatrolStepWaypoint: CurrentNodeIndex %d out of bounds (array size: %d), resetting to 0"),
			CurrentNodeIndex, PatrolWaypoints.Num());
		CurrentNodeIndex = 0;
		bWaypointReverse = false;
		PatrolDirection = 1.0f;
	}

	// VALIDATE: Minimum waypoints required for patrol
	if (PatrolWaypoints.Num() < MIN_WAYPOINT_COUNT)
	{
		UE_LOG(LogSideRunnerEnemy, Warning, TEXT("PatrolStepWaypoint: Only %d waypoint(s) configured (minimum: %d), enemy will stay at first waypoint"),
			PatrolWaypoints.Num(), MIN_WAYPOINT_COUNT);
		// Stay at first waypoint without moving
		return;
	}

	const FVector CurrentLocation = GetActorLocation();
	const FVector TargetLocation = GetCurrentPatrolTarget();

	// ARRIVAL CHECK: Use squared distance for performance (avoids sqrt)
	// AcceptanceRadius is exposed to Blueprint, allowing per-enemy tuning
	const float ArrivalThresholdSquared = AcceptanceRadius * AcceptanceRadius;
	const float DistToTargetSquared = FVector::DistSquared(CurrentLocation, TargetLocation);

	if (DistToTargetSquared <= ArrivalThresholdSquared)
	{
		// Store previous index for debugging
		PreviousNodeIndex = CurrentNodeIndex;

		UE_LOG(LogSideRunnerEnemy, Verbose, TEXT("PatrolStepWaypoint: Arrived at waypoint %d at %s (dist: %.2f)"),
			CurrentNodeIndex, *TargetLocation.ToString(), FMath::Sqrt(DistToTargetSquared));

		// Check if we should pause at terminal waypoints (PingPong mode)
		const bool bAtTerminalPoint = (CurrentNodeIndex == 0) || (CurrentNodeIndex == PatrolWaypoints.Num() - 1);

		if (bPauseAtWaypoints && bAtTerminalPoint && TraversalMode == EPatrolTraversalMode::PingPong)
		{
			// Pause before advancing
			UE_LOG(LogSideRunnerEnemy, Log, TEXT("PatrolStepWaypoint: Pausing at terminal waypoint %d before reversal"),
				CurrentNodeIndex);
			PauseAtEndpoint();
			return;
		}

		// Advance to next waypoint immediately (no pause)
		AdvanceWaypointIndex();

		// FALL THROUGH to move toward the newly selected waypoint
	}

	// MOVEMENT: Calculate direction toward current waypoint
	const FVector NewTargetLocation = GetCurrentPatrolTarget();
	const FVector Direction = (NewTargetLocation - CurrentLocation).GetSafeNormal();

	if (Direction.IsNearlyZero())
	{
		// Already at target or invalid direction
		return;
	}

	// Frame-rate independent movement: speed * timer_interval (60Hz = ~0.0167s)
	const float YDelta = Direction.Y * PatrolSpeed * PATROL_STEP_INTERVAL;

	// Update PatrolDirection for sprite facing
	const float NewPatrolDirection = (YDelta >= 0.0f) ? 1.0f : -1.0f;
	if (PatrolDirection != NewPatrolDirection)
	{
		PatrolDirection = NewPatrolDirection;
		UE_LOG(LogSideRunnerEnemy, Verbose, TEXT("PatrolStepWaypoint: Sprite facing changed to %s"),
			PatrolDirection >= 0.0f ? TEXT("right") : TEXT("left"));
	}

	const FVector PatrolMovement(0.0f, YDelta, 0.0f);
	const FVector NewLocation = CurrentLocation + PatrolMovement;
	
	// Use TeleportTo instead of SetActorLocation for Characters
	// CharacterMovementComponent overrides direct location changes
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->TeleportTo(NewLocation);
	}
	else
	{
		SetActorLocation(NewLocation);
	}

	UpdateSpriteDirection();
}

FVector AEnemyCharacter::GetCurrentPatrolTarget() const
{
	if (!PatrolWaypoints.IsValidIndex(CurrentNodeIndex))
	{
		return PatrolOrigin;
	}

	const FVector& WaypointOrOffset = PatrolWaypoints[CurrentNodeIndex];

	if (bWaypointsAreLocalSpace)
	{
		// Convert local offset to world space
		return PatrolOrigin + WaypointOrOffset;
	}
	else
	{
		// Legacy world-space waypoint
		return WaypointOrOffset;
	}
}

void AEnemyCharacter::AdvanceWaypointIndex()
{
	// VALIDATE: Ensure we have waypoints to patrol
	if (PatrolWaypoints.IsEmpty())
	{
		UE_LOG(LogSideRunnerEnemy, Warning, TEXT("AdvanceWaypointIndex: No waypoints configured, cannot advance"));
		return;
	}

	// EDGE CASE: Single waypoint — stay at index 0
	if (PatrolWaypoints.Num() == 1)
	{
		if (CurrentNodeIndex != 0)
		{
			UE_LOG(LogSideRunnerEnemy, Warning, TEXT("AdvanceWaypointIndex: Only 1 waypoint, resetting index to 0"));
			CurrentNodeIndex = 0;
		}
		return;
	}

	// EDGE CASE: Two waypoints — special handling for ping-pong
	if (PatrolWaypoints.Num() == 2 && TraversalMode == EPatrolTraversalMode::PingPong)
	{
		// Simply toggle between 0 and 1
		CurrentNodeIndex = (CurrentNodeIndex == 0) ? 1 : 0;
		PatrolDirection = (CurrentNodeIndex == 0) ? -1.0f : 1.0f;

		UE_LOG(LogSideRunnerEnemy, Log, TEXT("AdvanceWaypointIndex: 2-waypoint ping-pong, now targeting node %d"),
			CurrentNodeIndex);
		return;
	}

	// MAIN LOGIC: Handle each traversal mode
	if (TraversalMode == EPatrolTraversalMode::PingPong)
	{
		// PingPong: 0->1->2->...->N->N-1->...->0->1...
		if (!bWaypointReverse)
		{
			// Moving forward through the array
			if (CurrentNodeIndex >= PatrolWaypoints.Num() - 1)
			{
				// REVERSAL: Reached last waypoint, switch to reverse traversal
				bWaypointReverse = true;
				CurrentNodeIndex = FMath::Max(0, PatrolWaypoints.Num() - 2); // Clamp to prevent negative
				PatrolDirection = -1.0f;

				UE_LOG(LogSideRunnerEnemy, Log, TEXT("AdvanceWaypointIndex: REVERSAL at last waypoint (%d), now targeting %d in reverse"),
					PatrolWaypoints.Num() - 1, CurrentNodeIndex);
			}
			else
			{
				// Normal forward advancement
				++CurrentNodeIndex;

				UE_LOG(LogSideRunnerEnemy, Verbose, TEXT("AdvanceWaypointIndex: Forward advance to node %d"),
					CurrentNodeIndex);
			}
		}
		else
		{
			// Moving backward through the array
			if (CurrentNodeIndex <= 0)
			{
				// REVERSAL: Reached first waypoint, switch to forward traversal
				bWaypointReverse = false;
				CurrentNodeIndex = FMath::Min(PatrolWaypoints.Num() - 1, 1); // Clamp to prevent overflow
				PatrolDirection = 1.0f;

				UE_LOG(LogSideRunnerEnemy, Log, TEXT("AdvanceWaypointIndex: REVERSAL at first waypoint (0), now targeting %d in forward"),
					CurrentNodeIndex);
			}
			else
			{
				// Normal backward advancement
				--CurrentNodeIndex;

				UE_LOG(LogSideRunnerEnemy, Verbose, TEXT("AdvanceWaypointIndex: Backward advance to node %d"),
					CurrentNodeIndex);
			}
		}
	}
	else // EPatrolTraversalMode::Loop
	{
		// Loop: 0->1->2->...->N->0->1... (wraps around)
		const int32 OldIndex = CurrentNodeIndex;
		CurrentNodeIndex = (CurrentNodeIndex + 1) % PatrolWaypoints.Num();

		// Maintain forward direction for visual consistency
		PatrolDirection = 1.0f;

		if (OldIndex == PatrolWaypoints.Num() - 1)
		{
			UE_LOG(LogSideRunnerEnemy, Log, TEXT("AdvanceWaypointIndex: LOOP wrap from %d to %d"),
				OldIndex, CurrentNodeIndex);
		}
		else
		{
			UE_LOG(LogSideRunnerEnemy, Verbose, TEXT("AdvanceWaypointIndex: Loop advance to node %d"),
				CurrentNodeIndex);
		}
	}

	// FINAL SAFETY: Clamp index to valid range (defensive programming)
	if (!PatrolWaypoints.IsValidIndex(CurrentNodeIndex))
	{
		UE_LOG(LogSideRunnerEnemy, Error, TEXT("AdvanceWaypointIndex: Index %d out of bounds after update! Array size: %d. Clamping."),
			CurrentNodeIndex, PatrolWaypoints.Num());
		CurrentNodeIndex = FMath::Clamp(CurrentNodeIndex, 0, PatrolWaypoints.Num() - 1);
	}
}

void AEnemyCharacter::PauseAtEndpoint()
{
	bIsPatrolling = false;

	// Stop the patrol timer
	GetWorldTimerManager().ClearTimer(PatrolTimerHandle);

	// Switch to idle flipbook during pause
	if (EnemySprite && IdleFlipbook)
	{
		EnemySprite->SetFlipbook(IdleFlipbook);
	}

	// Reverse direction and resume after pause
	PatrolDirection *= -1.0f;

	GetWorldTimerManager().SetTimer(
		PauseTimerHandle,
		this,
		&AEnemyCharacter::ResumePatrol,
		PatrolPauseTime,
		false // One-shot
	);
}

void AEnemyCharacter::ResumePatrol()
{
	if (bIsDead) return;

	// Switch back to walk flipbook
	if (EnemySprite && WalkFlipbook)
	{
		EnemySprite->SetFlipbook(WalkFlipbook);
	}

	UpdateSpriteDirection();
	StartPatrol();
}

void AEnemyCharacter::UpdateSpriteDirection()
{
	if (!EnemySprite) return;

	// Flip sprite based on patrol direction
	// PatrolDirection > 0 = moving right (positive Y), < 0 = moving left
	const FVector CurrentScale = EnemySprite->GetRelativeScale3D();
	const float DesiredYScale = (PatrolDirection >= 0.0f) ? FMath::Abs(CurrentScale.Y) : -FMath::Abs(CurrentScale.Y);

	if (!FMath::IsNearlyEqual(CurrentScale.Y, DesiredYScale))
	{
		EnemySprite->SetRelativeScale3D(FVector(CurrentScale.X, DesiredYScale, CurrentScale.Z));
	}
}

// ============================================================================
// Combat
// ============================================================================

void AEnemyCharacter::OnDamageZoneOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (bIsDead) return;
	if (!IsValid(OtherActor)) return;

	// Only damage the player character
	APawn* PlayerPawn = Cast<APawn>(OtherActor);
	if (!PlayerPawn || !PlayerPawn->IsPlayerControlled()) return;

	// Check if player is falling onto us (stomp) — if so, don't damage
	if (UCharacterMovementComponent* PlayerMove = Cast<UCharacterMovementComponent>(PlayerPawn->GetMovementComponent()))
	{
		if (PlayerMove->IsFalling() && PlayerMove->Velocity.Z < -50.0f)
		{
			// Player is falling down onto us — let StompZone handle it
			return;
		}
	}

	UE_LOG(LogSideRunnerEnemy, Log, TEXT("Enemy [%s] damaged player [%s]"), *GetName(), *OtherActor->GetName());

	// Apply damage via Unreal's damage system
	FDamageEvent DamageEvent;
	OtherActor->TakeDamage(static_cast<float>(ContactDamage), DamageEvent, nullptr, this);

	// BP event for FX/audio
	OnDamagedPlayer(OtherActor);
}

void AEnemyCharacter::OnStompZoneOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (bIsDead) return;
	if (!IsValid(OtherActor)) return;

	APawn* PlayerPawn = Cast<APawn>(OtherActor);
	if (!PlayerPawn || !PlayerPawn->IsPlayerControlled()) return;

	// Player must be falling to stomp
	if (UCharacterMovementComponent* PlayerMove = Cast<UCharacterMovementComponent>(PlayerPawn->GetMovementComponent()))
	{
		if (PlayerMove->Velocity.Z < -50.0f)
		{
			UE_LOG(LogSideRunnerEnemy, Log, TEXT("Player stomped enemy [%s]!"), *GetName());

			// Bounce the player upward
			PlayerMove->Velocity.Z = 600.0f;

			DefeatEnemy();
		}
	}
}

void AEnemyCharacter::DefeatEnemy()
{
	if (bIsDead) return;
	bIsDead = true;
	bIsPatrolling = false;

	// Stop all patrol timers
	GetWorldTimerManager().ClearTimer(PatrolTimerHandle);
	GetWorldTimerManager().ClearTimer(PauseTimerHandle);

	// Disable collision immediately
	if (DamageZone) DamageZone->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (StompZone) StompZone->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Stop movement
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->StopMovementImmediately();
		MoveComp->DisableMovement();
	}

	// Play death flipbook if available, then destroy
	if (EnemySprite && DeathFlipbook)
	{
		EnemySprite->SetFlipbook(DeathFlipbook);
		EnemySprite->SetLooping(false);
		EnemySprite->PlayFromStart();

		// Destroy after death animation finishes
		const float DeathDuration = DeathFlipbook->GetTotalDuration();
		GetWorldTimerManager().SetTimer(
			DeathTimerHandle,
			FTimerDelegate::CreateLambda([WeakThis = TWeakObjectPtr<AEnemyCharacter>(this)]()
			{
				if (WeakThis.IsValid())
				{
					WeakThis->Destroy();
				}
			}),
			DeathDuration + 0.1f,
			false
		);
	}
	else
	{
		// No death anim — destroy immediately with small delay for FX
		GetWorldTimerManager().SetTimer(
			DeathTimerHandle,
			FTimerDelegate::CreateLambda([WeakThis = TWeakObjectPtr<AEnemyCharacter>(this)]()
			{
				if (WeakThis.IsValid())
				{
					WeakThis->Destroy();
				}
			}),
			0.15f,
			false
		);
	}

	// BP event for FX/audio (score increment, particles, sound)
	OnEnemyDefeated();

	UE_LOG(LogSideRunnerEnemy, Log, TEXT("Enemy [%s] defeated and scheduled for destruction"), *GetName());
}
