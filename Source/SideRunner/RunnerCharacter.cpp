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

// You need Paper2D plugin for this. Add it to your project if needed.
// If you're not using Paper2D, you can remove this include and work with USkeletalMeshComponent instead
// #include "PaperFlipbookComponent.h"

const float FALL_THRESHOLD = -1000.0f;

// Sets default values
ARunnerCharacter::ARunnerCharacter()
{
    // Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    GetCapsuleComponent()->InitCapsuleSize(42.f, 96.f);
    GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Overlap);

    // Stop the user from rotating the Character
    bUseControllerRotationPitch = true;
    bUseControllerRotationRoll = true;
    bUseControllerRotationYaw = true;

    // Create the Camera
    SideViewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Side View Camera"));
    // Stop the controller from rotating the Camera
    SideViewCamera->bUsePawnControlRotation = false;

    // Rotate the character towards the direction we are going
    GetCharacterMovement()->bOrientRotationToMovement = true;

    // Rotation Rate
    GetCharacterMovement()->RotationRate = FRotator(0.0f, RotationRate, 0.0f);

    // Set defaults for controlling the Character
    GetCharacterMovement()->GravityScale = 2.5f;
    GetCharacterMovement()->AirControl = 0.5f;
    GetCharacterMovement()->JumpZVelocity = 1000.0f;
    GetCharacterMovement()->GroundFriction = 3.0f;
    GetCharacterMovement()->MaxWalkSpeed = 600.0f;
    GetCharacterMovement()->MaxFlySpeed = 600.0f;

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

    TempPos = GetActorLocation();
    TempPos.X -= 800;
    TempPos.Z = zPosition;
    SideViewCamera->SetWorldLocation(TempPos);

    // Check for falling beyond threshold
    if (GetActorLocation().Z < FALL_THRESHOLD)
    {
        // Environmental hazard damage - falling off the map
        if (HealthComponent)
        {
            HealthComponent->TakeDamage(HealthComponent->GetCurrentHealth(), EDamageType::EnvironmentalHazard);
        }
        RestartLevel();
    }

    // Update the animation state based on movement
    UpdateAnimationState();

    // Increment state timer
    StateTimer += DeltaTime;
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
    // Check if running
    else if (FMath::Abs(GetVelocity().Y) > 10.0f)
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

    // Log state change
    UE_LOG(LogTemp, Verbose, TEXT("Character state changed from %s to %s"),
        *UEnum::GetValueAsString(OldState),
        *UEnum::GetValueAsString(CurrentState));

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
        CanDoubleJump = true;

        // Set jumping state
        SetCharacterState(ECharacterState::Jumping);
    }
    else if (CanDoubleJump)
    {
        // Double jump
        LaunchCharacter(FVector(0, 0, DoubleJumpZVelocity), false, true);
        CanDoubleJump = false;

        // Set double jumping state
        SetCharacterState(ECharacterState::DoubleJumping);
    }
}

void ARunnerCharacter::MoveRight(float Value)
{
    // Don't allow movement if dead
    if (IsDead())
        return;

    if (CanMove)
    {
        FVector Direction = FVector(0.0f, 1.0f, 0.0f); // Keep movement only along the Y-axis
        AddMovementInput(Direction, Value);

        // Update animation state based on movement
        if (FMath::Abs(Value) > 0.1f && !GetCharacterMovement()->IsFalling())
        {
            if (CurrentState != ECharacterState::Running)
            {
                SetCharacterState(ECharacterState::Running);
            }
        }
        else if (FMath::Abs(Value) < 0.1f && !GetCharacterMovement()->IsFalling())
        {
            if (CurrentState != ECharacterState::Idle)
            {
                SetCharacterState(ECharacterState::Idle);
            }
        }
    }
}

void ARunnerCharacter::RestartLevel()
{
    // Call restart level
    UGameplayStatics::OpenLevel(this, FName(*GetWorld()->GetName()));
}

void ARunnerCharacter::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
    const FHitResult& SweepResult)
{
    if (OtherActor != nullptr && !IsDead())
    {
        AWallSpike* WallSpike = Cast<AWallSpike>(OtherActor);
        ASpikes* Spike = Cast<ASpikes>(OtherActor);

        // Handle WallSpike (instant death) vs regular Spikes (incremental damage) differently
        if (WallSpike)
        {
            // WallSpike handles its own instant death logic
            UE_LOG(LogTemp, Warning, TEXT("Player touched WallSpike - should trigger instant death!"));
            // WallSpike will handle the death in its own Tick/overlap logic
        }
        else if (Spike)
        {
            // Regular spikes handle their own incremental damage through overlap events
            UE_LOG(LogTemp, Log, TEXT("Player touched regular Spikes - incremental damage will be applied"));
            // Spikes will handle the incremental damage in their own overlap logic
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
        CanDoubleJump = false;
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
