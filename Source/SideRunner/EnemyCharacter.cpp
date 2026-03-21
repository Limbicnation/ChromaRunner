// Copyright ChromaRunner. All Rights Reserved.

#include "EnemyCharacter.h"

#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PaperFlipbookComponent.h"
#include "PaperFlipbook.h"
#include "TimerManager.h"
#include "Engine/DamageEvents.h"

DEFINE_LOG_CATEGORY(LogSideRunnerEnemy);

// Patrol timer frequency — 60Hz movement update without using Tick
static constexpr float PATROL_STEP_INTERVAL = 1.0f / 60.0f;

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
		TEXT("Enemy [%s] spawned at %s, PatrolDist=%.0f, Speed=%.0f"),
		*GetName(), *PatrolOrigin.ToString(), PatrolDistance, PatrolSpeed);

	// Start patrol on next frame (allow physics to settle)
	StartPatrol();
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
// Patrol System (Timer-Driven — NO Tick)
// ============================================================================

void AEnemyCharacter::StartPatrol()
{
	if (bIsDead) return;

	bIsPatrolling = true;
	PatrolDirection = 1.0f;

	GetWorldTimerManager().SetTimer(
		PatrolTimerHandle,
		this,
		&AEnemyCharacter::PatrolStep,
		PATROL_STEP_INTERVAL,
		true // Looping
	);
}

void AEnemyCharacter::PatrolStep()
{
	if (bIsDead || !bIsPatrolling) return;

	const FVector CurrentLocation = GetActorLocation();
	const float DistanceFromOrigin = CurrentLocation.Y - PatrolOrigin.Y;

	// Check if we've reached a patrol boundary
	if (FMath::Abs(DistanceFromOrigin) >= PatrolDistance)
	{
		PauseAtEndpoint();
		return;
	}

	// Move along Y axis (side-scroller direction)
	AddMovementInput(FVector(0.0f, PatrolDirection, 0.0f), 1.0f);

	UpdateSpriteDirection();
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
