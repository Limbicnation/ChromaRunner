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
	DirectionalBias = 0.4f; // Renamed from RightwardBias
	ChaseRange = 1500.0f;
	DirectionChangeRate = 2.0f;
	bAccelerateWhenClose = true;
	AccelerationRange = 500.0f;
	MaxSpeedMultiplier = 2.0f;
	
	// Initialize directional properties - DEFAULT TO FORWARD (+Y)
	bUsePresetDirections = true;
	PresetDirectionIndex = 0; // 0 = Forward (+Y)
	PrimaryDirection = FVector(0.0f, 1.0f, 0.0f); // Forward by default
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
	MovementType = EMovementType::Static; // This stops base class movement
	bIsMoving = false; // Disable base movement
	Speed = 0.0f; // Zero out base speed
	
	// Initialize optimized private members
	TargetPlayer = nullptr;
	CurrentDirection = FVector(0.0f, 1.0f, 0.0f); // Start moving forward by default
	PlayerSearchTimer = 0.0f;
	PlayerSearchInterval = 0.5f; // Reduced frequency for better performance
	bHasTarget = false;
	bHasKilledPlayer = false;
	PlayerDeathTimer = 0.0f; // Instance variable instead of static
	bTrackingPlayerDeath = false;
	TimeBehindPlayer = 0.0f; // Initialize behind timer
	ChaseAudioComponent = nullptr;
}

void AWallSpike::BeginPlay()
{
	Super::BeginPlay();
	
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
	CurrentDirection = PrimaryDirection; // Initialize current direction
	
#if UE_BUILD_DEBUG
	// CRITICAL DEBUG LOGGING - only in debug builds
	UE_LOG(LogTemp, Error, TEXT("=== WALLSPIKE DEBUG START ==="));
	UE_LOG(LogTemp, Error, TEXT("WallSpike BeginPlay at location: %s"), *GetActorLocation().ToString());
	UE_LOG(LogTemp, Error, TEXT("Primary Direction: %s"), *PrimaryDirection.ToString());
	UE_LOG(LogTemp, Error, TEXT("Preset Direction: %s"), bUsePresetDirections ? TEXT("Enabled") : TEXT("Disabled"));
	UE_LOG(LogTemp, Error, TEXT("Directional Bias: %.2f"), DirectionalBias);
	UE_LOG(LogTemp, Error, TEXT("Actor Mobility: %d"), GetRootComponent() ? (int32)GetRootComponent()->Mobility : -1);
	UE_LOG(LogTemp, Error, TEXT("CollisionBox valid: %s"), CollisionBox ? TEXT("YES") : TEXT("NO"));
	UE_LOG(LogTemp, Error, TEXT("SpikeMesh valid: %s"), SpikeMesh ? TEXT("YES") : TEXT("NO"));
#endif
	
	// Ensure collision is properly set up for wall spikes
	if (CollisionBox)
	{
		// FORCE mobility settings
		CollisionBox->SetMobility(EComponentMobility::Movable);
		
		// IMPROVED COLLISION SETUP for better player detection
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		CollisionBox->SetCollisionProfileName(TEXT("BlockAll"));
		CollisionBox->SetNotifyRigidBodyCollision(true);
		CollisionBox->SetGenerateOverlapEvents(true);
		
		// Set specific collision responses for better detection
		CollisionBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
		CollisionBox->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
		
		// CRITICAL: Add overlap event binding for additional collision detection
		CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AWallSpike::OnOverlapBegin);
		CollisionBox->OnComponentHit.AddDynamic(this, &AWallSpike::OnHit);
		
#if UE_BUILD_DEBUG
		UE_LOG(LogTemp, Warning, TEXT("WallSpike collision configured with overlap and hit events"));
#endif
	}
#if UE_BUILD_DEBUG
	else
	{
		UE_LOG(LogTemp, Error, TEXT("WallSpike CollisionBox is null! Check Blueprint setup."));
	}
#endif
	
	// Ensure mesh is movable too
	if (SpikeMesh)
	{
		SpikeMesh->SetMobility(EComponentMobility::Movable);
#if UE_BUILD_DEBUG
		UE_LOG(LogTemp, Warning, TEXT("SpikeMesh mobility set to Movable"));
#endif
	}
	
	// Initialize player search immediately
	UpdateTargetPlayer();
	
	// Reset all flags and timers
	bHasKilledPlayer = false;
	PlayerDeathTimer = 0.0f;
	bTrackingPlayerDeath = false;
	
#if UE_BUILD_DEBUG
	UE_LOG(LogTemp, Error, TEXT("=== WALLSPIKE DEBUG END ==="));
#endif
}

void AWallSpike::Tick(float DeltaTime)
{
	// IMPORTANT: Call AActor::Tick directly to bypass base Spikes movement
	AActor::Tick(DeltaTime);
	
	// PERFORMANCE: Reduce debug output frequency and only in debug builds
#if UE_BUILD_DEBUG
	static float DebugTimer = 0.0f;
	DebugTimer += DeltaTime;
	if (DebugTimer > 5.0f) // Reduced frequency for performance
	{
		if (TargetPlayer)
		{
			UE_LOG(LogTemp, Warning, TEXT("WallSpike Tick - Position: %s | Target: %s | Direction: %s"), 
				   *GetActorLocation().ToString(),
				   *TargetPlayer->GetName(),
				   *CurrentDirection.ToString());
		}
		DebugTimer = 0.0f;
	}
#endif
	
	// PERFORMANCE: Optimized on-screen debug messages - only in development
#if UE_BUILD_DEVELOPMENT
	if (GEngine && TargetPlayer)
	{
		const FString DebugMsg = FString::Printf(TEXT("WallSpike: %.1f units from player | Dir: %s | Speed: %.1f"), 
			FVector::Dist(GetActorLocation(), TargetPlayer->GetActorLocation()),
			*CurrentDirection.ToString(), 
			ChaseSpeed);
		
		// Use unique ID for this specific instance - fix ambiguous call by being explicit
		int32 UniqueKey = GetUniqueID();
		GEngine->AddOnScreenDebugMessage(UniqueKey, 0.0f, FColor::Yellow, DebugMsg);
	}
#endif
	
	// Handle player search timing - optimized
	PlayerSearchTimer -= DeltaTime;
	if (PlayerSearchTimer <= 0.0f)
	{
		UpdateTargetPlayer();
		PlayerSearchTimer = PlayerSearchInterval;
	}
	
	// Update chasing movement - THIS IS THE KEY FUNCTION
	UpdateChaseMovement(DeltaTime);
	
	// Check if we should be destroyed
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
#if UE_BUILD_DEBUG
			UE_LOG(LogTemp, Log, TEXT("WallSpike: Player respawned or found new target, resetting death timer"));
#endif
		}
		
#if UE_BUILD_DEBUG
		const float Distance = FMath::Sqrt(DistanceSquared);
		UE_LOG(LogTemp, Verbose, TEXT("WallSpike locked onto target: %s (Distance: %.1f)"), 
			   *CachedPlayer->GetName(), Distance);
#endif
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
		// Start tracking death timer for this specific spike
		bTrackingPlayerDeath = true;
		PlayerDeathTimer = 0.0f;
		StopChaseAudio();
#if UE_BUILD_DEBUG
		UE_LOG(LogTemp, Log, TEXT("WallSpike: Player died, starting %f second cleanup timer"), DeathCleanupDelay);
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
		const float Distance = FVector::Dist(GetActorLocation(), TargetPlayer->GetActorLocation());
		UE_LOG(LogTemp, Verbose, TEXT("WallSpike lost target - player too far (Distance: %.1f)"), Distance);
#endif
		StopChaseAudio();
	}
	TargetPlayer = nullptr;
	bHasTarget = false;
}

void AWallSpike::HandleChaseAudioStart(bool bWasHasTarget)
{
	// ENHANCED SOUND: Play chase start sound when we first lock onto a target
	if (!bWasHasTarget && ChaseStartSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ChaseStartSound, GetActorLocation(), 
											 ChaseVolumeMultiplier, ChasePitchMultiplier);
#if UE_BUILD_DEBUG
		UE_LOG(LogTemp, Log, TEXT("WallSpike playing chase start sound"));
#endif
	}
	
	// ENHANCED SOUND: Start looping chase sound if not already playing
	if (!bWasHasTarget && ChaseLoopSound && !ChaseAudioComponent)
	{
		ChaseAudioComponent = UGameplayStatics::SpawnSoundAttached(
			ChaseLoopSound, GetRootComponent(), NAME_None, FVector::ZeroVector,
			EAttachLocation::KeepRelativeOffset, false, 
			ChaseVolumeMultiplier, ChasePitchMultiplier, 0.0f, nullptr, nullptr, true);
		
		if (ChaseAudioComponent && !ChaseAudioComponent->IsPlaying())
		{
			ChaseAudioComponent->Play();
#if UE_BUILD_DEBUG
			UE_LOG(LogTemp, Log, TEXT("WallSpike started chase loop sound"));
#endif
		}
	}
}

void AWallSpike::StopChaseAudio()
{
	if (ChaseAudioComponent && ChaseAudioComponent->IsPlaying())
	{
		ChaseAudioComponent->Stop();
#if UE_BUILD_DEBUG
		UE_LOG(LogTemp, Log, TEXT("WallSpike stopped chase loop sound"));
#endif
	}
}

FVector AWallSpike::CalculateChaseDirection() const
{
	FVector DesiredDirection = GetPrimaryDirection(); // Use primary direction instead of hardcoded right
	
	if (bHasTarget && TargetPlayer && !TargetPlayer->IsDead())
	{
		// Calculate direction to player
		const FVector ToPlayer = (TargetPlayer->GetActorLocation() - GetActorLocation()).GetSafeNormal();
		
		// Blend between chasing player and moving in primary direction based on bias
		// DirectionalBias of 0.0 = pure chase, 1.0 = pure directional movement
		const FVector PrimaryDir = GetPrimaryDirection();
		DesiredDirection = FMath::Lerp(ToPlayer, PrimaryDir, DirectionalBias).GetSafeNormal();
		
#if UE_BUILD_DEBUG && 0 // Very verbose logging - disabled by default
		UE_LOG(LogTemp, VeryVerbose, TEXT("WallSpike chase direction: Player(%.2f,%.2f,%.2f) + Primary(%.2f,%.2f,%.2f) bias %.2f = (%.2f,%.2f,%.2f)"),
			   ToPlayer.X, ToPlayer.Y, ToPlayer.Z, 
			   PrimaryDir.X, PrimaryDir.Y, PrimaryDir.Z, 
			   DirectionalBias,
			   DesiredDirection.X, DesiredDirection.Y, DesiredDirection.Z);
#endif
	}
	
	return DesiredDirection;
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
			
#if UE_BUILD_DEBUG && 0 // Very verbose logging - disabled by default
			UE_LOG(LogTemp, VeryVerbose, TEXT("WallSpike accelerating: Distance %.1f, Factor %.2f, Speed %.1f"), 
				   Distance, AccelerationFactor, CurrentSpeed);
#endif
		}
	}
	
	// Calculate movement delta
	const FVector MovementDelta = CurrentDirection * CurrentSpeed * DeltaTime;
	const FVector OldLocation = GetActorLocation();
	const FVector NewLocation = OldLocation + MovementDelta;
	
	// IMPROVED: Use sweep to detect collisions with detailed logging
	FHitResult HitResult;
	const bool bHitSomething = SetActorLocation(NewLocation, true, &HitResult);
	
	// ADDITIONAL: Check for player proximity and force collision detection
	if (TargetPlayer && !bHasKilledPlayer)
	{
		const float DistanceToPlayer = FVector::Dist(GetActorLocation(), TargetPlayer->GetActorLocation());
		
		// If we're very close to the player, ensure we detect the collision
		if (DistanceToPlayer < 200.0f) // Within 200 units
		{
#if UE_BUILD_DEBUG
			UE_LOG(LogTemp, Warning, TEXT("WallSpike very close to player (Distance: %.1f) - checking collision"), DistanceToPlayer);
#endif
			
			// Force a sphere trace to check for player collision
			const FVector SphereCenter = GetActorLocation();
			constexpr float SphereRadius = 150.0f; // Collision detection radius
			
			TArray<FHitResult> OutHits;
			FCollisionQueryParams QueryParams;
			QueryParams.AddIgnoredActor(this);
			
			const bool bFoundPlayer = GetWorld()->SweepMultiByChannel(
				OutHits,
				SphereCenter,
				SphereCenter + FVector(1.0f, 0.0f, 0.0f), // Small sweep
				FQuat::Identity,
				ECC_Pawn,
				FCollisionShape::MakeSphere(SphereRadius),
				QueryParams
			);
			
			if (bFoundPlayer)
			{
				for (const FHitResult& ProximityHit : OutHits)
				{
					ARunnerCharacter* ProximityPlayer = Cast<ARunnerCharacter>(ProximityHit.GetActor());
					if (ProximityPlayer && ProximityPlayer == TargetPlayer)
					{
#if UE_BUILD_DEBUG
						UE_LOG(LogTemp, Error, TEXT("WallSpike PROXIMITY DETECTION! Player found within collision sphere!"));
#endif
						ApplyInstantDeathToPlayer(ProximityPlayer, ProximityHit.Location);
						return;
					}
				}
			}
		}
	}
	
#if UE_BUILD_DEBUG && 0 // Very verbose logging - disabled by default
	// Log movement for debugging
	UE_LOG(LogTemp, VeryVerbose, TEXT("WallSpike Movement: Old=%s, New=%s, Delta=%s, Hit=%s"), 
		   *OldLocation.ToString(), *GetActorLocation().ToString(), *MovementDelta.ToString(), bHitSomething ? TEXT("YES") : TEXT("NO"));
#endif
	
	// Check if we hit the player during movement
	if (bHitSomething && HitResult.GetActor())
	{
#if UE_BUILD_DEBUG
		UE_LOG(LogTemp, Warning, TEXT("WallSpike SetActorLocation detected collision with: %s"), *HitResult.GetActor()->GetName());
#endif
		
		ARunnerCharacter* HitPlayer = Cast<ARunnerCharacter>(HitResult.GetActor());
		if (HitPlayer && !bHasKilledPlayer)
		{
#if UE_BUILD_DEBUG
			UE_LOG(LogTemp, Error, TEXT("WallSpike hit player during chase movement! Applying instant death!"));
#endif
			ApplyInstantDeathToPlayer(HitPlayer, HitResult.Location);
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
	constexpr float ProximityThresholdSquared = 150.0f * 150.0f; // 150 units squared
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
	// Handle player death cleanup with instance timer
	if (bTrackingPlayerDeath)
	{
		PlayerDeathTimer += GetWorld()->GetDeltaSeconds();
		
		if (PlayerDeathTimer >= DeathCleanupDelay)
		{
#if UE_BUILD_DEBUG
			UE_LOG(LogTemp, Log, TEXT("WallSpike destroying self - player has been dead for %.1fs"), PlayerDeathTimer);
#endif
			Destroy();
			return;
		}
		
		// Continue moving in primary direction while waiting for cleanup
		return;
	}
	
	if (!TargetPlayer)
		return;
	
	// Check if we're too far behind the player (updated for any direction)
	const FVector PlayerLocation = TargetPlayer->GetActorLocation();
	const FVector SpikeLocation = GetActorLocation();
	const FVector PrimaryDir = GetPrimaryDirection();
	
	// Calculate how far we are in the opposite direction of our primary movement
	const FVector ToSpike = SpikeLocation - PlayerLocation;
	const float DistanceInOppositeDirection = FVector::DotProduct(ToSpike, -PrimaryDir);
	
	if (DistanceInOppositeDirection > MaxDistanceBehindPlayer) // Spike is too far behind player
	{
		// Check if we're moving further away
		const float MovingTowardsPlayer = FVector::DotProduct(CurrentDirection, PrimaryDir);
		if (MovingTowardsPlayer <= 0.0f) // Moving away from primary direction
		{
#if UE_BUILD_DEBUG
			UE_LOG(LogTemp, Log, TEXT("WallSpike destroying self - too far behind player (Distance: %.1f)"), DistanceInOppositeDirection);
#endif
			Destroy();
			return;
		}
		
		// Track how long we've been behind using instance variable
		TimeBehindPlayer += GetWorld()->GetDeltaSeconds();
		
		if (TimeBehindPlayer >= MaxTimeBehindPlayer)
		{
#if UE_BUILD_DEBUG
			UE_LOG(LogTemp, Log, TEXT("WallSpike destroying self - been too far behind for %.1fs"), TimeBehindPlayer);
#endif
			Destroy();
			return;
		}
	}
	else
	{
		// Reset behind timer if we're not too far behind
		TimeBehindPlayer = 0.0f;
	}
}

void AWallSpike::ApplyInstantDeathToPlayer(ARunnerCharacter* Player, FVector HitLocation)
{
	if (!Player || Player->IsDead() || bHasKilledPlayer)
		return;
	
	// Set flag to prevent multiple kills
	bHasKilledPlayer = true;
	
#if UE_BUILD_DEBUG
	UE_LOG(LogTemp, Error, TEXT("WallSpike CAUGHT PLAYER! Applying instant death!"));
#endif
	
	// ENHANCED DEATH SOUND: Stop chase sound and play dramatic death sound
	StopChaseAudio();
	
	// Play collision sound with enhanced parameters for dramatic effect
	if (CollisionSound)
	{
		// Play death sound with higher volume and pitch for impact
		UGameplayStatics::PlaySoundAtLocation(this, CollisionSound, HitLocation, 
											 ChaseVolumeMultiplier * 1.5f, // 50% louder
											 ChasePitchMultiplier * 0.8f);  // Lower pitch for dramatic effect
#if UE_BUILD_DEBUG
		UE_LOG(LogTemp, Error, TEXT("WallSpike playing DEATH sound at location: %s"), *HitLocation.ToString());
#endif
	}
#if UE_BUILD_DEBUG
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("WallSpike: No CollisionSound set! Please assign a death sound in the Blueprint."));
	}
#endif
	
	// Show particle effect at hit location
	if (ImpactEffect)
	{
		ImpactEffect->SetWorldLocation(HitLocation);
		ImpactEffect->Activate(true);
	}
	
	// Apply instant death through health component
	if (UPlayerHealthComponent* HealthComp = Player->GetComponentByClass<UPlayerHealthComponent>())
	{
		// Apply massive damage to ensure instant death
		const int32 InstantDeathDamage = HealthComp->GetMaxHealth() * 10; // Massive overkill
		HealthComp->TakeDamage(InstantDeathDamage, EDamageType::Spikes);
	}
	else
	{
		// Fallback: Use Unreal's built-in damage system
		FDamageEvent DamageEvent;
		Player->TakeDamage(9999.0f, DamageEvent, nullptr, this);
	}
	
	// Stop chasing after killing the player
	bHasTarget = false;
	TargetPlayer = nullptr;
	
	// Start death tracking for cleanup
	bTrackingPlayerDeath = true;
	PlayerDeathTimer = 0.0f;
	
	// PERFORMANCE: Use lambda with timer for destruction
	FTimerHandle DestroyTimer;
	GetWorld()->GetTimerManager().SetTimer(DestroyTimer, [this]()
	{
		if (IsValid(this))
		{
#if UE_BUILD_DEBUG
			UE_LOG(LogTemp, Log, TEXT("WallSpike destroying self after successful kill"));
#endif
			Destroy();
		}
	}, 1.0f, false);
}

// Override NotifyHit as backup collision detection
void AWallSpike::NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, 
	bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);
	
	// Additional safety check - if sweep somehow missed, catch it here
	ARunnerCharacter* HitPlayer = Cast<ARunnerCharacter>(Other);
	if (HitPlayer && !bHasKilledPlayer)
	{
#if UE_BUILD_DEBUG
		UE_LOG(LogTemp, Warning, TEXT("WallSpike NotifyHit backup trigger - applying instant death"));
#endif
		ApplyInstantDeathToPlayer(HitPlayer, HitLocation);
	}
}

// Handle overlap events for collision detection
void AWallSpike::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
							   UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
							   bool bFromSweep, const FHitResult& SweepResult)
{
#if UE_BUILD_DEBUG
	UE_LOG(LogTemp, Warning, TEXT("WallSpike OnOverlapBegin triggered with: %s"), 
		   OtherActor ? *OtherActor->GetName() : TEXT("Unknown"));
#endif

	ARunnerCharacter* HitPlayer = Cast<ARunnerCharacter>(OtherActor);
	if (HitPlayer && !bHasKilledPlayer)
	{
#if UE_BUILD_DEBUG
		UE_LOG(LogTemp, Error, TEXT("WallSpike OVERLAP with player detected! Applying instant death!"));
#endif
		FVector HitLocation;
		if (SweepResult.IsValidBlockingHit())
		{
			HitLocation = SweepResult.Location;
		}
		else
		{
			HitLocation = GetActorLocation();
		}
		ApplyInstantDeathToPlayer(HitPlayer, HitLocation);
	}
}

// Handle hit events for collision detection
void AWallSpike::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, 
					   UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
#if UE_BUILD_DEBUG
	UE_LOG(LogTemp, Warning, TEXT("WallSpike OnHit triggered with: %s"), 
		   OtherActor ? *OtherActor->GetName() : TEXT("Unknown"));
#endif

	ARunnerCharacter* HitPlayer = Cast<ARunnerCharacter>(OtherActor);
	if (HitPlayer && !bHasKilledPlayer)
	{
#if UE_BUILD_DEBUG
		UE_LOG(LogTemp, Error, TEXT("WallSpike HIT with player detected! Applying instant death!"));
#endif
		ApplyInstantDeathToPlayer(HitPlayer, Hit.Location);
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
	
	// Draw primary direction in green
	const FVector PrimaryDir = GetPrimaryDirection() * 150.0f;
	DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + PrimaryDir,
				 FColor::Green, false, -1.0f, 0, 3.0f);
}
#endif