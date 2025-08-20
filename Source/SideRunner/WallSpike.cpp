// Fill out your copyright notice in the Description page of Project Settings.

#include "WallSpike.h"
#include "RunnerCharacter.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerHealthComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/DamageEvents.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"

AWallSpike::AWallSpike()
{
	PrimaryActorTick.bCanEverTick = true;
	
	// Initialize chasing properties with performance-optimized defaults
	ChaseSpeed = 400.0f;
	DirectionalBias = 0.4f;
	ChaseRange = 1500.0f;
	DirectionChangeRate = 2.0f;
	bAccelerateWhenClose = true;
	AccelerationRange = 500.0f;
	MaxSpeedMultiplier = 2.0f;
	
	// Initialize directional properties - DEFAULT TO FORWARD (+Y)
	bUsePresetDirections = true;
	PresetDirectionIndex = 0; // 0 = Forward (+Y)
	PrimaryDirection = FVector(0.0f, 1.0f, 0.0f);
	CustomDirection = FVector(0.0f, 1.0f, 0.0f);
	
	// Initialize lifetime properties
	DeathCleanupDelay = 3.0f;
	MaxDistanceBehindPlayer = 2000.0f;
	MaxTimeBehindPlayer = 10.0f;
	
	// Initialize sound properties
	ChaseStartSound = nullptr;
	ChaseLoopSound = nullptr;
	ChaseVolumeMultiplier = 1.0f;
	ChasePitchMultiplier = 1.0f;
	
	// Set wall spike specific damage (instant death)
	DamageAmount = 9999.0f;
	DamageCooldown = 0.0f;
	
	// OVERRIDE BASE CLASS MOVEMENT - disable base movement system
	MovementType = EMovementType::Static;
	bIsMoving = false;
	Speed = 0.0f;
	
	// Initialize optimized private members
	TargetPlayer = nullptr;
	CurrentDirection = FVector(0.0f, 1.0f, 0.0f);
	PlayerSearchTimer = 0.0f;
	PlayerSearchInterval = 0.5f; // Reduced frequency for better performance
	bHasTarget = false;
	bHasKilledPlayer = false;
	PlayerDeathTimer = 0.0f;
	bTrackingPlayerDeath = false;
	TimeBehindPlayer = 0.0f;
	ChaseAudioComponent = nullptr;
}

void AWallSpike::BeginPlay()
{
	// PERFORMANCE: Call ASpikes::BeginPlay instead of Super::BeginPlay
	ASpikes::BeginPlay();
	
	// CRITICAL: Ensure actor mobility is set to movable AFTER components are initialized
	if (GetRootComponent())
	{
		GetRootComponent()->SetMobility(EComponentMobility::Movable);
#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
		UE_LOG(LogTemp, Warning, TEXT("WallSpike root component mobility set to Movable"));
#endif
	}
	
	// Update primary direction based on settings
	PrimaryDirection = GetPrimaryDirection();
	CurrentDirection = PrimaryDirection;
	
#if UE_BUILD_DEBUG
	// OPTIMIZED DEBUG LOGGING - only in debug builds
	UE_LOG(LogTemp, Log, TEXT("WallSpike initialized at location: %s with direction: %s"), 
		   *GetActorLocation().ToString(), *PrimaryDirection.ToString());
#endif
	
	// Optimized collision setup
	if (CollisionBox)
	{
		CollisionBox->SetMobility(EComponentMobility::Movable);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		CollisionBox->SetCollisionProfileName(TEXT("BlockAll"));
		CollisionBox->SetNotifyRigidBodyCollision(true);
		CollisionBox->SetGenerateOverlapEvents(true);
		
		// Set specific collision responses for better detection
		CollisionBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
		CollisionBox->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
		
		// Bind collision events
		CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AWallSpike::OnOverlapBegin);
		CollisionBox->OnComponentHit.AddDynamic(this, &AWallSpike::OnHit);
	}
#if UE_BUILD_DEBUG
	else
	{
		UE_LOG(LogTemp, Error, TEXT("WallSpike CollisionBox is null! Check Blueprint setup."));
	}
#endif
	
	// Ensure mesh is movable
	if (SpikeMesh)
	{
		SpikeMesh->SetMobility(EComponentMobility::Movable);
	}
	
	// Initialize player search
	UpdateTargetPlayer();
	
	// Reset state variables
	bHasKilledPlayer = false;
	PlayerDeathTimer = 0.0f;
	bTrackingPlayerDeath = false;
}

void AWallSpike::Tick(float DeltaTime)
{
	// PERFORMANCE: Call AActor::Tick directly to bypass base Spikes movement
	AActor::Tick(DeltaTime);
	
	// PERFORMANCE: Reduce debug output frequency and only in debug builds
#if UE_BUILD_DEBUG
	static float DebugTimer = 0.0f;
	DebugTimer += DeltaTime;
	if (DebugTimer > 5.0f) // Reduced frequency from 2.0f to 5.0f
	{
		if (TargetPlayer)
		{
			UE_LOG(LogTemp, VeryVerbose, TEXT("WallSpike tracking player at distance: %.1f"), 
				   FVector::Dist(GetActorLocation(), TargetPlayer->GetActorLocation()));
		}
		DebugTimer = 0.0f;
	}
#endif
	
	// PERFORMANCE: Optimized on-screen debug messages - only in development
#if UE_BUILD_DEVELOPMENT
	if (GEngine && TargetPlayer)
	{
		const FString DebugMsg = FString::Printf(TEXT("WallSpike: %.1f units from player"), 
			FVector::Dist(GetActorLocation(), TargetPlayer->GetActorLocation()));
		// PERFORMANCE: Fix the ambiguous call by using explicit key parameter
		GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Yellow, DebugMsg, false);
	}
#endif
	
	// Handle player search timing - optimized
	PlayerSearchTimer -= DeltaTime;
	if (PlayerSearchTimer <= 0.0f)
	{
		UpdateTargetPlayer();
		PlayerSearchTimer = PlayerSearchInterval;
	}
	
	// Update chasing movement
	UpdateChaseMovement(DeltaTime);
	
	// Check lifetime and cleanup
	CheckLifetimeAndCleanup();
	
	// PERFORMANCE: Editor-only debug visualization
#if WITH_EDITOR
	DrawDebugVisualization();
#endif
}

FVector AWallSpike::GetPrimaryDirection() const
{
	if (bUsePresetDirections)
	{
		// PERFORMANCE: Using switch instead of if-else chain
		switch (PresetDirectionIndex)
		{
		case 0: return FVector(0.0f, 1.0f, 0.0f);  // Forward (+Y)
		case 1: return FVector(0.0f, -1.0f, 0.0f); // Backward (-Y)
		case 2: return FVector(1.0f, 0.0f, 0.0f);  // Right (+X)
		case 3: return FVector(-1.0f, 0.0f, 0.0f); // Left (-X)
		case 4: return FVector(0.0f, 0.0f, 1.0f);  // Up (+Z)
		case 5: return FVector(0.0f, 0.0f, -1.0f); // Down (-Z)
		default: return FVector(0.0f, 1.0f, 0.0f); // Default to Forward
		}
	}
	return CustomDirection.GetSafeNormal();
}

void AWallSpike::UpdateTargetPlayer()
{
	// PERFORMANCE: Cache the player character instead of getting it every time
	static ARunnerCharacter* CachedPlayer = nullptr;
	
	if (!CachedPlayer || !IsValid(CachedPlayer))
	{
		CachedPlayer = Cast<ARunnerCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
	}
	
	if (!CachedPlayer || CachedPlayer->IsDead())
	{
		HandlePlayerDeathOrLoss();
		return;
	}
	
	// PERFORMANCE: Use squared distance to avoid expensive square root calculation
	const FVector PlayerLocation = CachedPlayer->GetActorLocation();
	const FVector SpikeLocation = GetActorLocation();
	const float DistanceSquared = FVector::DistSquared(SpikeLocation, PlayerLocation);
	const float ChaseRangeSquared = ChaseRange * ChaseRange;
	
	if (DistanceSquared <= ChaseRangeSquared)
	{
		const bool bWasHasTarget = bHasTarget;
		TargetPlayer = CachedPlayer;
		bHasTarget = true;
		
		// Handle sound effects for target acquisition
		HandleChaseAudioStart(bWasHasTarget);
		
		// Reset death tracking if we found a living player
		if (bTrackingPlayerDeath)
		{
			bTrackingPlayerDeath = false;
			PlayerDeathTimer = 0.0f;
		}
	}
	else
	{
		HandlePlayerOutOfRange();
	}
}

void AWallSpike::HandlePlayerDeathOrLoss()
{
	if (TargetPlayer && TargetPlayer->IsDead() && !bTrackingPlayerDeath)
	{
		// Start tracking death timer
		bTrackingPlayerDeath = true;
		PlayerDeathTimer = 0.0f;
		StopChaseAudio();
#if UE_BUILD_DEBUG
		UE_LOG(LogTemp, Log, TEXT("WallSpike: Player died, starting cleanup timer"));
#endif
	}
	
	if (!bTrackingPlayerDeath)
	{
		StopChaseAudio();
		TargetPlayer = nullptr;
		bHasTarget = false;
	}
}

void AWallSpike::HandlePlayerOutOfRange()
{
	if (bHasTarget)
	{
#if UE_BUILD_DEBUG
		UE_LOG(LogTemp, VeryVerbose, TEXT("WallSpike lost target - player too far"));
#endif
		StopChaseAudio();
	}
	TargetPlayer = nullptr;
	bHasTarget = false;
}

void AWallSpike::HandleChaseAudioStart(bool bWasHasTarget)
{
	if (!bWasHasTarget && ChaseStartSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ChaseStartSound, GetActorLocation(), 
											 ChaseVolumeMultiplier, ChasePitchMultiplier);
	}
	
	if (!bWasHasTarget && ChaseLoopSound && !ChaseAudioComponent)
	{
		ChaseAudioComponent = UGameplayStatics::SpawnSoundAttached(
			ChaseLoopSound, GetRootComponent(), NAME_None, FVector::ZeroVector,
			EAttachLocation::KeepRelativeOffset, false, 
			ChaseVolumeMultiplier, ChasePitchMultiplier, 0.0f, nullptr, nullptr, true);
		
		if (ChaseAudioComponent && !ChaseAudioComponent->IsPlaying())
		{
			ChaseAudioComponent->Play();
		}
	}
}

void AWallSpike::StopChaseAudio()
{
	if (ChaseAudioComponent && ChaseAudioComponent->IsPlaying())
	{
		ChaseAudioComponent->Stop();
	}
}

FVector AWallSpike::CalculateChaseDirection() const
{
	if (!bHasTarget || !TargetPlayer || TargetPlayer->IsDead())
	{
		return GetPrimaryDirection();
	}
	
	// Calculate direction to player
	const FVector ToPlayer = (TargetPlayer->GetActorLocation() - GetActorLocation()).GetSafeNormal();
	const FVector PrimaryDir = GetPrimaryDirection();
	
	// Blend between chasing player and moving in primary direction
	return FMath::Lerp(ToPlayer, PrimaryDir, DirectionalBias).GetSafeNormal();
}

void AWallSpike::UpdateChaseMovement(float DeltaTime)
{
	// Calculate desired direction
	const FVector DesiredDirection = CalculateChaseDirection();
	
	// Smoothly interpolate current direction towards desired direction
	CurrentDirection = FMath::VInterpTo(CurrentDirection, DesiredDirection, DeltaTime, DirectionChangeRate);
	CurrentDirection.Normalize();
	
	// Calculate movement speed with potential acceleration
	float CurrentSpeed = ChaseSpeed;
	
	if (bAccelerateWhenClose && bHasTarget && TargetPlayer && !TargetPlayer->IsDead())
	{
		// PERFORMANCE: Use squared distance comparison
		const float DistanceSquared = FVector::DistSquared(GetActorLocation(), TargetPlayer->GetActorLocation());
		const float AccelerationRangeSquared = AccelerationRange * AccelerationRange;
		
		if (DistanceSquared <= AccelerationRangeSquared)
		{
			const float Distance = FMath::Sqrt(DistanceSquared); // Only calculate sqrt when needed
			const float AccelerationFactor = FMath::Clamp(1.0f - (Distance / AccelerationRange), 0.0f, 1.0f);
			CurrentSpeed *= FMath::Lerp(1.0f, MaxSpeedMultiplier, AccelerationFactor);
		}
	}
	
	// Calculate and apply movement
	const FVector MovementDelta = CurrentDirection * CurrentSpeed * DeltaTime;
	const FVector NewLocation = GetActorLocation() + MovementDelta;
	
	// Use sweep for collision detection
	FHitResult HitResult;
	const bool bHitSomething = SetActorLocation(NewLocation, true, &HitResult);
	
	// Handle collision with player
	if (bHitSomething && HitResult.GetActor())
	{
		if (ARunnerCharacter* HitPlayer = Cast<ARunnerCharacter>(HitResult.GetActor()))
		{
			if (!bHasKilledPlayer)
			{
#if UE_BUILD_DEBUG
				UE_LOG(LogTemp, Error, TEXT("WallSpike collision detected during movement!"));
#endif
				ApplyInstantDeathToPlayer(HitPlayer, HitResult.Location);
			}
		}
	}
	
	// Additional proximity check for enhanced collision detection
	CheckProximityCollision();
}

void AWallSpike::CheckProximityCollision()
{
	if (!TargetPlayer || bHasKilledPlayer)
		return;
	
	// PERFORMANCE: Use squared distance for proximity check
	const float ProximityThresholdSquared = 150.0f * 150.0f; // 150 units squared
	const float DistanceSquared = FVector::DistSquared(GetActorLocation(), TargetPlayer->GetActorLocation());
	
	if (DistanceSquared < ProximityThresholdSquared)
	{
#if UE_BUILD_DEBUG
		UE_LOG(LogTemp, Warning, TEXT("WallSpike proximity collision detected!"));
#endif
		ApplyInstantDeathToPlayer(TargetPlayer, TargetPlayer->GetActorLocation());
	}
}

void AWallSpike::CheckLifetimeAndCleanup()
{
	// Handle player death cleanup
	if (bTrackingPlayerDeath)
	{
		PlayerDeathTimer += GetWorld()->GetDeltaSeconds();
		if (PlayerDeathTimer >= DeathCleanupDelay)
		{
#if UE_BUILD_DEBUG
			UE_LOG(LogTemp, Log, TEXT("WallSpike destroying self - player dead for %.1fs"), PlayerDeathTimer);
#endif
			Destroy();
			return;
		}
		return;
	}
	
	if (!TargetPlayer)
		return;
	
	// Check if too far behind player - optimized with squared distance
	const FVector PlayerLocation = TargetPlayer->GetActorLocation();
	const FVector SpikeLocation = GetActorLocation();
	const FVector PrimaryDir = GetPrimaryDirection();
	const FVector ToSpike = SpikeLocation - PlayerLocation;
	const float DistanceInOppositeDirection = FVector::DotProduct(ToSpike, -PrimaryDir);
	
	if (DistanceInOppositeDirection > MaxDistanceBehindPlayer)
	{
		const float MovingTowardsPlayer = FVector::DotProduct(CurrentDirection, PrimaryDir);
		if (MovingTowardsPlayer <= 0.0f)
		{
#if UE_BUILD_DEBUG
			UE_LOG(LogTemp, Log, TEXT("WallSpike destroying self - too far behind"));
#endif
			Destroy();
			return;
		}
		
		TimeBehindPlayer += GetWorld()->GetDeltaSeconds();
		if (TimeBehindPlayer >= MaxTimeBehindPlayer)
		{
#if UE_BUILD_DEBUG
			UE_LOG(LogTemp, Log, TEXT("WallSpike destroying self - behind too long"));
#endif
			Destroy();
			return;
		}
	}
	else
	{
		TimeBehindPlayer = 0.0f;
	}
}

void AWallSpike::ApplyInstantDeathToPlayer(ARunnerCharacter* Player, FVector HitLocation)
{
	if (!Player || Player->IsDead() || bHasKilledPlayer)
		return;
	
	bHasKilledPlayer = true;
	
#if UE_BUILD_DEBUG
	UE_LOG(LogTemp, Error, TEXT("WallSpike applying instant death to player!"));
#endif
	
	// Stop chase audio and play death sound
	StopChaseAudio();
	
	if (CollisionSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, CollisionSound, HitLocation, 
											 ChaseVolumeMultiplier * 1.5f, // Louder for death
											 ChasePitchMultiplier * 0.8f);  // Lower pitch for drama
	}
	
	// Show particle effect
	if (ImpactEffect)
	{
		ImpactEffect->SetWorldLocation(HitLocation);
		ImpactEffect->Activate(true);
	}
	
	// Apply damage through health component
	if (UPlayerHealthComponent* HealthComp = Player->GetComponentByClass<UPlayerHealthComponent>())
	{
		const int32 InstantDeathDamage = HealthComp->GetMaxHealth() * 10;
		HealthComp->TakeDamage(InstantDeathDamage, EDamageType::Spikes);
	}
	else
	{
		// Fallback to Unreal's damage system
		FDamageEvent DamageEvent;
		Player->TakeDamage(9999.0f, DamageEvent, nullptr, this);
	}
	
	// Stop chasing and start cleanup
	bHasTarget = false;
	TargetPlayer = nullptr;
	bTrackingPlayerDeath = true;
	PlayerDeathTimer = 0.0f;
	
	// PERFORMANCE: Use lambda with timer for destruction
	FTimerHandle DestroyTimer;
	GetWorld()->GetTimerManager().SetTimer(DestroyTimer, [this]()
	{
		if (IsValid(this))
		{
			Destroy();
		}
	}, 1.0f, false);
}

void AWallSpike::NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, 
	bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);
	
	if (ARunnerCharacter* HitPlayer = Cast<ARunnerCharacter>(Other))
	{
		if (!bHasKilledPlayer)
		{
#if UE_BUILD_DEBUG
			UE_LOG(LogTemp, Warning, TEXT("WallSpike NotifyHit backup collision detection"));
#endif
			ApplyInstantDeathToPlayer(HitPlayer, HitLocation);
		}
	}
}

void AWallSpike::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
							   UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
							   bool bFromSweep, const FHitResult& SweepResult)
{
	if (ARunnerCharacter* HitPlayer = Cast<ARunnerCharacter>(OtherActor))
	{
		if (!bHasKilledPlayer)
		{
#if UE_BUILD_DEBUG
			UE_LOG(LogTemp, Error, TEXT("WallSpike overlap collision detected!"));
#endif
			FVector HitLocation;
			if (SweepResult.IsValidBlockingHit())
			{
				HitLocation = FVector(SweepResult.Location);
			}
			else
			{
				HitLocation = GetActorLocation();
			}
			ApplyInstantDeathToPlayer(HitPlayer, HitLocation);
		}
	}
}

void AWallSpike::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, 
					   UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (ARunnerCharacter* HitPlayer = Cast<ARunnerCharacter>(OtherActor))
	{
		if (!bHasKilledPlayer)
		{
#if UE_BUILD_DEBUG
			UE_LOG(LogTemp, Error, TEXT("WallSpike hit collision detected!"));
#endif
			ApplyInstantDeathToPlayer(HitPlayer, Hit.Location);
		}
	}
}

#if WITH_EDITOR
void AWallSpike::DrawDebugVisualization()
{
	if (!bHasTarget || !TargetPlayer)
		return;
	
	// Draw line to target
	DrawDebugLine(GetWorld(), GetActorLocation(), TargetPlayer->GetActorLocation(), 
				 FColor::Red, false, -1.0f, 0, 2.0f);
	
	// Draw chase range
	DrawDebugSphere(GetWorld(), GetActorLocation(), ChaseRange, 16, FColor::Orange, false, -1.0f, 0, 1.0f);
	
	// Draw acceleration range
	if (bAccelerateWhenClose)
	{
		DrawDebugSphere(GetWorld(), GetActorLocation(), AccelerationRange, 16, FColor::Yellow, false, -1.0f, 0, 1.0f);
	}
	
	// Draw movement direction - renamed to avoid conflict with base class member
	const FVector WallSpikeDirection = CurrentDirection * 200.0f;
	DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + WallSpikeDirection,
				 FColor::Blue, false, -1.0f, 0, 5.0f);
	
	// Draw primary direction
	const FVector PrimaryDir = GetPrimaryDirection() * 150.0f;
	DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + PrimaryDir,
				 FColor::Green, false, -1.0f, 0, 3.0f);
}
#endif
