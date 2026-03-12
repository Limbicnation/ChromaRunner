#include "ProceduralLevelBuilder.h"
#include "Engine/World.h"
#include "Spikes.h"
#include "CoinPickup.h"
#include "SimpleEnemy.h"
#include "SideRunner.h" // Custom log categories

UProceduralLevelBuilder::UProceduralLevelBuilder()
{
    PrimaryComponentTick.bCanEverTick = false;

    // Platform config defaults
    ChunkLength = 2000.0f;
    MinPlatformWidth = 150.0f;
    MaxPlatformWidth = 400.0f;
    MinGapSize = 100.0f;
    MaxGapSize = 350.0f;
    BasePlatformMeshSize = 100.0f;

    // Physics constraints from RunnerCharacter constructor
    JumpZVelocity = 1000.0f;
    DoubleJumpZVelocity = 800.0f;
    GravityScale = 2.5f;
    MaxWalkSpeed = 600.0f;

    // Compute jump distances
    CalculateJumpDistances();
}

// ======================================================================
// Jump Distance Calculation
// ======================================================================

void UProceduralLevelBuilder::CalculateJumpDistances()
{
    // Physics: time_in_air = 2 * Vz / (g * GravityScale)
    // horizontal_distance = MaxWalkSpeed * time_in_air
    const float Gravity = 980.0f; // UE default gravity in cm/s²
    const float EffectiveGravity = Gravity * GravityScale;

    // Single jump
    const float SingleJumpTime = 2.0f * JumpZVelocity / EffectiveGravity;
    MaxSingleJumpDistance = MaxWalkSpeed * SingleJumpTime;

    // Double jump (additive impulse during arc)
    // Approximation: total air time includes double jump impulse
    const float DoubleJumpTime = (2.0f * JumpZVelocity / EffectiveGravity) +
                                  (2.0f * DoubleJumpZVelocity / EffectiveGravity);
    MaxDoubleJumpDistance = MaxWalkSpeed * DoubleJumpTime;

    // Safety margin: use 85% of theoretical max to account for reaction time
    MaxSingleJumpDistance *= 0.85f;
    MaxDoubleJumpDistance *= 0.85f;

    UE_LOG(LogSideRunner, Log, TEXT("ProceduralLevelBuilder: MaxSingleJump=%.0f MaxDoubleJump=%.0f"),
           MaxSingleJumpDistance, MaxDoubleJumpDistance);
}

// ======================================================================
// Pool-or-Spawn Helper
// ======================================================================

AActor* UProceduralLevelBuilder::GetOrSpawnActor(FActorPool<AActor>& Pool, TArray<AActor*>& GCRefs,
    UWorld* World, UClass* ActorClass, const FVector& SpawnLocation)
{
    AActor* Actor = Pool.GetActor();
    if (Actor)
    {
        GCRefs.Remove(Actor);
        Actor->SetActorLocation(SpawnLocation);
        Actor->SetActorHiddenInGame(false);
        Actor->SetActorEnableCollision(true);
        Actor->SetActorTickEnabled(true);
    }
    else if (ActorClass)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        Actor = World->SpawnActor<AActor>(ActorClass, SpawnLocation,
            FRotator::ZeroRotator, SpawnParams);
    }
    return Actor;
}

// ======================================================================
// Core Generation
// ======================================================================

TArray<AActor*> UProceduralLevelBuilder::GenerateLevelContent(UWorld* World, float StartY, float Difficulty, int32 Seed)
{
    TArray<AActor*> SpawnedActors;

    if (!World)
    {
        UE_LOG(LogSideRunner, Error, TEXT("ProceduralLevelBuilder: World is null"));
        return SpawnedActors;
    }

    // Clamp difficulty to valid range
    Difficulty = FMath::Clamp(Difficulty, 1.0f, 10.0f);

    FRandomStream RandomStream(Seed);

    // Phase 1: Generate platforms (controlled random walk)
    TArray<FPlatformPlacement> Placements;
    GeneratePlatforms(World, StartY, Difficulty, RandomStream, SpawnedActors, Placements);

    // Phase 2: Place obstacles on platforms
    GenerateObstacles(World, Difficulty, RandomStream, Placements, SpawnedActors);

    // Phase 2: Place coins
    GenerateCoins(World, Difficulty, RandomStream, Placements, SpawnedActors);

#if UE_BUILD_DEVELOPMENT
    UE_LOG(LogSideRunner, Log, TEXT("ProceduralLevelBuilder: Generated %d actors (Difficulty=%.1f, Seed=%d, StartY=%.0f)"),
           SpawnedActors.Num(), Difficulty, Seed, StartY);
#endif

    return SpawnedActors;
}

// ======================================================================
// Platform Generation (Controlled Random Walk)
// ======================================================================

void UProceduralLevelBuilder::GeneratePlatforms(UWorld* World, float StartY, float Difficulty,
    FRandomStream& RandomStream, TArray<AActor*>& OutActors, TArray<FPlatformPlacement>& OutPlacements)
{
    // Determine platform class to use
    TSubclassOf<AActor> EffectivePlatformClass = PlatformClass;
    if (!EffectivePlatformClass)
    {
        UE_LOG(LogSideRunner, Warning, TEXT("ProceduralLevelBuilder: No PlatformClass set, skipping platform generation"));
        return;
    }

    const float DifficultyAlpha = GetDifficultyAlpha(Difficulty);
    float CurrentY = StartY;
    const float EndY = StartY + ChunkLength;

    while (CurrentY < EndY)
    {
        FPlatformPlacement Placement;

        // Platform width shrinks with difficulty
        Placement.Width = FMath::Lerp(MaxPlatformWidth, MinPlatformWidth, DifficultyAlpha);

        // Add slight random variation (±10%)
        Placement.Width *= RandomStream.FRandRange(0.9f, 1.1f);
        Placement.Width = FMath::Clamp(Placement.Width, MinPlatformWidth, MaxPlatformWidth);

        // Platform Y position
        Placement.YPosition = CurrentY;

        // Height variation increases with difficulty
        const float MaxHeightVariation = FMath::Lerp(0.0f, 200.0f, DifficultyAlpha);
        Placement.ZPosition = BaseGroundZ + RandomStream.FRandRange(-MaxHeightVariation * 0.3f, MaxHeightVariation);

        // Moving platform chance
        const float MovingChance = FMath::Lerp(0.05f, 0.4f, DifficultyAlpha);
        Placement.bIsMoving = RandomStream.FRand() < MovingChance;

        // Coin chance
        const float CoinChance = 0.3f + Difficulty * 0.05f;
        Placement.bHasCollectible = RandomStream.FRand() < CoinChance;

        // Platform length (Y-axis depth)
        Placement.Length = RandomStream.FRandRange(200.0f, 400.0f);

        OutPlacements.Add(Placement);

        // Select platform variant for visual variety
        TSubclassOf<AActor> SpawnClass = EffectivePlatformClass;
        if (PlatformVariants.Num() > 0)
        {
            const int32 VariantIndex = RandomStream.RandRange(0, PlatformVariants.Num() - 1);
            if (PlatformVariants[VariantIndex])
            {
                SpawnClass = PlatformVariants[VariantIndex];
            }
        }

        // Spawn platform (try pool first, then spawn new)
        const FVector PlatformLocation(0.0f, Placement.YPosition, Placement.ZPosition);
        AActor* Platform = GetOrSpawnActor(PlatformPool, PlatformPoolGCRefs, World, SpawnClass, PlatformLocation);

        if (Platform)
        {
            // Scale platform to desired width
            FVector CurrentScale = Platform->GetActorScale3D();
            CurrentScale.Y = Placement.Width / BasePlatformMeshSize;
            Platform->SetActorScale3D(CurrentScale);

            OutActors.Add(Platform);
        }

        // Gap to next platform
        float GapSize = FMath::Lerp(MinGapSize, MaxGapSize, DifficultyAlpha);
        GapSize *= RandomStream.FRandRange(0.8f, 1.2f);

        // CRITICAL: Validate gap is jumpable — never exceed max double-jump distance
        GapSize = FMath::Min(GapSize, MaxDoubleJumpDistance * 0.9f);
        GapSize = FMath::Max(GapSize, MinGapSize);

        // Advance position past platform + gap
        CurrentY += Placement.Width + GapSize;
    }
}

// ======================================================================
// Obstacle Generation
// ======================================================================

void UProceduralLevelBuilder::GenerateObstacles(UWorld* World, float Difficulty, FRandomStream& RandomStream,
    const TArray<FPlatformPlacement>& Placements, TArray<AActor*>& OutActors)
{
    if (ObstacleClasses.Num() == 0)
    {
        return; // No obstacle classes configured
    }

    const float DifficultyAlpha = GetDifficultyAlpha(Difficulty);

    // Obstacle density: 10% at difficulty 1, 60% at difficulty 10
    const float ObstacleDensity = FMath::Lerp(0.1f, 0.6f, DifficultyAlpha);

    for (int32 i = 0; i < Placements.Num(); ++i)
    {
        const FPlatformPlacement& Placement = Placements[i];

        // Skip first platform (give player safe landing zone)
        if (i == 0)
        {
            continue;
        }

        // Determine if this platform should have an obstacle
        if (RandomStream.FRand() >= ObstacleDensity)
        {
            continue;
        }

        // Select obstacle class
        const int32 ClassIndex = RandomStream.RandRange(0, ObstacleClasses.Num() - 1);
        TSubclassOf<ASpikes> SpikeClass = ObstacleClasses[ClassIndex];
        if (!SpikeClass)
        {
            continue;
        }

        // Spawn obstacle (try pool first, then spawn new)
        FVector SpawnLocation(0.0f, Placement.YPosition + Placement.Width * 0.5f, Placement.ZPosition + 50.0f);
        AActor* Obstacle = GetOrSpawnActor(ObstaclePool, ObstaclePoolGCRefs, World, SpikeClass, SpawnLocation);

        if (Obstacle)
        {
            // Configure movement type based on difficulty
            if (ASpikes* Spike = Cast<ASpikes>(Obstacle))
            {
                const uint8 MovementTypeVal = SelectMovementTypeForDifficulty(Difficulty, RandomStream);
                Spike->MovementType = static_cast<EMovementType>(MovementTypeVal);

                // Enable movement for non-static types
                Spike->bIsMoving = (Spike->MovementType != EMovementType::Static);
            }

            OutActors.Add(Obstacle);
        }
    }

    // Wall spike: rare event at difficulty 5+ (5% chance per chunk)
    if (Difficulty >= 5.0f && WallSpikeClass && RandomStream.FRand() < WallSpikeChancePerChunk)
    {
        // Spawn wall spike at a random Y position within the chunk
        if (Placements.Num() > 2)
        {
            const int32 PlacementIndex = RandomStream.RandRange(1, Placements.Num() - 1);
            const FPlatformPlacement& Placement = Placements[PlacementIndex];

            FActorSpawnParameters SpawnParams;
            SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

            AActor* WallSpike = World->SpawnActor<AActor>(WallSpikeClass,
                FVector(0.0f, Placement.YPosition - 500.0f, Placement.ZPosition),
                FRotator::ZeroRotator, SpawnParams);

            if (WallSpike)
            {
                OutActors.Add(WallSpike);
            }
        }
    }
}

// ======================================================================
// Movement Type Selection
// ======================================================================

uint8 UProceduralLevelBuilder::SelectMovementTypeForDifficulty(float Difficulty, FRandomStream& RandomStream) const
{
    // Difficulty 1-3: Static only
    if (Difficulty < 4.0f)
    {
        return static_cast<uint8>(EMovementType::Static);
    }

    // Difficulty 4-6: Static + UpDown/LeftRight
    if (Difficulty < 7.0f)
    {
        const int32 Choice = RandomStream.RandRange(0, 2);
        switch (Choice)
        {
        case 0: return static_cast<uint8>(EMovementType::Static);
        case 1: return static_cast<uint8>(EMovementType::UpDown);
        case 2: return static_cast<uint8>(EMovementType::LeftRight);
        default: return static_cast<uint8>(EMovementType::Static);
        }
    }

    // Difficulty 7+: All movement types including Circular/Zigzag
    const int32 Choice = RandomStream.RandRange(0, 4);
    switch (Choice)
    {
    case 0: return static_cast<uint8>(EMovementType::Static);
    case 1: return static_cast<uint8>(EMovementType::UpDown);
    case 2: return static_cast<uint8>(EMovementType::LeftRight);
    case 3: return static_cast<uint8>(EMovementType::Circular);
    case 4: return static_cast<uint8>(EMovementType::Zigzag);
    default: return static_cast<uint8>(EMovementType::Static);
    }
}

// ======================================================================
// Coin Generation
// ======================================================================

void UProceduralLevelBuilder::GenerateCoins(UWorld* World, float Difficulty, FRandomStream& RandomStream,
    const TArray<FPlatformPlacement>& Placements, TArray<AActor*>& OutActors)
{
    if (!CoinClass)
    {
        return; // No coin class configured
    }

    for (const FPlatformPlacement& Placement : Placements)
    {
        if (!Placement.bHasCollectible)
        {
            continue;
        }

        // Place coin above center of platform
        FVector CoinLocation(0.0f, Placement.YPosition + Placement.Width * 0.5f,
                             Placement.ZPosition + CoinHeightOffset);

        AActor* Coin = GetOrSpawnActor(CoinPool, CoinPoolGCRefs, World, CoinClass, CoinLocation);

        // Reset coin state for reused coins
        if (ACoinPickup* CoinPickup = Cast<ACoinPickup>(Coin))
        {
            CoinPickup->Respawn();
        }

        if (Coin)
        {
            OutActors.Add(Coin);
        }
    }

    // Phase 4: Coin arcs between platforms (at higher difficulty)
    if (Difficulty >= 3.0f && Placements.Num() >= 2)
    {
        for (int32 i = 0; i < Placements.Num() - 1; ++i)
        {
            // 20% chance for a coin arc between platforms
            if (RandomStream.FRand() > 0.2f)
            {
                continue;
            }

            const FPlatformPlacement& Current = Placements[i];
            const FPlatformPlacement& Next = Placements[i + 1];

            const float GapCenter = Current.YPosition + Current.Width +
                                    (Next.YPosition - (Current.YPosition + Current.Width)) * 0.5f;
            const float ArcHeight = FMath::Max(Current.ZPosition, Next.ZPosition) + 200.0f;

            // Place 3 coins in an arc pattern
            for (int32 CoinIdx = 0; CoinIdx < 3; ++CoinIdx)
            {
                const float T = (CoinIdx + 1) / 4.0f;
                const float CoinY = FMath::Lerp(Current.YPosition + Current.Width, Next.YPosition, T);
                const float ParabolaT = T * 2.0f - 1.0f; // Map to [-1, 1]
                const float CoinZ = ArcHeight - (ParabolaT * ParabolaT * 100.0f);

                const FVector ArcCoinLocation(0.0f, CoinY, CoinZ);

                AActor* ArcCoin = GetOrSpawnActor(CoinPool, CoinPoolGCRefs, World, CoinClass, ArcCoinLocation);

                // Reset coin state for reused coins
                if (ACoinPickup* CoinPickup = Cast<ACoinPickup>(ArcCoin))
                {
                    CoinPickup->Respawn();
                }

                if (ArcCoin)
                {
                    OutActors.Add(ArcCoin);
                }
            }
        }
    }
}

// ======================================================================
// Actor Type Identification
// ======================================================================

bool UProceduralLevelBuilder::IsPlatformActor(const AActor* Actor) const
{
    if (!Actor)
    {
        return false;
    }

    if (PlatformClass && Actor->IsA(PlatformClass))
    {
        return true;
    }

    for (const TSubclassOf<AActor>& Variant : PlatformVariants)
    {
        if (Variant && Actor->IsA(Variant))
        {
            return true;
        }
    }

    return false;
}

// ======================================================================
// Pool Management
// ======================================================================

void UProceduralLevelBuilder::ReturnActorsToPool(const TArray<AActor*>& Actors)
{
    for (AActor* Actor : Actors)
    {
        if (!IsValid(Actor))
        {
            continue;
        }

        // Deactivate the actor
        Actor->SetActorHiddenInGame(true);
        Actor->SetActorEnableCollision(false);
        Actor->SetActorTickEnabled(false);

        // Return to appropriate pool based on class and add GC root reference
        if (Actor->IsA(ASpikes::StaticClass()))
        {
            ObstaclePool.ReturnActor(Actor);
            ObstaclePoolGCRefs.AddUnique(Actor);
        }
        else if (Actor->IsA(ACoinPickup::StaticClass()))
        {
            CoinPool.ReturnActor(Actor);
            CoinPoolGCRefs.AddUnique(Actor);
        }
        else if (IsPlatformActor(Actor))
        {
            PlatformPool.ReturnActor(Actor);
            PlatformPoolGCRefs.AddUnique(Actor);
        }
        else
        {
            // Unknown actor type (e.g. WallSpike) — not pooled, just destroy
            UE_LOG(LogSideRunner, Verbose, TEXT("ReturnActorsToPool: Actor %s not poolable, destroying"), *Actor->GetName());
            Actor->Destroy();
        }
    }

#if UE_BUILD_DEVELOPMENT
    UE_LOG(LogSideRunner, Verbose, TEXT("ProceduralLevelBuilder: Returned %d actors to pools (Platform=%d, Obstacle=%d, Coin=%d)"),
           Actors.Num(), PlatformPool.GetPooledCount(), ObstaclePool.GetPooledCount(), CoinPool.GetPooledCount());
#endif
}

void UProceduralLevelBuilder::ClearPools()
{
    PlatformPool.Clear();
    ObstaclePool.Clear();
    CoinPool.Clear();

    PlatformPoolGCRefs.Empty();
    ObstaclePoolGCRefs.Empty();
    CoinPoolGCRefs.Empty();
}
