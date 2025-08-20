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

// PERFORMANCE: Constants for better maintainability and performance
namespace RunnerCharacterConstants
{
    constexpr float FALL_THRESHOLD = -1000.0f;
    constexpr float CAMERA_HEIGHT_OFFSET = 150.0f;
    constexpr float CAMERA_X_OFFSET = -800.0f;
    constexpr float MOVEMENT_THRESHOLD = 10.0f;
    constexpr float STATE_TIMER_RESET = 0.0f;
    constexpr float RESTART_LEVEL_DELAY = 2.0f;
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
    if (HealthComponent)
    {
        HealthComponent->OnHealthChanged.AddDynamic(this, &ARunnerCharacter::OnHealthChanged);
        HealthComponent->OnTakeDamage.AddDynamic(this, &ARunnerCharacter::OnTakeDamage);
        HealthComponent->OnPlayerDeath.AddDynamic(this, &ARunnerCharacter::HandlePlayerDeath);
    }
#if UE_BUILD_DEBUG
    else
    {
        UE_LOG(LogTemp, Error, TEXT("RunnerCharacter: HealthComponent is null!"));
    }
#endif
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
    if (HealthComponent)
    {
        HealthComponent->TakeDamage(HealthComponent->GetCurrentHealth(), EDamageType::EnvironmentalHazard);
    }
    RestartLevel();
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
    else if (CanDoubleJump)
    {
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
    // PERFORMANCE: Use more efficient level restart
    const UWorld* World = GetWorld();
    if (World)
    {
        const FString CurrentLevelName = World->GetName();
        UGameplayStatics::OpenLevel(this, FName(*CurrentLevelName));
    }
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
#if UE_BUILD_DEVELOPMENT
    UE_LOG(LogTemp, Warning, TEXT("Player overlapped with WallSpike - instant death triggered"));
#endif
    // WallSpike handles its own instant death logic
}

void ARunnerCharacter::HandleRegularSpikeOverlap(ASpikes* RegularSpike)
{
#if UE_BUILD_DEVELOPMENT
    UE_LOG(LogTemp, Log, TEXT("Player overlapped with regular Spikes - damage will be applied"));
#endif
    // Regular spikes handle their own damage logic
}

void ARunnerCharacter::ProcessDamage(float DamageAmount, AActor* DamageCauser)
{
    // PERFORMANCE: Validate inputs
    if (!HealthComponent || HealthComponent->IsInvulnerable() || DamageAmount <= 0.0f)
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
    return HealthComponent ? (HealthComponent->GetCurrentHealth() <= 0) : false;
}

void ARunnerCharacter::HandlePlayerDeath(int32 TotalHitsTaken)
{
    // PERFORMANCE: Early exit if already dead
    if (CurrentState == ECharacterState::Dead)
        return;

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

    // Call blueprint event
    DeathOfPlayer();

    // PERFORMANCE: Use optimized timer for level restart
    FTimerHandle RestartTimer;
    GetWorldTimerManager().SetTimer(RestartTimer, this, &ARunnerCharacter::RestartLevel, 
                                   RunnerCharacterConstants::RESTART_LEVEL_DELAY, false);
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
