// Fill out your copyright notice in the Description page of Project Settings.

#include "RunnerCharacter.h"
#include "GameFramework/Actor.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "WallSpike.h"
#include "Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Spikes.h"
#include "PlayerHealthComponent.h"
#include "SideRunnerGameInstance.h"
#include "GameFramework/PlayerStart.h"

// CRITICAL FIX: Comprehensive validation macro for HealthComponent access
// Prevents access violations by validating component before use
// UE 5.5: Uses IsValid() global function for comprehensive validation
#define VALIDATE_HEALTH_COMPONENT_VOID() \
    if (!IsValid(HealthComponent)) \
    { \
        UE_LOG(LogTemp, Error, TEXT("%s: HealthComponent invalid! Address: %p"), \
               *FString(__FUNCTION__), HealthComponent); \
        return; \
    }

// PERFORMANCE: Constants for better maintainability and performance
namespace RunnerCharacterConstants
{
    constexpr float FALL_THRESHOLD = -1000.0f;
    constexpr float CAMERA_HEIGHT_OFFSET = 150.0f;
    constexpr float CAMERA_X_OFFSET = -800.0f;
    constexpr float MOVEMENT_THRESHOLD = 10.0f;
    constexpr float STATE_TIMER_RESET = 0.0f;
    constexpr float RESTART_LEVEL_DELAY = 2.0f;

    // Fallback spawn location when no PlayerStart found
    const FVector FALLBACK_RESPAWN_LOCATION = FVector(0.0f, 0.0f, 200.0f);
}

// You need Paper2D plugin for this. Add it to your project if needed.
// If you're not using Paper2D, you can remove this include and work with USkeletalMeshComponent instead
// #include "PaperFlipbookComponent.h"

ARunnerCharacter::ARunnerCharacter()
{
    // Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    // PERFORMANCE: Optimized capsule component setup
    GetCapsuleComponent()->InitCapsuleSize(42.f, 96.f);
    GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Overlap);

    // Stop the user from rotating the Character
    bUseControllerRotationPitch = true;
    bUseControllerRotationRoll = true;
    bUseControllerRotationYaw = true;

    // Create and configure camera
    SideViewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Side View Camera"));
    SideViewCamera->bUsePawnControlRotation = false;

    // PERFORMANCE: Optimize character movement setup
    UCharacterMovementComponent* Movement = GetCharacterMovement();
    Movement->bOrientRotationToMovement = true;
    Movement->RotationRate = FRotator(0.0f, RotationRate, 0.0f);
    Movement->GravityScale = 2.5f;
    Movement->AirControl = 0.5f;
    Movement->JumpZVelocity = 1000.0f;
    Movement->GroundFriction = 3.0f;
    Movement->MaxWalkSpeed = 600.0f;
    Movement->MaxFlySpeed = 600.0f;

    // Initialize camera positioning
    TempPos = GetActorLocation();
    zPosition = TempPos.Z + RunnerCharacterConstants::CAMERA_HEIGHT_OFFSET;

    // Initialize animation state
    CurrentState = ECharacterState::Idle;
    PreviousState = ECharacterState::Idle;
    StateTimer = RunnerCharacterConstants::STATE_TIMER_RESET;

    // Set jump properties
    DoubleJumpZVelocity = 800.0f;

    // PERFORMANCE: Create health component with validation
    HealthComponent = CreateDefaultSubobject<UPlayerHealthComponent>(TEXT("HealthComponent"));
    ensure(HealthComponent); // Debug validation

    // Initialize death processing flag
    bIsProcessingDeath = false;
}

// Called when the game starts or when spawned
void ARunnerCharacter::BeginPlay()
{
    Super::BeginPlay();

    // PERFORMANCE: Bind overlap events efficiently
    if (UCapsuleComponent* Capsule = GetCapsuleComponent())
    {
        Capsule->OnComponentBeginOverlap.AddDynamic(this, &ARunnerCharacter::OnOverlapBegin);
    }

    // Initialize movement and jump state
    CanMove = true;
    CanJump = true;
    bCanDoubleJump = true;

    // Set initial character state
    SetCharacterState(ECharacterState::Idle);

    // PERFORMANCE: Bind health events if component exists
    if (IsValid(HealthComponent))
    {
        HealthComponent->OnHealthChanged.AddDynamic(this, &ARunnerCharacter::OnHealthChanged);
        HealthComponent->OnTakeDamage.AddDynamic(this, &ARunnerCharacter::OnTakeDamage);
        HealthComponent->OnPlayerDeath.AddDynamic(this, &ARunnerCharacter::HandlePlayerDeath);
    }

    // PERFORMANCE: Cache GameInstance to avoid 60 casts/second in Tick()
    CachedGameInstance = Cast<USideRunnerGameInstance>(GetGameInstance());

    // Store initial spawn location as respawn point AND initialize score tracking
    if (CachedGameInstance)
    {
        const FVector SpawnLocation = GetActorLocation();
        CachedGameInstance->SetRespawnLocation(SpawnLocation);
        CachedGameInstance->InitializeDistanceTracking(SpawnLocation.X); // CRITICAL FIX: Start score from spawn X
        UE_LOG(LogTemp, Log, TEXT("Initial spawn location stored: %s"), *SpawnLocation.ToString());
    }
}

// Called every frame
void ARunnerCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // PERFORMANCE: Optimized camera positioning
    UpdateCameraPosition();

    // PERFORMANCE: Early exit if character is dead
    if (CurrentState == ECharacterState::Dead)
    {
        return;
    }

    // Update distance score in game instance
    if (!IsDead() && CachedGameInstance)
    {
        CachedGameInstance->UpdateDistanceScore(GetActorLocation().X);
    }

    // Check for fall threshold
    if (GetActorLocation().Z < RunnerCharacterConstants::FALL_THRESHOLD)
    {
        HandleEnvironmentalDeath();
        return;
    }

    // Update animation state and timer
    UpdateAnimationState();
    StateTimer += DeltaTime;
}

void ARunnerCharacter::UpdateCameraPosition()
{
    TempPos = GetActorLocation();
    TempPos.X += RunnerCharacterConstants::CAMERA_X_OFFSET;
    TempPos.Z = zPosition;
    SideViewCamera->SetWorldLocation(TempPos);
}

void ARunnerCharacter::HandleEnvironmentalDeath()
{
    // CRITICAL FIX: Use same death system as obstacles - respects lives system
    // TakeDamage triggers OnPlayerDeath → HandlePlayerDeath → lives system
    VALIDATE_HEALTH_COMPONENT_VOID();

    if (!IsDead())
    {
        HealthComponent->TakeDamage(HealthComponent->GetCurrentHealth(), EDamageType::EnvironmentalHazard);
    }
}

void ARunnerCharacter::UpdateAnimationState()
{
    // PERFORMANCE: Early exit for dead state
    if (CurrentState == ECharacterState::Dead)
        return;

    // PERFORMANCE: Cache movement component
    const UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
    if (!MovementComponent)
        return;

    ECharacterState NewState = CurrentState;

    // PERFORMANCE: Efficient state determination
    if (MovementComponent->IsFalling())
    {
        NewState = DetermineAirborneState();
    }
    else if (IsMovingHorizontally())
    {
        NewState = ECharacterState::Running;
    }
    else
    {
        NewState = ECharacterState::Idle;
    }

    // Update state if changed
    if (NewState != CurrentState)
    {
        // CRITICAL FIX: Reset double-jump when landing on ground
        if ((NewState == ECharacterState::Running || NewState == ECharacterState::Idle) &&
            (CurrentState == ECharacterState::Falling ||
             CurrentState == ECharacterState::Jumping ||
             CurrentState == ECharacterState::DoubleJumping))
        {
            bCanDoubleJump = true;  // Reset for next jump sequence
        }

        SetCharacterState(NewState);
    }
}

ECharacterState ARunnerCharacter::DetermineAirborneState() const
{
    const float VerticalVelocity = GetVelocity().Z;
    
    if (VerticalVelocity > 0)
    {
        return (CurrentState == ECharacterState::DoubleJumping) ? ECharacterState::DoubleJumping : ECharacterState::Jumping;
    }
    
    return ECharacterState::Falling;
}

bool ARunnerCharacter::IsMovingHorizontally() const
{
    return FMath::Abs(GetVelocity().Y) > RunnerCharacterConstants::MOVEMENT_THRESHOLD;
}

void ARunnerCharacter::SetCharacterState(ECharacterState NewState)
{
    // PERFORMANCE: Early exit if state hasn't changed
    if (NewState == CurrentState)
        return;

    const ECharacterState OldState = CurrentState;
    PreviousState = OldState;
    CurrentState = NewState;
    StateTimer = RunnerCharacterConstants::STATE_TIMER_RESET;

#if UE_BUILD_DEVELOPMENT
    UE_LOG(LogTemp, Verbose, TEXT("Character state changed from %s to %s"),
        *UEnum::GetValueAsString(OldState),
        *UEnum::GetValueAsString(CurrentState));
#endif

    // Call blueprint events
    OnCharacterStateChanged(NewState, OldState);
    UpdateCharacterSprite();
}

// Called to bind functionality to input
void ARunnerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // PERFORMANCE: Efficient input binding
    if (PlayerInputComponent)
    {
        PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ARunnerCharacter::Jump);
        PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
        PlayerInputComponent->BindAxis("MoveRight", this, &ARunnerCharacter::MoveRight);
    }
}

void ARunnerCharacter::Jump()
{
    // PERFORMANCE: Early exit for dead state
    if (IsDead())
        return;

    const UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
    if (!MovementComponent)
        return;

    if (CanJump && !MovementComponent->IsFalling())
    {
        // First jump
        ACharacter::Jump();
        bCanDoubleJump = true;
        SetCharacterState(ECharacterState::Jumping);
    }
    else if (bCanDoubleJump && MovementComponent->IsFalling())
    {
        // CRITICAL FIX: Use bCanDoubleJump (public UPROPERTY) and ensure we're airborne
        // Double jump
        LaunchCharacter(FVector(0, 0, DoubleJumpZVelocity), false, true);
        bCanDoubleJump = false;
        SetCharacterState(ECharacterState::DoubleJumping);
    }
}

void ARunnerCharacter::MoveRight(float Value)
{
    // PERFORMANCE: Early exit for invalid states
    if (IsDead() || !CanMove || FMath::IsNearlyZero(Value))
        return;

    // PERFORMANCE: Use constant direction vector
    static const FVector MovementDirection = FVector(0.0f, 1.0f, 0.0f);
    AddMovementInput(MovementDirection, Value);

    // PERFORMANCE: Only update animation state if significant movement
    if (FMath::Abs(Value) > 0.1f)
    {
        const UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
        if (MovementComponent && !MovementComponent->IsFalling())
        {
            if (CurrentState != ECharacterState::Running)
            {
                SetCharacterState(ECharacterState::Running);
            }
        }
    }
    else if (CurrentState == ECharacterState::Running)
    {
        const UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
        if (MovementComponent && !MovementComponent->IsFalling())
        {
            SetCharacterState(ECharacterState::Idle);
        }
    }
}

void ARunnerCharacter::RestartLevel()
{
    UE_LOG(LogTemp, Log, TEXT("RestartLevel called"));

    // CRITICAL FIX: Clean up before level transition
    CleanupBeforeDestroy();

    // CRITICAL FIX: Validate world
    const UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("RestartLevel: World is null!"));
        return;
    }

    // Reset game instance
    if (IsValid(CachedGameInstance))
    {
        CachedGameInstance->ResetGameSession();
    }

    // Reload level
    const FString CurrentLevelName = World->GetName();
    UGameplayStatics::OpenLevel(this, FName(*CurrentLevelName));
}

void ARunnerCharacter::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
    const FHitResult& SweepResult)
{
    // PERFORMANCE: Early validation and exit
    if (!OtherActor || IsDead())
        return;

    // PERFORMANCE: Handle different spike types efficiently
    if (AWallSpike* WallSpike = Cast<AWallSpike>(OtherActor))
    {
        HandleWallSpikeOverlap(WallSpike);
    }
    else if (ASpikes* RegularSpike = Cast<ASpikes>(OtherActor))
    {
        HandleRegularSpikeOverlap(RegularSpike);
    }
}

void ARunnerCharacter::HandleWallSpikeOverlap(AWallSpike* WallSpike)
{
    // CRITICAL FIX: Centralized damage handling to prevent double damage
    if (!WallSpike || !IsValid(HealthComponent))
        return;

#if UE_BUILD_DEVELOPMENT
    UE_LOG(LogTemp, Warning, TEXT("Player overlapped with WallSpike - applying instant death damage"));
#endif

    // Apply instant death damage through health component
    const int32 InstantDeathDamage = HealthComponent->GetMaxHealth() * 10;
    HealthComponent->TakeDamage(InstantDeathDamage, EDamageType::Spikes);

    // Check for death and handle accordingly
    if (IsDead())
    {
        HandlePlayerDeath(HealthComponent->GetTotalHitsTaken());
    }
}

void ARunnerCharacter::HandleRegularSpikeOverlap(ASpikes* RegularSpike)
{
    // CRITICAL FIX: Centralized damage handling to prevent double damage
    if (!RegularSpike || !IsValid(HealthComponent) || HealthComponent->IsInvulnerable())
        return;

#if UE_BUILD_DEVELOPMENT
    UE_LOG(LogTemp, VeryVerbose, TEXT("Player overlapped with regular Spikes - applying damage"));
#endif

    // Apply regular spike damage through health component
    const int32 SpikeDamage = static_cast<int32>(RegularSpike->DamageAmount);
    HealthComponent->TakeDamage(SpikeDamage, EDamageType::Spikes);

    // Check for death and handle accordingly
    if (IsDead())
    {
        HandlePlayerDeath(HealthComponent->GetTotalHitsTaken());
    }
}

void ARunnerCharacter::ProcessDamage(float DamageAmount, AActor* DamageCauser)
{
    // PERFORMANCE: Validate inputs
    if (!IsValid(HealthComponent) || HealthComponent->IsInvulnerable() || DamageAmount <= 0.0f)
        return;

    // Determine damage type based on causer
    EDamageType DamageType = EDamageType::EnvironmentalHazard;
    if (Cast<ASpikes>(DamageCauser))
    {
        DamageType = EDamageType::Spikes;
    }

    // Apply damage
    HealthComponent->TakeDamage(static_cast<int32>(DamageAmount), DamageType);

    // Check for death
    if (IsDead())
    {
        HandlePlayerDeath(HealthComponent->GetTotalHitsTaken());
    }
}

bool ARunnerCharacter::IsDead() const
{
    return IsHealthComponentValid() && (HealthComponent->GetCurrentHealth() <= 0);
}

bool ARunnerCharacter::IsGameOverSafe() const
{
	// Validate this actor
	if (!IsValid(this) || IsPendingKillPending())
	{
		UE_LOG(LogTemp, Error, TEXT("IsGameOverSafe: Character is invalid or pending kill"));
		return false;
	}

	// Validate world
	UWorld* World = GetWorld();
	if (!World || World->bIsTearingDown)
	{
		UE_LOG(LogTemp, Error, TEXT("IsGameOverSafe: World is invalid or tearing down"));
		return false;
	}

	// Validate player controller
	APlayerController* PC = World->GetFirstPlayerController();
	if (!IsValid(PC))
	{
		UE_LOG(LogTemp, Error, TEXT("IsGameOverSafe: PlayerController is invalid"));
		return false;
	}

	// Validate game instance
	USideRunnerGameInstance* GI = Cast<USideRunnerGameInstance>(GetGameInstance());
	if (!IsValid(GI))
	{
		UE_LOG(LogTemp, Error, TEXT("IsGameOverSafe: GameInstance is invalid"));
		return false;
	}

	// All systems valid
	return true;
}

void ARunnerCharacter::HandlePlayerDeath(int32 TotalHitsTaken)
{
    // CRITICAL FIX: Prevent duplicate death processing
    if (bIsProcessingDeath || CurrentState == ECharacterState::Dead)
    {
        return;
    }

    bIsProcessingDeath = true;
    SetCharacterState(ECharacterState::Dead);

    // Disable mesh and movement
    if (USkeletalMeshComponent* SkeletalMesh = GetMesh())
    {
        SkeletalMesh->Deactivate();
        SkeletalMesh->SetVisibility(false);
    }

    // Disable input
    CanMove = false;
    CanJump = false;
    bCanDoubleJump = false;

    // Reset jump velocity through movement component
    if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
    {
        MovementComponent->JumpZVelocity = 0.0f;
    }

#if UE_BUILD_DEVELOPMENT
    UE_LOG(LogTemp, Log, TEXT("Player died after taking %d hits"), TotalHitsTaken);
#endif

    // CRITICAL FIX: Re-validate game instance before use
    CachedGameInstance = Cast<USideRunnerGameInstance>(GetGameInstance());

    if (IsValid(CachedGameInstance))
    {
        const bool bHasLivesRemaining = CachedGameInstance->DecrementLives();

        if (bHasLivesRemaining)
        {
            // Player has lives remaining - respawn after brief pause
            UE_LOG(LogTemp, Log, TEXT("Player has lives remaining - respawning"));

            // CRITICAL FIX: Use member variable RespawnTimerHandle (NOT local variable)
            // Brief delay to show death state, then respawn
            GetWorldTimerManager().SetTimer(RespawnTimerHandle, this, &ARunnerCharacter::RespawnPlayer,
                                           0.2f, false);  // POLISH FIX: Reduced from 0.5s to 0.2s for faster feedback
        }
        else
        {
            // No lives remaining - game over (immediate, no delay)
            UE_LOG(LogTemp, Warning, TEXT("No lives remaining - triggering game over"));
            // Game over already triggered by DecrementLives()

            // CRITICAL: Add diagnostic logging before calling Blueprint event
            UE_LOG(LogTemp, Warning, TEXT("=== HandlePlayerDeath: About to trigger game over ==="));
            UE_LOG(LogTemp, Warning, TEXT("  this=%p valid=%d"), this, IsValid(this) ? 1 : 0);
            UE_LOG(LogTemp, Warning, TEXT("  GameInstance=%p valid=%d"), CachedGameInstance, IsValid(CachedGameInstance) ? 1 : 0);
            UE_LOG(LogTemp, Warning, TEXT("  World=%p"), GetWorld());
            UE_LOG(LogTemp, Warning, TEXT("  PlayerController=%p"), GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr);

            // CRITICAL FIX: Wrap blueprint event call with validation
            if (IsGameOverSafe())
            {
                UE_LOG(LogTemp, Warning, TEXT("Calling DeathOfPlayer Blueprint event - all systems valid"));
                DeathOfPlayer();
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("Cannot call DeathOfPlayer - system validation failed!"));
                // Fallback: Reload level directly to prevent soft-lock
                UWorld* World = GetWorld();
                if (World)
                {
                    UGameplayStatics::OpenLevel(this, FName(*UGameplayStatics::GetCurrentLevelName(this)));
                }
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("HandlePlayerDeath: GameInstance is invalid!"));
    }
}

float ARunnerCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, 
                                  AController* EventInstigator, AActor* DamageCauser)
{
    const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    
    // PERFORMANCE: Only process if damage was actually applied
    if (ActualDamage > 0.0f)
    {
        ProcessDamage(ActualDamage, DamageCauser);
    }

    return ActualDamage;
}

void ARunnerCharacter::RespawnPlayer()
{
    UE_LOG(LogTemp, Log, TEXT("RespawnPlayer called"));

    // CRITICAL FIX: Validate 'this' pointer itself (timer may fire on destroyed object)
    // UE 5.5: Use IsValid() for 'this' pointer validation
    if (!IsValid(this))
    {
        UE_LOG(LogTemp, Error, TEXT("RespawnPlayer: 'this' pointer is invalid!"));
        return;
    }

    // CRITICAL FIX: Validate world before proceeding
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("RespawnPlayer: World is null!"));
        return;
    }

    // Clear death processing flag
    bIsProcessingDeath = false;

    // CRITICAL FIX: Use validation macro for consistency
    VALIDATE_HEALTH_COMPONENT_VOID();
    HealthComponent->ResetHealth();

    // Grant 2 seconds of invulnerability on respawn to prevent instant death
    HealthComponent->SetInvulnerabilityTime(2.0f);

    // Re-enable mesh and movement
    if (USkeletalMeshComponent* SkeletalMesh = GetMesh())
    {
        SkeletalMesh->Activate();
        SkeletalMesh->SetVisibility(true);
    }

    // Re-enable input
    CanMove = true;
    CanJump = true;
    bCanDoubleJump = true;

    // Restore jump velocity
    if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
    {
        MovementComponent->JumpZVelocity = 1000.0f;
    }

    // Reset animation state
    SetCharacterState(ECharacterState::Idle);

    // CRITICAL FIX: Get respawn location from game instance (or use player start)
    FVector RespawnLocation = FVector::ZeroVector;
    FRotator RespawnRotation = FRotator::ZeroRotator;

    // Try to get saved respawn location
    CachedGameInstance = Cast<USideRunnerGameInstance>(GetGameInstance());
    if (IsValid(CachedGameInstance))
    {
        RespawnLocation = CachedGameInstance->GetRespawnLocation();
    }

    // If no saved location, find PlayerStart
    if (RespawnLocation.IsZero())
    {
        TArray<AActor*> PlayerStarts;
        UGameplayStatics::GetAllActorsOfClass(World, APlayerStart::StaticClass(), PlayerStarts);

        if (PlayerStarts.Num() > 0)
        {
            APlayerStart* PlayerStart = Cast<APlayerStart>(PlayerStarts[0]);
            if (PlayerStart)
            {
                RespawnLocation = PlayerStart->GetActorLocation();
                RespawnRotation = PlayerStart->GetActorRotation();
                UE_LOG(LogTemp, Log, TEXT("Using PlayerStart location: %s"), *RespawnLocation.ToString());
            }
        }
        else
        {
            // Fallback to origin if no PlayerStart found
            RespawnLocation = RunnerCharacterConstants::FALLBACK_RESPAWN_LOCATION;
            UE_LOG(LogTemp, Warning, TEXT("No PlayerStart found - using fallback location"));
        }
    }

    // Teleport player to respawn location
    SetActorLocation(RespawnLocation, false, nullptr, ETeleportType::ResetPhysics);
    SetActorRotation(RespawnRotation);

    // Reset velocity
    if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
    {
        MovementComponent->Velocity = FVector::ZeroVector;
    }

    UE_LOG(LogTemp, Log, TEXT("Player respawned at: %s"), *RespawnLocation.ToString());
}

void ARunnerCharacter::CleanupBeforeDestroy()
{
    // CRITICAL FIX: Clear all active timers to prevent callbacks on destroyed object
    if (UWorld* World = GetWorld())
    {
        if (FTimerManager* TimerManager = &World->GetTimerManager())
        {
            TimerManager->ClearTimer(RespawnTimerHandle);  // CRITICAL FIX: Clear respawn timer
            TimerManager->ClearAllTimersForObject(this);
        }
    }

    // Unbind health component delegates - use validation helper
    if (IsHealthComponentValid())
    {
        HealthComponent->OnHealthChanged.RemoveAll(this);
        HealthComponent->OnTakeDamage.RemoveAll(this);
        HealthComponent->OnPlayerDeath.RemoveAll(this);
    }

    UE_LOG(LogTemp, Verbose, TEXT("RunnerCharacter cleanup completed"));
}

void ARunnerCharacter::BeginDestroy()
{
    CleanupBeforeDestroy();
    Super::BeginDestroy();
}

#if !UE_BUILD_SHIPPING
// ======================================================================
// Debug Console Commands (Development/Editor builds only)
// ======================================================================

void ARunnerCharacter::TeleportToDistance(float DistanceMeters)
{
    // Convert meters to Unreal units (1 meter = 100 units)
    const float TargetX = DistanceMeters * 100.0f;

    // Get current location and update X position only
    FVector NewLocation = GetActorLocation();
    NewLocation.X = TargetX;

    // Teleport player
    SetActorLocation(NewLocation, false, nullptr, ETeleportType::TeleportPhysics);

    // Update game instance distance tracking
    if (IsValid(CachedGameInstance))
    {
        CachedGameInstance->UpdateDistanceScore(TargetX);
    }

    UE_LOG(LogTemp, Warning, TEXT("DEBUG: Teleported to %.1f meters (X=%.1f units)"), DistanceMeters, TargetX);
}

void ARunnerCharacter::KillPlayer()
{
    // Instantly kill player by dealing massive damage
    if (IsHealthComponentValid())
    {
        const float MaxHealth = HealthComponent->GetMaxHealth();
        HealthComponent->TakeDamage(MaxHealth * 10.0f, EDamageType::EnvironmentalHazard);

        UE_LOG(LogTemp, Warning, TEXT("DEBUG: Player killed via console command"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("DEBUG: Cannot kill player - HealthComponent is invalid!"));
    }
}
#else
// ======================================================================
// Shipping Build Stubs (No-op implementations)
// ======================================================================

void ARunnerCharacter::TeleportToDistance(float DistanceMeters)
{
    // No-op in shipping builds - debug commands are disabled
}

void ARunnerCharacter::KillPlayer()
{
    // No-op in shipping builds - debug commands are disabled
}
#endif
