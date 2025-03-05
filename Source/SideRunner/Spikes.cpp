#include "Spikes.h"
#include "Kismet/GameplayStatics.h"
#include "RunnerCharacter.h"
#include "Components/BoxComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/DamageEvents.h"
#include "DrawDebugHelpers.h"

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

    // Initialize new properties
    bProximityTriggered = false;
    TriggerRadius = 300.0f;
    bIsTriggered = false;
    CurrentTime = 0.0f;
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

    // Reset time for smooth movements
    CurrentTime = 0.0f;

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

    // For proximity triggered spikes, check if player is nearby
    if (bProximityTriggered)
    {
        ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(this, 0);
        if (PlayerCharacter)
        {
            float DistanceToPlayer = FVector::Dist(GetActorLocation(), PlayerCharacter->GetActorLocation());
            bIsTriggered = (DistanceToPlayer <= TriggerRadius);

            // If not triggered, don't move
            if (!bIsTriggered)
            {
                return;
            }
        }
    }

    // Update time for smooth oscillation
    CurrentTime += DeltaTime;

    // Handle movement based on selected type
    FVector NewLocation = GetActorLocation();

    switch (MovementType)
    {
    case EMovementType::UpDown:
        // Up/down movement using smooth sine wave
        NewLocation.Z = InitialPosition.Z + FMath::Sin(CurrentTime * Speed / 100.0f) * MaxMovementOffset;
        break;

    case EMovementType::LeftRight:
        // Left/right movement using smooth sine wave
        NewLocation.X = InitialPosition.X + FMath::Sin(CurrentTime * Speed / 100.0f) * MaxMovementOffset;
        break;

    case EMovementType::FrontBack:
        // Front/back movement using smooth sine wave
        NewLocation.Y = InitialPosition.Y + FMath::Sin(CurrentTime * Speed / 100.0f) * MaxMovementOffset;
        break;

    case EMovementType::Circular:
        // Circular movement pattern
        NewLocation.X = InitialPosition.X + FMath::Sin(CurrentTime * Speed / 100.0f) * MaxMovementOffset;
        NewLocation.Y = InitialPosition.Y + FMath::Cos(CurrentTime * Speed / 100.0f) * MaxMovementOffset;
        break;

    case EMovementType::Zigzag:
    {
        // Create a zigzag pattern using modulo and absolute values
        float CyclePosition = FMath::Fmod(CurrentTime * Speed / 50.0f, 4.0f);
        if (CyclePosition < 1.0f)
            NewLocation.X = InitialPosition.X + CyclePosition * MaxMovementOffset;
        else if (CyclePosition < 2.0f)
            NewLocation.X = InitialPosition.X + MaxMovementOffset;
        else if (CyclePosition < 3.0f)
            NewLocation.X = InitialPosition.X + (3.0f - CyclePosition) * MaxMovementOffset;
        else
            NewLocation.X = InitialPosition.X;

        // Add slight vertical movement to zigzag
        NewLocation.Z = InitialPosition.Z + FMath::Sin(CurrentTime * Speed / 50.0f) * (MaxMovementOffset * 0.2f);
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

#if WITH_EDITOR
// Draw debug visualization for editor
void ASpikes::DrawDebugMovementPath()
{
    if (GetWorld())
    {
        // Clear any previous debug shapes for this actor
        FlushDebugStrings(GetWorld());

        // Different visualizations based on movement type
        switch (MovementType)
        {
        case EMovementType::Static:
            // No movement, no visualization needed
            break;

        case EMovementType::UpDown:
        {
            FVector TopPoint = InitialPosition + FVector(0, 0, MaxMovementOffset);
            FVector BottomPoint = InitialPosition - FVector(0, 0, MaxMovementOffset);
            DrawDebugLine(GetWorld(), TopPoint, BottomPoint, FColor::Yellow, true, -1.0f, 0, 2.0f);

            // Add labels
            DrawDebugString(GetWorld(), TopPoint, TEXT("Max Height"), nullptr, FColor::White, -1.0f, true);
            DrawDebugString(GetWorld(), BottomPoint, TEXT("Min Height"), nullptr, FColor::White, -1.0f, true);
        }
        break;

        case EMovementType::LeftRight:
        {
            FVector LeftPoint = InitialPosition - FVector(MaxMovementOffset, 0, 0);
            FVector RightPoint = InitialPosition + FVector(MaxMovementOffset, 0, 0);
            DrawDebugLine(GetWorld(), LeftPoint, RightPoint, FColor::Yellow, true, -1.0f, 0, 2.0f);

            // Add labels
            DrawDebugString(GetWorld(), LeftPoint, TEXT("Left Extent"), nullptr, FColor::White, -1.0f, true);
            DrawDebugString(GetWorld(), RightPoint, TEXT("Right Extent"), nullptr, FColor::White, -1.0f, true);
        }
        break;

        case EMovementType::FrontBack:
        {
            FVector FrontPoint = InitialPosition + FVector(0, MaxMovementOffset, 0);
            FVector BackPoint = InitialPosition - FVector(0, MaxMovementOffset, 0);
            DrawDebugLine(GetWorld(), FrontPoint, BackPoint, FColor::Yellow, true, -1.0f, 0, 2.0f);

            // Add labels
            DrawDebugString(GetWorld(), FrontPoint, TEXT("Front Extent"), nullptr, FColor::White, -1.0f, true);
            DrawDebugString(GetWorld(), BackPoint, TEXT("Back Extent"), nullptr, FColor::White, -1.0f, true);
        }
        break;

        case EMovementType::Circular:
        {
            const int32 NumSegments = 32;
            for (int32 i = 0; i < NumSegments; i++)
            {
                float Angle1 = (float)i / NumSegments * 2.0f * PI;
                float Angle2 = (float)(i + 1) / NumSegments * 2.0f * PI;

                FVector Point1 = InitialPosition + FVector(
                    FMath::Sin(Angle1) * MaxMovementOffset,
                    FMath::Cos(Angle1) * MaxMovementOffset,
                    0);
                FVector Point2 = InitialPosition + FVector(
                    FMath::Sin(Angle2) * MaxMovementOffset,
                    FMath::Cos(Angle2) * MaxMovementOffset,
                    0);

                DrawDebugLine(GetWorld(), Point1, Point2, FColor::Yellow, true, -1.0f, 0, 2.0f);
            }

            // Add center label
            DrawDebugString(GetWorld(), InitialPosition, TEXT("Center"), nullptr, FColor::White, -1.0f, true);
        }
        break;

        case EMovementType::Zigzag:
        {
            const int32 NumPoints = 5;
            FVector Points[NumPoints];

            // Generate zigzag points
            Points[0] = InitialPosition;
            Points[1] = InitialPosition + FVector(MaxMovementOffset * 0.25f, 0, MaxMovementOffset * 0.2f);
            Points[2] = InitialPosition + FVector(MaxMovementOffset * 0.5f, 0, 0);
            Points[3] = InitialPosition + FVector(MaxMovementOffset * 0.75f, 0, MaxMovementOffset * 0.2f);
            Points[4] = InitialPosition + FVector(MaxMovementOffset, 0, 0);

            // Draw lines between points
            for (int32 i = 0; i < NumPoints - 1; i++)
            {
                DrawDebugLine(GetWorld(), Points[i], Points[i + 1], FColor::Yellow, true, -1.0f, 0, 2.0f);
            }

            // Add labels
            DrawDebugString(GetWorld(), Points[0], TEXT("Start"), nullptr, FColor::White, -1.0f, true);
            DrawDebugString(GetWorld(), Points[4], TEXT("End"), nullptr, FColor::White, -1.0f, true);
        }
        break;
        }

        // If proximity triggered, draw the trigger radius
        if (bProximityTriggered)
        {
            DrawDebugSphere(GetWorld(), InitialPosition, TriggerRadius, 32, FColor::Red, true, -1.0f, 0, 1.0f);
            DrawDebugString(GetWorld(), InitialPosition + FVector(0, 0, TriggerRadius + 20.0f),
                FString::Printf(TEXT("Trigger Radius: %.1f"), TriggerRadius),
                nullptr, FColor::Red, -1.0f, true);
        }
    }
}

// Handle property changes in editor
void ASpikes::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    // Get the name of the property that was changed
    const FName PropertyName = (PropertyChangedEvent.Property != nullptr) ?
        PropertyChangedEvent.Property->GetFName() : NAME_None;

    // If movement related properties changed, update the movement path visualization
    if (PropertyName == GET_MEMBER_NAME_CHECKED(ASpikes, MovementType) ||
        PropertyName == GET_MEMBER_NAME_CHECKED(ASpikes, MaxMovementOffset) ||
        PropertyName == GET_MEMBER_NAME_CHECKED(ASpikes, bProximityTriggered) ||
        PropertyName == GET_MEMBER_NAME_CHECKED(ASpikes, TriggerRadius))
    {
        DrawDebugMovementPath();
    }
}
#endif