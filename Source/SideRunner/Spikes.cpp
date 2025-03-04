#include "Spikes.h"
#include "Kismet/GameplayStatics.h"
#include "RunnerCharacter.h"
#include "Components/BoxComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/DamageEvents.h"

// Sets default values
ASpikes::ASpikes()
{
    // Set this actor to call Tick() every frame
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create collision box
    CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
    CollisionBox->SetupAttachment(RootComponent);
    CollisionBox->SetCollisionProfileName(TEXT("BlockAll"));
    CollisionBox->SetNotifyRigidBodyCollision(true); // Enable hit notifications

    // Create mesh component
    SpikeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SpikeMesh"));
    SpikeMesh->SetupAttachment(CollisionBox);
    SpikeMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // Collision handled by box

    // Create particle effect component
    ImpactEffect = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ImpactEffect"));
    ImpactEffect->SetupAttachment(RootComponent);
    ImpactEffect->bAutoActivate = false;

    // Initialize default values
    Speed = 100.0f;
    MaxMovementOffset = 100.0f;
    MovementDirection = 1;
    DamageAmount = 10.0f;
    DamageCooldown = 1.0f;
    DamageTimer = 0.0f;
    bIsMoving = true;
    MovementType = EMovementType::UpDown;
}

// Called when the game starts or when spawned
void ASpikes::BeginPlay()
{
    Super::BeginPlay();

    // Store initial position
    InitialPosition = GetActorLocation();

    // Hide impact effect until needed
    if (ImpactEffect)
    {
        ImpactEffect->Deactivate();
    }

    // Reset damage timer
    DamageTimer = 0.0f;

    // Log warning if collision sound is not set
    if (!CollisionSound)
    {
        UE_LOG(LogTemp, Warning, TEXT("%s: CollisionSound is not set in the editor!"), *GetName());
    }
}

// Called every frame
void ASpikes::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Decrease damage timer if active
    if (DamageTimer > 0.0f)
    {
        DamageTimer -= DeltaTime;
    }

    // If not moving, exit early
    if (!bIsMoving)
    {
        return;
    }

    // Handle movement based on selected type
    FVector NewLocation = GetActorLocation();

    switch (MovementType)
    {
    case EMovementType::UpDown:
        // Up/down movement
        NewLocation.Z += Speed * DeltaTime * MovementDirection;

        if ((MovementDirection == 1 && NewLocation.Z >= InitialPosition.Z + MaxMovementOffset) ||
            (MovementDirection == -1 && NewLocation.Z <= InitialPosition.Z))
        {
            MovementDirection *= -1;
        }
        break;

    case EMovementType::LeftRight:
        // Left/right movement
        NewLocation.X += Speed * DeltaTime * MovementDirection;

        if ((MovementDirection == 1 && NewLocation.X >= InitialPosition.X + MaxMovementOffset) ||
            (MovementDirection == -1 && NewLocation.X <= InitialPosition.X))
        {
            MovementDirection *= -1;
        }
        break;

    case EMovementType::FrontBack:
        // Front/back movement
        NewLocation.Y += Speed * DeltaTime * MovementDirection;

        if ((MovementDirection == 1 && NewLocation.Y >= InitialPosition.Y + MaxMovementOffset) ||
            (MovementDirection == -1 && NewLocation.Y <= InitialPosition.Y))
        {
            MovementDirection *= -1;
        }
        break;

    case EMovementType::Static:
        // No movement
        return;
    }

    SetActorLocation(NewLocation);
}

// Override the NotifyHit function to detect collisions
void ASpikes::NotifyHit(
    UPrimitiveComponent* MyComp,
    AActor* Other,
    UPrimitiveComponent* OtherComp,
    bool bSelfMoved,
    FVector HitLocation,
    FVector HitNormal,
    FVector NormalImpulse,
    const FHitResult& Hit)
{
    Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);

    // Debug message to confirm collision detection
    UE_LOG(LogTemp, Verbose, TEXT("%s: Hit detected with %s"), *GetName(), *Other->GetName());

    // Check if the colliding actor is the player character
    ARunnerCharacter* PlayerCharacter = Cast<ARunnerCharacter>(Other);
    if (PlayerCharacter)
    {
        // Play collision sound
        if (CollisionSound)
        {
            UGameplayStatics::PlaySoundAtLocation(this, CollisionSound, GetActorLocation());
        }

        // Show particle effect at collision point
        if (ImpactEffect)
        {
            ImpactEffect->SetWorldLocation(HitLocation);
            ImpactEffect->Activate(true);
        }

        // Apply damage to player (with cooldown)
        ApplyDamageToPlayer(PlayerCharacter);
    }
}

void ASpikes::SetMovementEnabled(bool bEnabled)
{
    bIsMoving = bEnabled;
}

void ASpikes::ApplyDamageToPlayer(AActor* Player)
{
    // Check if cooldown has expired
    if (DamageTimer <= 0.0f)
    {
        // Apply damage to player
        FDamageEvent DamageEvent;
        Player->TakeDamage(DamageAmount, DamageEvent, nullptr, this);

        // Set cooldown timer
        DamageTimer = DamageCooldown;

        // Log damage application
        UE_LOG(LogTemp, Log, TEXT("%s: Applied %.1f damage to %s"),
            *GetName(), DamageAmount, *Player->GetName());
    }
}
