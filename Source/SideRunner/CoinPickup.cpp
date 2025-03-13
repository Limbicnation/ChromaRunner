#include "CoinPickup.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "CoinCounter.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"

// Pooling system statics
TMap<UWorld*, FActorPool<ACoinPickup>> ACoinPickup::CoinPools;

// Sets default values
ACoinPickup::ACoinPickup()
{
    // Set this actor to call Tick() every frame
    PrimaryActorTick.bCanEverTick = true;

    // Create the static mesh component
    CoinMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CoinMesh"));
    RootComponent = CoinMesh;

    // Configure the mesh to not block anything but be visible
    CoinMesh->SetCollisionProfileName(TEXT("OverlapOnlyPawn"));
    CoinMesh->SetGenerateOverlapEvents(false);

    // Create the collision sphere
    CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
    CollisionSphere->SetupAttachment(RootComponent);
    CollisionSphere->SetSphereRadius(50.0f);
    CollisionSphere->SetCollisionProfileName(TEXT("OverlapOnlyPawn"));
    CollisionSphere->SetGenerateOverlapEvents(true);

    // Create the particle system component
    CollectParticles = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("CollectParticles"));
    CollectParticles->SetupAttachment(RootComponent);
    CollectParticles->bAutoActivate = false;

    // Create the coin magnet
    CoinMagnet = CreateDefaultSubobject<USphereComponent>(TEXT("CoinMagnet"));
    CoinMagnet->SetupAttachment(RootComponent);
    CoinMagnet->SetSphereRadius(200.0f);
    CoinMagnet->SetCollisionProfileName(TEXT("OverlapOnlyPawn"));
    CoinMagnet->SetGenerateOverlapEvents(true);
    CoinMagnet->SetVisibility(false);

    // Set default values
    RotationSpeed = 100.0f;
    HoverAmplitude = 10.0f;
    HoverFrequency = 2.0f;
    CoinValue = 1;
    bIsCollected = false;
    CurrentTime = 0.0f;

    // Optimization flags
    bDisableTickWhenFar = true;
    TickDistance = 2000.0f;
    bCollected = false;

    // Magnetism properties
    bEnableMagnetism = false;
    MagnetismSpeed = 500.0f;
    bMagnetActivated = false;
    TargetActor = nullptr;

    // Respawn properties
    bCanRespawn = false;
    RespawnTime = 10.0f;

    // Debug visualization
    bShowDebugInfo = false;

    // Pool tag
    PoolTag = TEXT("DefaultCoin");

    // Set network settings
    bReplicates = true;
    NetUpdateFrequency = 0.5f;

    // Object pooling
    UseActorPooling = false;
}

// Called when the game starts or when spawned
void ACoinPickup::BeginPlay()
{
    Super::BeginPlay();

    // Store the initial location for the hover effect
    InitialLocation = GetActorLocation();

    // Bind the overlap event
    CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &ACoinPickup::OnPlayerOverlap);

    // If magnetism is enabled, bind the magnet overlap
    if (bEnableMagnetism)
    {
        CoinMagnet->OnComponentBeginOverlap.AddDynamic(this, &ACoinPickup::OnMagnetOverlap);
        CoinMagnet->SetGenerateOverlapEvents(true);
    }
    else
    {
        CoinMagnet->SetGenerateOverlapEvents(false);
    }

    // Make sure to initialize as not collected
    bCollected = false;
    bIsCollected = false;

    // Set visibility and collision
    CoinMesh->SetVisibility(true);
    CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

// Called every frame
void ACoinPickup::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Check if we should be ticking based on distance to player
    if (bDisableTickWhenFar)
    {
        APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
        if (PC && PC->GetPawn())
        {
            APawn* Pawn = PC->GetPawn();
            float Distance = FVector::Dist(GetActorLocation(), Pawn->GetActorLocation());

            // Don't update if too far away, but don't disable tick because we need to check distance
            if (Distance > TickDistance)
            {
                return;
            }
        }
    }

    // Don't do any animations if collected
    if (!bIsCollected && !bCollected)
    {
        // Check for magnetism
        if (bMagnetActivated && TargetActor)
        {
            FVector Direction = (TargetActor->GetActorLocation() - GetActorLocation()).GetSafeNormal();
            FVector NewLocation = GetActorLocation() + Direction * MagnetismSpeed * DeltaTime;
            SetActorLocation(NewLocation);
        }
        else
        {
            // Rotate the coin
            FRotator NewRotation = GetActorRotation();
            NewRotation.Yaw += RotationSpeed * DeltaTime;
            SetActorRotation(NewRotation);

            // Make the coin hover up and down
            CurrentTime += DeltaTime;
            FVector NewLocation = InitialLocation;
            NewLocation.Z += HoverAmplitude * FMath::Sin(HoverFrequency * CurrentTime);
            SetActorLocation(NewLocation);
        }

        // Debug visualization
        if (bShowDebugInfo)
        {
            DrawDebugSphere(GetWorld(), GetActorLocation(), CollisionSphere->GetScaledSphereRadius(), 8, FColor::Green, false, -1.0f, 0, 1.0f);

            if (bEnableMagnetism)
            {
                DrawDebugSphere(GetWorld(), GetActorLocation(), CoinMagnet->GetScaledSphereRadius(), 16, FColor::Blue, false, -1.0f, 0, 0.5f);
            }

            // Display value
            FString DebugText = FString::Printf(TEXT("Coin Value: %d"), CoinValue);
            DrawDebugString(GetWorld(), GetActorLocation() + FVector(0, 0, 50), DebugText, nullptr, FColor::White, 0.0f);
        }
    }
}

void ACoinPickup::OnPlayerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    // Check if the overlapping actor is a character (likely the player)
    ACharacter* Character = Cast<ACharacter>(OtherActor);
    if (Character && !bIsCollected && !bCollected)
    {
        Collect(Character);
    }
}

void ACoinPickup::OnMagnetOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    // Check if the overlapping actor is a character (likely the player)
    ACharacter* Character = Cast<ACharacter>(OtherActor);
    if (Character && !bIsCollected && !bCollected && bEnableMagnetism)
    {
        // Enable magnetism
        bMagnetActivated = true;
        TargetActor = OtherActor;
    }
}

void ACoinPickup::Collect_Implementation(ACharacter* Character)
{
    if (!bIsCollected && !bCollected)
    {
        bIsCollected = true;
        bCollected = true;

        // Hide the coin mesh
        CoinMesh->SetVisibility(false);

        // Disable collision
        CollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        CoinMagnet->SetCollisionEnabled(ECollisionEnabled::NoCollision);

        // Play the collection particles
        if (CollectParticles && CollectParticles->Template)
        {
            CollectParticles->ActivateSystem();
        }

        // Play the collection sound
        if (CollectSound)
        {
            UGameplayStatics::PlaySoundAtLocation(this, CollectSound, GetActorLocation());
        }

        // UPDATED: Find and update the CoinCounter
        if (Character)
        {
            // First try to find CoinCounter on the character
            UCoinCounter* CoinCounterComp = Character->FindComponentByClass<UCoinCounter>();

            if (CoinCounterComp)
            {
                CoinCounterComp->AddCoins(CoinValue);
                UE_LOG(LogTemp, Log, TEXT("Coin collected with value: %d, added to character's counter"), CoinValue);
            }
            else
            {
                // Try to find CoinCounter on the player controller
                APlayerController* PC = Character->GetController<APlayerController>();
                if (PC)
                {
                    UCoinCounter* ControllerCoinCounter = PC->FindComponentByClass<UCoinCounter>();
                    if (ControllerCoinCounter)
                    {
                        ControllerCoinCounter->AddCoins(CoinValue);
                        UE_LOG(LogTemp, Log, TEXT("Coin collected with value: %d, added to controller's counter"), CoinValue);
                    }
                    else
                    {
                        UE_LOG(LogTemp, Warning, TEXT("No CoinCounter found on character or controller for coin value: %d"), CoinValue);
                    }
                }
            }
        }

        // Broadcast the collected event
        OnCoinCollected.Broadcast(this, Character);

        // Rest of the existing code...
    }
}

void ACoinPickup::Respawn()
{
    // Reset collection state
    bIsCollected = false;
    bCollected = false;
    bMagnetActivated = false;

    // Show the coin mesh
    CoinMesh->SetVisibility(true);

    // Enable collision
    CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

    // Enable magnet if needed
    if (bEnableMagnetism)
    {
        CoinMagnet->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    }

    // Reset position if it was magnetized
    SetActorLocation(InitialLocation);

    // Broadcast respawn event
    OnCoinRespawned.Broadcast(this);
}

ACoinPickup* ACoinPickup::SpawnFromPool(UWorld* World, TSubclassOf<ACoinPickup> CoinClass,
    const FTransform& Transform, FName Tag)
{
    if (!World || !CoinClass)
    {
        return nullptr;
    }

    ACoinPickup* TestCoin = World->SpawnActor<ACoinPickup>(CoinClass, Transform);
    if (!TestCoin || !TestCoin->UseActorPooling)
    {
        // If pooling is disabled or invalid parameters, spawn normally
        return TestCoin;
    }

    // If we're here, we need to destroy the test coin and use pooling
    if (TestCoin)
    {
        TestCoin->Destroy();
    }

    // Get or create the pool for this world
    FActorPool<ACoinPickup>& Pool = CoinPools.FindOrAdd(World);

    // Try to get an actor from the pool
    ACoinPickup* Coin = Pool.GetActor(Tag);

    if (Coin)
    {
        // Reset and reuse the existing coin
        Coin->SetActorTransform(Transform);
        Coin->InitialLocation = Transform.GetLocation();
        Coin->CurrentTime = 0.0f;
        Coin->bIsCollected = false;
        Coin->bCollected = false;
        Coin->bMagnetActivated = false;
        Coin->CoinMesh->SetVisibility(true);
        Coin->CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        Coin->PoolTag = Tag;
        Coin->SetActorHiddenInGame(false);
        Coin->SetActorEnableCollision(true);
        Coin->SetActorTickEnabled(true);

        // Return the reused coin
        return Coin;
    }
    else
    {
        // Spawn a new coin if none available in the pool
        ACoinPickup* NewCoin = World->SpawnActor<ACoinPickup>(CoinClass, Transform);
        if (NewCoin)
        {
            NewCoin->UseActorPooling = true;
            NewCoin->PoolTag = Tag;
        }
        return NewCoin;
    }
}

void ACoinPickup::ReturnToPool()
{
    if (!UseActorPooling)
    {
        // If pooling is disabled, just destroy the actor
        Destroy();
        return;
    }

    // Get the pool for this world
    FActorPool<ACoinPickup>* Pool = CoinPools.Find(GetWorld());
    if (Pool)
    {
        // Reset the coin for reuse
        CoinMesh->SetVisibility(false);
        CollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        CoinMagnet->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        SetActorHiddenInGame(true);
        SetActorEnableCollision(false);
        SetActorTickEnabled(false);

        // Return to pool
        Pool->ReturnActor(this, PoolTag);
    }
    else
    {
        // If no pool exists, just destroy the actor
        Destroy();
    }
}

void ACoinPickup::ClearPool(UWorld* World)
{
    // Remove the pool for the specified world
    CoinPools.Remove(World);
}

void ACoinPickup::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    // If the level is being unloaded, clean up any pooled actors for this world
    if (EndPlayReason == EEndPlayReason::LevelTransition ||
        EndPlayReason == EEndPlayReason::EndPlayInEditor)
    {
        ClearPool(GetWorld());
    }
}