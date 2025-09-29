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
#include "TimerManager.h"

// You need Paper2D plugin for this. Add it to your project if needed.
// If you're not using Paper2D, you can remove this include and work with USkeletalMeshComponent instead
// #include "PaperFlipbookComponent.h"

const float FALL_THRESHOLD = -1000.0f;
const float MOVEMENT_DEADZONE = 0.1f;  // NEW: Prevent micro-movements
const float STUCK_VELOCITY_THRESHOLD = 10.0f;  // NEW: Detect when character is stuck

// Sets default values
ARunnerCharacter::ARunnerCharacter()
{
    // Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    // OPTIMIZATION: Improved capsule collision setup
    GetCapsuleComponent()->InitCapsuleSize(42.f, 96.f);
    GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Overlap);
    
    // NEW: Better collision handling for stuck prevention
    GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
    GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);

    // Stop the user from rotating the Character
    bUseControllerRotationPitch = false;  // FIXED: Should be false for side-scroller
    bUseControllerRotationRoll = false;   // FIXED: Should be false for side-scroller
    bUseControllerRotationYaw = false;    // FIXED: Should be false for side-scroller

    // Create the Camera
    SideViewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Side View Camera"));
    // Stop the controller from rotating the Camera
    SideViewCamera->bUsePawnControlRotation = false;

    // Rotate the character towards the direction we are going
    GetCharacterMovement()->bOrientRotationToMovement = true;

    // Rotation Rate
    GetCharacterMovement()->RotationRate = FRotator(0.0f, RotationRate, 0.0f);

    // OPTIMIZATION: Improved movement settings for side-scroller
    GetCharacterMovement()->GravityScale = 2.5f;
    GetCharacterMovement()->AirControl = 0.5f;
    GetCharacterMovement()->JumpZVelocity = 1000.0f;
    GetCharacterMovement()->GroundFriction = 3.0f;
    GetCharacterMovement()->MaxWalkSpeed = 600.0f;
    GetCharacterMovement()->MaxFlySpeed = 600.0f;
    
    // NEW: Anti-stuck settings
    GetCharacterMovement()->bCanWalkOffLedges = true;
    GetCharacterMovement()->bCanWalkOffLedgesWhenCrouching = true;
    GetCharacterMovement()->MaxStepHeight = 45.0f;
    GetCharacterMovement()->PerchRadiusThreshold = 0.0f;
    GetCharacterMovement()->PerchAdditionalHeight = 0.0f;

    // Find the Characters temporary position
    TempPos = GetActorLocation();
    // The Camera height position
    zPosition = TempPos.Z + 150.0f;

    // Initialize animation state variables
    CurrentState = ECharacterState::Idle;
    PreviousState = ECharacterState::Idle;
    StateTimer = 0.0f;

    // Set default jump properties
    DoubleJumpZVelocity = 800.0f;
    
    // NEW: Initialize stuck detection
    StuckTimer = 0.0f;
    PreviousLocation = FVector::ZeroVector;

    // Create and set up the health component
    HealthComponent = CreateDefaultSubobject<UPlayerHealthComponent>(TEXT("HealthComponent"));
}

// Called when the game starts or when spawned
void ARunnerCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Get the Capsule Component
    GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &ARunnerCharacter::OnOverlapBegin);

    CanMove = true;
    CanJump = true;
    bCanDoubleJump = true; // set CanDoubleJump to true by default

    // Initialize the character in idle state
    SetCharacterState(ECharacterState::Idle);
    
    // NEW: Initialize stuck detection
    PreviousLocation = GetActorLocation();
    StuckTimer = 0.0f;

    // Bind health events
    if (HealthComponent)
    {
        HealthComponent->OnHealthChanged.AddDynamic(this, &ARunnerCharacter::OnHealthChanged);
        HealthComponent->OnTakeDamage.AddDynamic(this, &ARunnerCharacter::OnTakeDamage);
        HealthComponent->OnPlayerDeath.AddDynamic(this, &ARunnerCharacter::HandlePlayerDeath);
    }
}

// Called every frame
void ARunnerCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // OPTIMIZATION: Camera following with smoothing
    UpdateCameraPosition(DeltaTime);

    // Check for falling beyond threshold
    if (GetActorLocation().Z < FALL_THRESHOLD)
    {
        // Environmental hazard damage - falling off the map
        if (HealthComponent)
        {
            HealthComponent->TakeDamage(HealthComponent->GetCurrentHealth(), EDamageType::EnvironmentalHazard);
        }
        RestartLevel();
        return;  // Exit early after death
    }
    
    // NEW: Stuck detection and resolution
    HandleStuckDetection(DeltaTime);

    // Update the animation state based on movement
    UpdateAnimationState();

    // Increment state timer
    StateTimer += DeltaTime;
}

void ARunnerCharacter::UpdateCameraPosition(float DeltaTime)
{
    // OPTIMIZATION: Smooth camera following
    FVector CurrentCameraPos = SideViewCamera->GetComponentLocation();
    FVector TargetPos = GetActorLocation();
    TargetPos.X -= 800;
    TargetPos.Z = zPosition;
    
    // Smooth interpolation for better feel
    float LerpSpeed = 5.0f;  // Adjust this for faster/slower camera follow
    FVector NewCameraPos = FMath::VInterpTo(CurrentCameraPos, TargetPos, DeltaTime, LerpSpeed);
    SideViewCamera->SetWorldLocation(NewCameraPos);
}

void ARunnerCharacter::HandleStuckDetection(float DeltaTime)
{
    FVector CurrentLocation = GetActorLocation();
    FVector CurrentVelocity = GetVelocity();
    
    // Check if character is trying to move but velocity is very low
    bool bTryingToMove = FMath::Abs(GetInputAxisValue("MoveRight")) > MOVEMENT_DEADZONE;
    bool bVelocityLow = CurrentVelocity.Size2D() < STUCK_VELOCITY_THRESHOLD;
    bool bLocationSimilar = FVector::Dist2D(CurrentLocation, PreviousLocation) < 5.0f;
    
    if (bTryingToMove && bVelocityLow && bLocationSimilar)
    {
        StuckTimer += DeltaTime;
        
        // If stuck for more than 0.5 seconds, try to resolve
        if (StuckTimer > 0.5f)
        {
            ResolveStuckCharacter();
            StuckTimer = 0.0f;
        }
    }
    else
    {
        StuckTimer = 0.0f;
    }
    
    // Update previous location
    PreviousLocation = CurrentLocation;
}

void ARunnerCharacter::ResolveStuckCharacter()
{
    UE_LOG(LogTemp, Warning, TEXT("Character stuck detected, attempting resolution"));
    
    // Try multiple resolution strategies
    FVector CurrentLocation = GetActorLocation();
    
    // Strategy 1: Small upward impulse
    LaunchCharacter(FVector(0, 0, 200), false, false);
    
    // Strategy 2: Slight horizontal push in movement direction
    float MoveInput = GetInputAxisValue("MoveRight");
    if (FMath::Abs(MoveInput) > MOVEMENT_DEADZONE)
    {
        FVector PushDirection = FVector(0, MoveInput * 100, 0);
        LaunchCharacter(PushDirection, false, false);
    }
    
    // Strategy 3: If still stuck after a frame, try teleport slightly up
    FTimerHandle TeleportTimer;
    GetWorldTimerManager().SetTimer(TeleportTimer, this, &ARunnerCharacter::TeleportStuckCharacter, 0.1f, false);
}

void ARunnerCharacter::TeleportStuckCharacter()
{
    FVector CurrentLocation = GetActorLocation();
    FVector NewLocation = CurrentLocation + FVector(0, 0, 50);  // Move up 50 units
    SetActorLocation(NewLocation, true);  // Use sweep to avoid teleporting into walls
    
    UE_LOG(LogTemp, Log, TEXT("Character teleported from stuck position"));
}

void ARunnerCharacter::UpdateAnimationState()
{
    // Skip if the character is dead
    if (CurrentState == ECharacterState::Dead)
        return;

    // Get the character movement component
    UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
    if (!MovementComponent)
        return;

    // Determine the character state based on movement
    ECharacterState NewState = CurrentState;

    // Check if falling
    if (MovementComponent->IsFalling())
    {
        // Check if rising or falling
        if (GetVelocity().Z > 0)
        {
            if (CurrentState == ECharacterState::DoubleJumping)
            {
                // Keep double jumping state
                NewState = ECharacterState::DoubleJumping;
            }
            else
            {
                // Regular jump
                NewState = ECharacterState::Jumping;
            }
        }
        else
        {
            NewState = ECharacterState::Falling;
        }
    }
    // Check if running (with deadzone to prevent micro-movement detection)
    else if (FMath::Abs(GetVelocity().Y) > MOVEMENT_DEADZONE * 100.0f)  // Scale deadzone for velocity
    {
        NewState = ECharacterState::Running;
    }
    // Otherwise idle
    else
    {
        NewState = ECharacterState::Idle;
    }

    // If state changed, update it
    if (NewState != CurrentState)
    {
        SetCharacterState(NewState);
    }
}

void ARunnerCharacter::SetCharacterState(ECharacterState NewState)
{
    // Store previous state before changing current
    ECharacterState OldState = CurrentState;

    // Set new state
    CurrentState = NewState;

    // Reset state timer
    StateTimer = 0.0f;

    // OPTIMIZATION: Reduce log spam - only log important state changes
    if (OldState != NewState)
    {
        UE_LOG(LogTemp, Verbose, TEXT("Character state changed from %s to %s"),
            *UEnum::GetValueAsString(OldState),
            *UEnum::GetValueAsString(CurrentState));
    }

    // The previous state is stored in the class member
    PreviousState = OldState;

    // Call the blueprint event to handle visual updates
    OnCharacterStateChanged(NewState, OldState);

    // Update the sprite animation
    UpdateCharacterSprite();
}

// Called to bind functionality to input
void ARunnerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // Bind Character Input functionality
    PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ARunnerCharacter::Jump);
    PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

    PlayerInputComponent->BindAxis("MoveRight", this, &ARunnerCharacter::MoveRight);
}

void ARunnerCharacter::Jump()
{
    // Don't allow jump if dead
    if (IsDead())
        return;

    if (CanJump && !GetCharacterMovement()->IsFalling())
    {
        // First jump
        ACharacter::Jump();
        bCanDoubleJump = true;

        // Set jumping state
        SetCharacterState(ECharacterState::Jumping);
    }
    else if (bCanDoubleJump)
    {
        // Double jump
        LaunchCharacter(FVector(0, 0, DoubleJumpZVelocity), false, true);
        bCanDoubleJump = false;

        // Set double jumping state
        SetCharacterState(ECharacterState::DoubleJumping);
    }
}

void ARunnerCharacter::MoveRight(float Value)
{
    // Don't allow movement if dead
    if (IsDead())
        return;

    // OPTIMIZATION: Apply deadzone to prevent micro-movements
    if (FMath::Abs(Value) < MOVEMENT_DEADZONE)
    {
        Value = 0.0f;
    }

    if (CanMove && FMath::Abs(Value) > 0.0f)
    {
        FVector Direction = FVector(0.0f, 1.0f, 0.0f); // Keep movement only along the Y-axis
        AddMovementInput(Direction, Value);

        // Update animation state based on movement
        if (!GetCharacterMovement()->IsFalling())
        {
            if (CurrentState != ECharacterState::Running)
            {
                SetCharacterState(ECharacterState::Running);
            }
        }
    }
    else if (FMath::Abs(Value) <= MOVEMENT_DEADZONE && !GetCharacterMovement()->IsFalling())
    {
        if (CurrentState != ECharacterState::Idle)
        {
            SetCharacterState(ECharacterState::Idle);
        }
    }
}

void ARunnerCharacter::RestartLevel()
{
    // OPTIMIZATION: Add safety check
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot restart level - World is null"));
        return;
    }

    // Call restart level
    UGameplayStatics::OpenLevel(this, FName(*GetWorld()->GetName()));
}

void ARunnerCharacter::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
    const FHitResult& SweepResult)
{
    if (!IsValid(OtherActor) || !HealthComponent)
    {
        return;
    }

    // OPTIMIZATION: Early exit if invulnerable
    if (HealthComponent->IsInvulnerable())
    {
        return;
    }

    AWallSpike* WallSpike = Cast<AWallSpike>(OtherActor);
    ASpikes* Spike = Cast<ASpikes>(OtherActor);

    // Check if we collide with either Wall or Spike Actor
    if (WallSpike || Spike)
    {
        // Determine damage amount and type
        float damageAmount = 0.0f;

        if (Spike)
        {
            // Use the damage amount specified by the spike
            damageAmount = Spike->DamageAmount;
        }
        else if (WallSpike)
        {
            // Use a default damage amount for wall spikes
            damageAmount = 10.0f;
        }

        // Apply damage
        HealthComponent->TakeDamage(damageAmount, EDamageType::Spikes);

        // Check if the player is dead after taking damage
        if (IsDead())
        {
            HandlePlayerDeath(HealthComponent->GetTotalHitsTaken());
        }
    }
}

// Process damage from a damage source
void ARunnerCharacter::ProcessDamage(float DamageAmount, AActor* DamageCauser)
{
    if (!HealthComponent || HealthComponent->IsInvulnerable())
        return;

    // Determine damage type based on the causer
    EDamageType damageType = EDamageType::EnvironmentalHazard;

    // Check what type of actor caused the damage
    if (Cast<ASpikes>(DamageCauser))
    {
        damageType = EDamageType::Spikes;
    }
    // Add more damage type checks here as you add more enemy types

    // Apply the damage
    HealthComponent->TakeDamage(DamageAmount, damageType);

    // Check if the player is dead after taking damage
    if (IsDead())
    {
        HandlePlayerDeath(HealthComponent->GetTotalHitsTaken());
    }
}

// Check if the player is dead
bool ARunnerCharacter::IsDead() const
{
    return HealthComponent ? (HealthComponent->GetCurrentHealth() <= 0) : false;
}

// Handle player death from health component
void ARunnerCharacter::HandlePlayerDeath(int32 TotalHitsTaken)
{
    // Set character to dead state if not already
    if (CurrentState != ECharacterState::Dead)
    {
        SetCharacterState(ECharacterState::Dead);

        // get the mesh and deactivate it
        GetMesh()->Deactivate();
        // Set the visibility of the mesh to false
        GetMesh()->SetVisibility(false);
        // disable movement input
        CanMove = false;
        // disable Jump input
        CanJump = false;
        bCanDoubleJump = false;
        JumpZVelocity = 0.0f;

        // Log death information
        UE_LOG(LogTemp, Log, TEXT("Player died after taking %d hits."), TotalHitsTaken);

        // Called in character blueprint
        DeathOfPlayer();

        // restart the Level after a delay
        FTimerHandle UnusedHandle;
        GetWorldTimerManager().SetTimer(UnusedHandle, this, &ARunnerCharacter::RestartLevel, 2.0f, false);
    }
}

// Override TakeDamage to use our health component
float ARunnerCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    // Process damage through our health component
    ProcessDamage(ActualDamage, DamageCauser);

    return ActualDamage;
}
