// Fill out your copyright notice in the Description page of Project Settings.

#include "RunnerCharacter.h"
#include "GameFramework/Actor.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "WallSpike.h"
#include "Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Spikes.h"
#include "PlayerHealthComponent.h"
#include "SideRunnerGameInstance.h"
#include "GameFramework/PlayerStart.h"
#include "SideRunner.h" // Custom log categories

// CRITICAL FIX: Comprehensive validation macro for HealthComponent access
// Prevents access violations by validating component before use
// UE 5.5: Uses IsValid() global function for comprehensive validation
#define VALIDATE_HEALTH_COMPONENT_VOID() \
    if (!IsValid(HealthComponent)) \
    { \
        UE_LOG(LogSideRunner, Error, TEXT("%s: HealthComponent invalid! Address: %p"), \
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

    // CRITICAL FIX: Disable controller rotation for side-scroller (was incorrectly true)
    // Side-scrollers should NOT rotate with controller - character faces movement direction
    bUseControllerRotationPitch = false;
    bUseControllerRotationRoll = false;
    bUseControllerRotationYaw = false;

    // CRITICAL FIX: Spring arm configured for 2.5D side-scroller with LOCKED rotation
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 800.0f;  // Distance from character
    CameraBoom->bEnableCameraLag = true;   // Smooth camera movement
    CameraBoom->CameraLagSpeed = 8.0f;     // Responsive tracking for fast platformer
    CameraBoom->bDoCollisionTest = false;  // Side-scroller doesn't need collision
    CameraBoom->bUsePawnControlRotation = false;  // Fixed camera angle

    // CRITICAL FIX: Lock spring arm to absolute rotation (side-view)
    // This prevents camera from rotating with character movement
    CameraBoom->SetUsingAbsoluteRotation(true);  // KEY: Ignore parent rotation (UE 5.5 API)
    CameraBoom->bInheritPitch = false;     // Don't follow character pitch
    CameraBoom->bInheritYaw = false;       // Don't follow character yaw
    CameraBoom->bInheritRoll = false;      // Don't follow character roll
    CameraBoom->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));  // Side view angle (looking along the X-axis)

    // Create and configure camera
    SideViewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Side View Camera"));
    SideViewCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    SideViewCamera->bUsePawnControlRotation = false;
    SideViewCamera->FieldOfView = 95.0f;  // Optimal FOV for 2.5D side-scroller

    // PERFORMANCE: Optimize character movement setup for 2.5D side-scroller
    UCharacterMovementComponent* Movement = GetCharacterMovement();
    // Character rotation is now handled by flipping the sprite's scale in MoveRight().
    // All actor/controller rotation must be disabled.
    Movement->bOrientRotationToMovement = false;
    Movement->bUseControllerDesiredRotation = false;
    Movement->RotationRate = FRotator(0.0f, 0.0f, 0.0f);

    // --- 2.5D PLANAR MOVEMENT CONSTRAINT ---
    // Constrain all movement and collision resolution to the Y-Z plane.
    Movement->bConstrainToPlane = true;
    Movement->SetPlaneConstraintNormal(FVector(1.0f, 0.0f, 0.0f));
    // ------------------------------------

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

    // PROFESSIONAL 2D SPRITE FACING SYSTEM
    // Character starts facing right by default
    bIsFacingRight = true;

    // 2.5D PLANAR MOVEMENT CONSTRAINT
    // Will be set in BeginPlay to actual spawn location
    InitialXPosition = 0.0f;
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

    // CRITICAL FIX: Enforce absolute rotation in BeginPlay (belt-and-suspenders approach)
    // Ensures camera stays locked to side view even if something tries to change it
    if (CameraBoom)
    {
        CameraBoom->SetRelativeRotation(FRotator(0.0f, 0.0f, -90.0f));

        // COMPREHENSIVE DEBUG: Diagnose why camera shows front view despite correct rotation
        UE_LOG(LogSideRunner, Verbose, TEXT("=========================================="));
        UE_LOG(LogSideRunner, Verbose, TEXT("=== CAMERA DIAGNOSTIC INFO ==="));
        UE_LOG(LogSideRunner, Verbose, TEXT("=========================================="));

        // Camera Boom Info
        UE_LOG(LogSideRunner, Verbose, TEXT("CameraBoom World Rotation: %s"), *CameraBoom->GetComponentRotation().ToString());
        UE_LOG(LogSideRunner, Verbose, TEXT("CameraBoom Relative Rotation: %s"), *CameraBoom->GetRelativeRotation().ToString());
        UE_LOG(LogSideRunner, Verbose, TEXT("CameraBoom World Location: %s"), *CameraBoom->GetComponentLocation().ToString());
        UE_LOG(LogSideRunner, Verbose, TEXT("CameraBoom Forward Vector: %s"), *CameraBoom->GetForwardVector().ToString());

        // Camera Info
        UE_LOG(LogSideRunner, Verbose, TEXT("SideViewCamera World Rotation: %s"), *SideViewCamera->GetComponentRotation().ToString());
        UE_LOG(LogSideRunner, Verbose, TEXT("SideViewCamera World Location: %s"), *SideViewCamera->GetComponentLocation().ToString());
        UE_LOG(LogSideRunner, Verbose, TEXT("SideViewCamera Forward Vector: %s"), *SideViewCamera->GetForwardVector().ToString());

        // Character Info
        UE_LOG(LogSideRunner, Verbose, TEXT("Character World Rotation: %s"), *GetActorRotation().ToString());
        UE_LOG(LogSideRunner, Verbose, TEXT("Character World Location: %s"), *GetActorLocation().ToString());
        UE_LOG(LogSideRunner, Verbose, TEXT("Character Forward Vector: %s"), *GetActorForwardVector().ToString());

        // Sprite Component Info (if exists)
        if (CharacterVisual)
        {
            UE_LOG(LogSideRunner, Verbose, TEXT("CharacterVisual Component Name: %s"), *CharacterVisual->GetName());
            UE_LOG(LogSideRunner, Verbose, TEXT("CharacterVisual World Rotation: %s"), *CharacterVisual->GetComponentRotation().ToString());
            UE_LOG(LogSideRunner, Verbose, TEXT("CharacterVisual Relative Rotation: %s"), *CharacterVisual->GetRelativeRotation().ToString());
        }
        else
        {
            UE_LOG(LogSideRunner, Error, TEXT("CharacterVisual is NULL - sprite component not found!"));
        }

        // Active Camera Info
        APlayerController* PC = Cast<APlayerController>(GetController());
        if (PC)
        {
            AActor* ViewTarget = PC->GetViewTarget();
            UE_LOG(LogSideRunner, Verbose, TEXT("Active View Target: %s"), ViewTarget ? *ViewTarget->GetName() : TEXT("NULL"));

            FVector CameraLoc;
            FRotator CameraRot;
            PC->GetPlayerViewPoint(CameraLoc, CameraRot);
            UE_LOG(LogSideRunner, Verbose, TEXT("Active Camera Location: %s"), *CameraLoc.ToString());
            UE_LOG(LogSideRunner, Verbose, TEXT("Active Camera Rotation: %s"), *CameraRot.ToString());
        }

        UE_LOG(LogSideRunner, Verbose, TEXT("=========================================="));
    }

    // CRITICAL FIX: Lock character rotation for 2.5D side-scroller
    // Character should face forward (sprite orientation) and never rotate
    SetActorRotation(FRotator(0.0f, 0.0f, 0.0f));

    // CRITICAL FIX: Defer delegate binding until next frame to ensure HealthComponent is fully initialized
    // This prevents accessing an uninitialized component during BeginPlay ordering issues
    if (IsValid(HealthComponent))
    {
        // Use a short timer to ensure HealthComponent BeginPlay has completed
        FTimerHandle BindDelegatesTimer;
        GetWorldTimerManager().SetTimer(BindDelegatesTimer, [this]()
        {
            if (IsValid(this) && IsValid(HealthComponent) && HealthComponent->IsFullyInitialized())
            {
                HealthComponent->OnHealthChanged.AddDynamic(this, &ARunnerCharacter::OnHealthChanged);
                HealthComponent->OnTakeDamage.AddDynamic(this, &ARunnerCharacter::OnTakeDamage);
                HealthComponent->OnPlayerDeath.AddDynamic(this, &ARunnerCharacter::HandlePlayerDeath);
#if UE_BUILD_DEVELOPMENT
                UE_LOG(LogSideRunner, Log, TEXT("Health component delegates bound successfully"));
#endif
            }
            else
            {
                UE_LOG(LogSideRunner, Error, TEXT("Failed to bind health delegates - component not initialized"));
            }
        }, 0.1f, false);  // Small delay to ensure component initialization
    }

    // PERFORMANCE: Cache GameInstance to avoid 60 casts/second in Tick()
    // CRITICAL FIX: Use UGameplayStatics for more reliable GameInstance retrieval
    CachedGameInstance = Cast<USideRunnerGameInstance>(
        UGameplayStatics::GetGameInstance(this)
    );

    // Store initial spawn location as respawn point AND initialize score tracking
    if (CachedGameInstance)
    {
        const FVector SpawnLocation = GetActorLocation();
        CachedGameInstance->SetRespawnLocation(SpawnLocation);
        CachedGameInstance->InitializeDistanceTracking(SpawnLocation.X); // CRITICAL FIX: Start score from spawn X
        UE_LOG(LogSideRunner, Log, TEXT("Initial spawn location stored: %s"), *SpawnLocation.ToString());
    }
    else
    {
        // Diagnostic logging to identify why GameInstance is unavailable
        UWorld* World = GetWorld();
        UE_LOG(LogSideRunner, Warning, TEXT("BeginPlay: Failed to get GameInstance!"));
        UE_LOG(LogSideRunner, Warning, TEXT("  World pointer: %p"), World);
        if (World)
        {
            UGameInstance* RawGI = World->GetGameInstance();
            UE_LOG(LogSideRunner, Warning, TEXT("  Raw GameInstance: %p"), RawGI);
            if (RawGI)
            {
                UE_LOG(LogSideRunner, Warning, TEXT("  GameInstance class: %s"), *RawGI->GetClass()->GetName());
            }
        }
    }

    // CRITICAL FIX: Store initial X position for 2.5D constraint enforcement
    // This ensures character remains locked to the 2.5D plane even if physics tries to push them off
    InitialXPosition = GetActorLocation().X;
    UE_LOG(LogSideRunner, Log, TEXT("2.5D Constraint: Initial X-position locked at %.2f"), InitialXPosition);
}

// Called every frame
void ARunnerCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // DIAGNOSTIC: Force camera rotation every frame to override Blueprint settings
    // TODO: Remove this once camera is working correctly in Blueprint
    if (CameraBoom)
    {
        const FRotator CurrentRelativeRot = CameraBoom->GetRelativeRotation();
        const FRotator DesiredRotation = FRotator(0.0f, 0.0f, -90.0f);

        // Only reset if it's wrong (avoid spamming logs)
        if (!CurrentRelativeRot.Equals(DesiredRotation, 0.1f))
        {
            CameraBoom->SetRelativeRotation(DesiredRotation);
            UE_LOG(LogSideRunner, Verbose, TEXT("TICK: Camera rotation was wrong (%s), forcing to side view (%s)"),
                   *CurrentRelativeRot.ToString(), *DesiredRotation.ToString());
        }
    }

    // PERFORMANCE: Spring arm handles camera positioning automatically - no manual update needed

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

    // BELT-AND-SUSPENDERS: Enforce X-axis constraint in case physics pushes character off-plane
    // This is a safety net - the movement component constraint should prevent this, but we enforce it here too
    FVector CurrentLocation = GetActorLocation();
    if (!FMath::IsNearlyEqual(CurrentLocation.X, InitialXPosition, 1.0f))
    {
        CurrentLocation.X = InitialXPosition;
        SetActorLocation(CurrentLocation, false, nullptr, ETeleportType::TeleportPhysics);

#if UE_BUILD_DEVELOPMENT
        UE_LOG(LogSideRunner, Verbose, TEXT("2.5D Constraint: X-axis position corrected (%.2f -> %.2f) - character was pushed off plane"),
               GetActorLocation().X, InitialXPosition);
#endif
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

// DEPRECATED: Camera positioning now handled by USpringArmComponent
// Left here for reference - can be removed in future cleanup
/*
void ARunnerCharacter::UpdateCameraPosition()
{
    TempPos = GetActorLocation();
    TempPos.X += RunnerCharacterConstants::CAMERA_X_OFFSET;
    TempPos.Z = zPosition;
    SideViewCamera->SetWorldLocation(TempPos);
}
*/

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
    UE_LOG(LogSideRunner, Verbose, TEXT("Character state changed from %s to %s"),
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
    // Early exit for invalid states
    if (IsDead() || !CanMove)
        return;

    // Add movement input
    static const FVector MovementDirection = FVector(0.0f, 1.0f, 0.0f);
    AddMovementInput(MovementDirection, Value);

    // --- PROFESSIONAL 2D SPRITE FACING SYSTEM ---
    if (!FMath::IsNearlyZero(Value))
    {
        // Determine desired facing direction from input
        const bool bShouldFaceRight = (Value > 0.0f);

        // Only update sprite if direction has changed
        if (bShouldFaceRight != bIsFacingRight)
        {
            bIsFacingRight = bShouldFaceRight;

            // Flip sprite by inverting X-scale
            if (CharacterVisual)
            {
                FVector CurrentScale = CharacterVisual->GetRelativeScale3D();
                CurrentScale.X = bIsFacingRight ? 1.0f : -1.0f;
                CharacterVisual->SetRelativeScale3D(CurrentScale);
            }
        }
    }
}

void ARunnerCharacter::RestartLevel()
{
    UE_LOG(LogSideRunner, Log, TEXT("RestartLevel called"));

    // CRITICAL FIX: Clean up before level transition
    CleanupBeforeDestroy();

    // CRITICAL FIX: Validate world
    const UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogSideRunner, Error, TEXT("RestartLevel: World is null!"));
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
    UE_LOG(LogSideRunnerCombat, Verbose, TEXT("Player overlapped with WallSpike - applying instant death damage"));
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
    UE_LOG(LogSideRunnerCombat, VeryVerbose, TEXT("Player overlapped with regular Spikes - applying damage"));
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
    // CRITICAL FIX: Return FALSE (alive) when HealthComponent is invalid
    // Returning TRUE during initialization blocks ALL animation updates and breaks idle/running states

    // Check 1: Use IsValid() first. This is the standard check.
    if (!IsValid(HealthComponent))
    {
        UE_LOG(LogSideRunner, VeryVerbose, TEXT("IsDead: HealthComponent not valid - treating as ALIVE (not initialized yet)"));
        // During initialization, component may not be ready - treat as ALIVE to allow animations
        return false;  // ✓ FIXED: Return false (alive) instead of true (dead)
    }

    // Check 2: Defensive check for fully initialized state
    if (!HealthComponent->IsFullyInitialized())
    {
        UE_LOG(LogSideRunner, VeryVerbose, TEXT("IsDead: HealthComponent not fully initialized - treating as ALIVE"));
        // Component exists but BeginPlay hasn't completed - treat as ALIVE
        return false;  // ✓ FIXED: Return false (alive) during initialization
    }

    // Check 3: Verify component outer matches this character (memory corruption check)
    if (HealthComponent->GetOuter() != this)
    {
        UE_LOG(LogSideRunner, Error, TEXT("IsDead CHECK FAILED: Character '%s' has a CORRUPTED HealthComponent pointer!"), *GetName());
        // Memory corruption detected - fail-safe to ALIVE to prevent crash
        return false;  // ✓ FIXED: Return false (alive) to prevent animation lock
    }

    // All checks passed - safely get health status
    return (HealthComponent->GetCurrentHealth() <= 0);
}

bool ARunnerCharacter::IsGameOverSafe() const
{
	// Validate this actor
	if (!IsValid(this) || IsPendingKillPending())
	{
		UE_LOG(LogSideRunner, Error, TEXT("IsGameOverSafe: Character is invalid or pending kill"));
		return false;
	}

	// Validate world
	UWorld* World = GetWorld();
	if (!World || World->bIsTearingDown)
	{
		UE_LOG(LogSideRunner, Error, TEXT("IsGameOverSafe: World is invalid or tearing down"));
		return false;
	}

	// Validate player controller
	APlayerController* PC = World->GetFirstPlayerController();
	if (!IsValid(PC))
	{
		UE_LOG(LogSideRunner, Error, TEXT("IsGameOverSafe: PlayerController is invalid"));
		return false;
	}

	// Validate game instance
	USideRunnerGameInstance* GI = Cast<USideRunnerGameInstance>(
		UGameplayStatics::GetGameInstance(this)
	);
	if (!IsValid(GI))
	{
		UE_LOG(LogSideRunner, Error, TEXT("IsGameOverSafe: GameInstance is invalid"));
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

    // NOTE: Death logging handled authoritatively by PlayerHealthComponent

    // CRITICAL FIX: Use UGameplayStatics for reliable GameInstance retrieval
    // Enhanced diagnostic logging to identify exact failure point
    UWorld* World = GetWorld();
    UE_LOG(LogSideRunner, Log, TEXT("HandlePlayerDeath Diagnostics:"));
    UE_LOG(LogSideRunner, Log, TEXT("  World pointer: %p"), World);

    if (World)
    {
        UGameInstance* RawGI = World->GetGameInstance();
        UE_LOG(LogSideRunner, Log, TEXT("  Raw GameInstance: %p"), RawGI);
        UE_LOG(LogSideRunner, Log, TEXT("  World is tearing down: %s"),
               World->bIsTearingDown ? TEXT("YES") : TEXT("NO"));

        if (RawGI)
        {
            UE_LOG(LogSideRunner, Log, TEXT("  GameInstance class: %s"),
                   *RawGI->GetClass()->GetName());
        }
    }

    // Use UGameplayStatics instead of direct AActor::GetGameInstance
    CachedGameInstance = Cast<USideRunnerGameInstance>(
        UGameplayStatics::GetGameInstance(this)
    );

    if (IsValid(CachedGameInstance))
    {
        const bool bHasLivesRemaining = CachedGameInstance->DecrementLives();

        if (bHasLivesRemaining)
        {
            // Player has lives remaining - respawn after brief pause
            UE_LOG(LogSideRunnerScoring, Log, TEXT("Player has lives remaining - respawning"));

            // CRITICAL FIX: Use member variable RespawnTimerHandle (NOT local variable)
            // Brief delay to show death state, then respawn
            GetWorldTimerManager().SetTimer(RespawnTimerHandle, this, &ARunnerCharacter::RespawnPlayer,
                                           0.2f, false);  // POLISH FIX: Reduced from 0.5s to 0.2s for faster feedback
        }
        else
        {
            // No lives remaining - game over (immediate, no delay)
            UE_LOG(LogSideRunnerScoring, Warning, TEXT("No lives remaining - triggering game over"));
            // Game over already triggered by DecrementLives()

            // CRITICAL: Add diagnostic logging before calling Blueprint event
            UE_LOG(LogSideRunnerScoring, Verbose, TEXT("=== HandlePlayerDeath: About to trigger game over ==="));
            UE_LOG(LogSideRunnerScoring, Verbose, TEXT("  this=%p valid=%d"), this, IsValid(this) ? 1 : 0);
            UE_LOG(LogSideRunnerScoring, Verbose, TEXT("  GameInstance=%p valid=%d"), CachedGameInstance, IsValid(CachedGameInstance) ? 1 : 0);
            UE_LOG(LogSideRunnerScoring, Verbose, TEXT("  World=%p"), GetWorld());
            UE_LOG(LogSideRunnerScoring, Verbose, TEXT("  PlayerController=%p"), GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr);

            // CRITICAL FIX: Wrap blueprint event call with validation
            if (IsGameOverSafe())
            {
                UE_LOG(LogSideRunnerScoring, Verbose, TEXT("Calling DeathOfPlayer Blueprint event - all systems valid"));
                DeathOfPlayer();
            }
            else
            {
                UE_LOG(LogSideRunnerScoring, Error, TEXT("Cannot call DeathOfPlayer - system validation failed!"));
                // Fallback: Reload level directly to prevent soft-lock
                if (World)
                {
                    UGameplayStatics::OpenLevel(this, FName(*UGameplayStatics::GetCurrentLevelName(this)));
                }
            }
        }
    }
    else
    {
        UE_LOG(LogSideRunner, Error, TEXT("HandlePlayerDeath: GameInstance is invalid!"));
        
        // FALLBACK: Trigger immediate game over/restart to prevent soft-lock
        if (IsGameOverSafe())
        {
            UE_LOG(LogSideRunner, Warning, TEXT("Fallback: Calling DeathOfPlayer despite invalid GameInstance"));
            DeathOfPlayer();
        }
        else if (World)
        {
            // Last resort: reload level
            UE_LOG(LogSideRunner, Warning, TEXT("Last resort: Reloading current level"));
            UGameplayStatics::OpenLevel(this, FName(*UGameplayStatics::GetCurrentLevelName(this)));
        }
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
    UE_LOG(LogSideRunner, Log, TEXT("RespawnPlayer called"));

    // CRITICAL FIX: Validate 'this' pointer itself (timer may fire on destroyed object)
    // UE 5.5: Use IsValid() for 'this' pointer validation
    if (!IsValid(this))
    {
        UE_LOG(LogSideRunner, Error, TEXT("RespawnPlayer: 'this' pointer is invalid!"));
        return;
    }

    // CRITICAL FIX: Validate world before proceeding
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogSideRunner, Error, TEXT("RespawnPlayer: World is null!"));
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
                UE_LOG(LogSideRunner, Log, TEXT("Using PlayerStart location: %s"), *RespawnLocation.ToString());
            }
        }
        else
        {
            // Fallback to origin if no PlayerStart found
            RespawnLocation = RunnerCharacterConstants::FALLBACK_RESPAWN_LOCATION;
            UE_LOG(LogSideRunner, Warning, TEXT("No PlayerStart found - using fallback location"));
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

    UE_LOG(LogSideRunner, Log, TEXT("Player respawned at: %s"), *RespawnLocation.ToString());
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

    UE_LOG(LogSideRunner, Verbose, TEXT("RunnerCharacter cleanup completed"));
}

void ARunnerCharacter::BeginDestroy()
{
    CleanupBeforeDestroy();
    Super::BeginDestroy();
}

// Note: Debug console commands (TeleportToDistance, KillPlayer) have been moved to
// ASideRunnerPlayerController for proper Exec function support in UE5.5
