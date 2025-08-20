#include "CoinPickup.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "CoinCounter.h"
#include "Engine/Engine.h"

#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
#include "DrawDebugHelpers.h"
#endif

// PERFORMANCE: Static pooling system with thread safety considerations
TMap<UWorld*, FActorPool<ACoinPickup>> ACoinPickup::CoinPools;

ACoinPickup::ACoinPickup()
{
    PrimaryActorTick.bCanEverTick = true;

    // PERFORMANCE: Create components with optimal setup
    CoinMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CoinMesh"));
    RootComponent = CoinMesh;
    CoinMesh->SetCollisionProfileName(TEXT("OverlapOnlyPawn"));
    CoinMesh->SetGenerateOverlapEvents(false); // Only collision sphere generates events

    // Collision sphere - primary interaction component
    CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
    CollisionSphere->SetupAttachment(RootComponent);
    CollisionSphere->SetSphereRadius(50.0f);
    CollisionSphere->SetCollisionProfileName(TEXT("OverlapOnlyPawn"));
    CollisionSphere->SetGenerateOverlapEvents(true);

    // Particle system for collection effects
    CollectParticles = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("CollectParticles"));
    CollectParticles->SetupAttachment(RootComponent);
    CollectParticles->bAutoActivate = false;

    // Magnet component for coin attraction
    CoinMagnet = CreateDefaultSubobject<USphereComponent>(TEXT("CoinMagnet"));
    CoinMagnet->SetupAttachment(RootComponent);
    CoinMagnet->SetSphereRadius(200.0f);
    CoinMagnet->SetCollisionProfileName(TEXT("OverlapOnlyPawn"));
    CoinMagnet->SetGenerateOverlapEvents(false); // Enabled conditionally
    CoinMagnet->SetVisibility(false);

    // PERFORMANCE: Initialize with optimized defaults
    RotationSpeed = 100.0f;
    HoverAmplitude = 10.0f;
    HoverFrequency = 2.0f;
    CoinValue = 1;
    bIsCollected = false;
    bCollected = false;
    CurrentTime = 0.0f;

    // Optimization settings
    bDisableTickWhenFar = true;
    TickDistance = 2000.0f;

    // Magnetism properties
    bEnableMagnetism = false;
    MagnetismSpeed = 500.0f;
    bMagnetActivated = false;
    TargetActor = nullptr;

    // Respawn and pooling
    bCanRespawn = false;
    RespawnTime = 10.0f;
    UseActorPooling = false;
    PoolTag = TEXT("DefaultCoin");

    // Debug and networking
    bShowDebugInfo = false;
    bReplicates = false; // Optimize by disabling replication unless needed
    NetUpdateFrequency = 0.1f; // Reduced frequency for better performance
}

void ACoinPickup::BeginPlay()
{
    Super::BeginPlay();

    // Store initial location for hover effects
    InitialLocation = GetActorLocation();

    // PERFORMANCE: Bind events only when needed
    if (CollisionSphere)
    {
        CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &ACoinPickup::OnPlayerOverlap);
    }

    // Setup magnetism if enabled
    if (bEnableMagnetism && CoinMagnet)
    {
        CoinMagnet->OnComponentBeginOverlap.AddDynamic(this, &ACoinPickup::OnMagnetOverlap);
        CoinMagnet->SetGenerateOverlapEvents(true);
    }

    // Initialize state
    ResetCoinState();
}

void ACoinPickup::ResetCoinState()
{
    bCollected = false;
    bIsCollected = false;
    bMagnetActivated = false;
    TargetActor = nullptr;
    
    // Ensure visibility and collision
    if (CoinMesh)
    {
        CoinMesh->SetVisibility(true);
    }
    
    if (CollisionSphere)
    {
        CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    }
}

void ACoinPickup::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // PERFORMANCE: Distance-based tick optimization
    if (bDisableTickWhenFar && !ShouldTickBasedOnDistance())
    {
        return;
    }

    // Don't animate if collected
    if (bIsCollected || bCollected)
    {
        return;
    }

    // Handle magnetism movement
    if (bMagnetActivated && TargetActor)
    {
        UpdateMagnetMovement(DeltaTime);
    }
    else
    {
        // Normal coin animation (rotation and hover)
        UpdateCoinAnimation(DeltaTime);
    }

    // PERFORMANCE: Debug info only in development builds
#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
    if (bShowDebugInfo)
    {
        DrawDebugInformation();
    }
#endif
}

bool ACoinPickup::ShouldTickBasedOnDistance() const
{
    const APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC || !PC->GetPawn())
    {
        return false;
    }

    // PERFORMANCE: Use squared distance to avoid sqrt calculation
    const float DistanceSquared = FVector::DistSquared(GetActorLocation(), PC->GetPawn()->GetActorLocation());
    return DistanceSquared <= (TickDistance * TickDistance);
}

void ACoinPickup::UpdateMagnetMovement(float DeltaTime)
{
    if (!TargetActor)
    {
        bMagnetActivated = false;
        return;
    }

    const FVector Direction = (TargetActor->GetActorLocation() - GetActorLocation()).GetSafeNormal();
    const FVector NewLocation = GetActorLocation() + Direction * MagnetismSpeed * DeltaTime;
    SetActorLocation(NewLocation);
}

void ACoinPickup::UpdateCoinAnimation(float DeltaTime)
{
    // PERFORMANCE: Combine rotation and hover updates
    CurrentTime += DeltaTime;
    
    // Rotate the coin
    FRotator NewRotation = GetActorRotation();
    NewRotation.Yaw += RotationSpeed * DeltaTime;
    
    // Calculate hover position
    FVector NewLocation = InitialLocation;
    NewLocation.Z += HoverAmplitude * FMath::Sin(HoverFrequency * CurrentTime);
    
    // Apply both transformations
    SetActorRotation(NewRotation);
    SetActorLocation(NewLocation);
}

void ACoinPickup::OnPlayerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    // PERFORMANCE: Quick validation and early exit
    if (!OtherActor || bIsCollected || bCollected)
        return;

    if (ACharacter* Character = Cast<ACharacter>(OtherActor))
    {
        Collect(Character);
    }
}

void ACoinPickup::OnMagnetOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    // PERFORMANCE: Enable magnetism only when valid
    if (!OtherActor || bIsCollected || bCollected || !bEnableMagnetism)
        return;

    if (ACharacter* Character = Cast<ACharacter>(OtherActor))
    {
        bMagnetActivated = true;
        TargetActor = OtherActor;
    }
}

void ACoinPickup::Collect_Implementation(ACharacter* Character)
{
    // PERFORMANCE: Atomic collection check to prevent double collection
    if (bIsCollected || bCollected)
        return;

    bIsCollected = true;
    bCollected = true;

    // Handle visual and audio effects
    HandleCollectionEffects();

    // PERFORMANCE: Optimized coin counter update
    UpdateCoinCounter(Character);

    // Broadcast collection event
    OnCoinCollected.Broadcast(this, Character);

    // Handle respawn or destruction
    HandlePostCollection();
}

void ACoinPickup::HandleCollectionEffects()
{
    // Hide the mesh
    if (CoinMesh)
    {
        CoinMesh->SetVisibility(false);
    }

    // Disable collision
    if (CollisionSphere)
    {
        CollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
    
    if (CoinMagnet)
    {
        CoinMagnet->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    // Play effects
    if (CollectParticles && CollectParticles->Template)
    {
        CollectParticles->ActivateSystem();
    }

    if (CollectSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, CollectSound, GetActorLocation());
    }
}

void ACoinPickup::UpdateCoinCounter(ACharacter* Character)
{
    if (!Character)
        return;

    // Try character first, then controller
    UCoinCounter* CoinCounterComp = Character->FindComponentByClass<UCoinCounter>();
    
    if (!CoinCounterComp)
    {
        if (APlayerController* PC = Character->GetController<APlayerController>())
        {
            CoinCounterComp = PC->FindComponentByClass<UCoinCounter>();
        }
    }

    if (CoinCounterComp)
    {
        CoinCounterComp->AddCoins(CoinValue);
#if UE_BUILD_DEVELOPMENT
        UE_LOG(LogTemp, Log, TEXT("Coin collected with value: %d"), CoinValue);
#endif
    }
#if UE_BUILD_DEVELOPMENT
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No CoinCounter found for coin value: %d"), CoinValue);
    }
#endif
}

void ACoinPickup::HandlePostCollection()
{
    if (bCanRespawn)
    {
        // Schedule respawn
        FTimerHandle RespawnTimer;
        GetWorldTimerManager().SetTimer(RespawnTimer, this, &ACoinPickup::Respawn, RespawnTime, false);
    }
    else if (UseActorPooling)
    {
        // Return to pool after brief delay for effects to finish
        FTimerHandle PoolTimer;
        GetWorldTimerManager().SetTimer(PoolTimer, [this]()
        {
            ReturnToPool();
        }, 1.0f, false);
    }
    else
    {
        // Simple destruction after effects
        FTimerHandle DestroyTimer;
        GetWorldTimerManager().SetTimer(DestroyTimer, [this]()
        {
            Destroy();
        }, 2.0f, false);
    }
}

void ACoinPickup::Respawn()
{
    ResetCoinState();

    // Reset position if magnetized
    SetActorLocation(InitialLocation);

    // Broadcast respawn event
    OnCoinRespawned.Broadcast(this);
}

// PERFORMANCE: Simplified pooling system
ACoinPickup* ACoinPickup::SpawnFromPool(UWorld* World, TSubclassOf<ACoinPickup> CoinClass,
    const FTransform& Transform, FName Tag)
{
    if (!World || !CoinClass)
    {
        return nullptr;
    }

    // Simple spawn if pooling is disabled
    ACoinPickup* NewCoin = World->SpawnActor<ACoinPickup>(CoinClass, Transform);
    if (!NewCoin || !NewCoin->UseActorPooling)
    {
        return NewCoin;
    }

    // Setup for pooled coin
    NewCoin->PoolTag = Tag;
    NewCoin->InitialLocation = Transform.GetLocation();
    NewCoin->ResetCoinState();
    
    return NewCoin;
}

void ACoinPickup::ReturnToPool()
{
    if (!UseActorPooling)
    {
        Destroy();
        return;
    }

    // Hide and disable
    SetActorHiddenInGame(true);
    SetActorEnableCollision(false);
    SetActorTickEnabled(false);

    // Reset state for reuse
    ResetCoinState();
}

void ACoinPickup::ClearPool(UWorld* World)
{
    if (World)
    {
        CoinPools.Remove(World);
    }
}

void ACoinPickup::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    // Clean up pools on level transition
    if (EndPlayReason == EEndPlayReason::LevelTransition ||
        EndPlayReason == EEndPlayReason::EndPlayInEditor)
    {
        ClearPool(GetWorld());
    }
}

#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
void ACoinPickup::DrawDebugInformation() const
{
    const UWorld* World = GetWorld();
    if (!World)
        return;

    // Draw collision sphere
    DrawDebugSphere(World, GetActorLocation(), CollisionSphere->GetScaledSphereRadius(), 
                   8, FColor::Green, false, -1.0f, 0, 1.0f);

    // Draw magnet range if enabled
    if (bEnableMagnetism)
    {
        DrawDebugSphere(World, GetActorLocation(), CoinMagnet->GetScaledSphereRadius(), 
                       16, FColor::Blue, false, -1.0f, 0, 0.5f);
    }

    // Display coin value
    const FString DebugText = FString::Printf(TEXT("Coin Value: %d"), CoinValue);
    DrawDebugString(World, GetActorLocation() + FVector(0, 0, 50), DebugText, nullptr, FColor::White, 0.0f);
}
#endif