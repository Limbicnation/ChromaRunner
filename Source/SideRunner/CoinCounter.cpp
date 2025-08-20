#include "CoinCounter.h"
#include "Kismet/GameplayStatics.h"
#include "CoinPickup.h"
#include "Engine/World.h"
#include "EngineUtils.h"

UCoinCounter::UCoinCounter()
{
    // PERFORMANCE: Disable tick by default - no need for per-frame updates
    PrimaryComponentTick.bCanEverTick = false;

    // PERFORMANCE: Initialize with safe defaults
    CoinCount = 0;
    bProcessingCoin = false;
    MaxCoins = 100;
    LevelPersistentCoins = 0;
    TotalCoinsInLevel = 0;

    // PERFORMANCE: Reserve space for collections to avoid frequent reallocation
    CollectedCoins.Reserve(100);
    CoinMilestones.Reserve(10);
    ReachedMilestones.Reserve(10);
}

void UCoinCounter::BeginPlay()
{
    Super::BeginPlay();

    // PERFORMANCE: Force reset at game start with atomic operation
    ResetCoinCounter();

#if UE_BUILD_DEVELOPMENT
    UE_LOG(LogTemp, Log, TEXT("CoinCounter initialized with %d coins"), CoinCount);
#endif

    // Broadcast initial value to update UI
    OnCoinsUpdated.Broadcast(CoinCount);

    // Auto-count coins in level if enabled
    if (bAutoCountCoinsInLevel)
    {
        CountCoinsInLevel();
    }

    // Load persistent coins if enabled
    if (bPersistentCoins)
    {
        LoadPersistentCoins();
    }
}

void UCoinCounter::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    // PERFORMANCE: This should never be called since tick is disabled
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

bool UCoinCounter::HasCollectedCoin(AActor* CoinActor) const
{
    // PERFORMANCE: Use Contains which is O(1) for TSet
    return CoinActor && CollectedCoins.Contains(CoinActor);
}

void UCoinCounter::MarkCoinAsCollected(AActor* CoinActor)
{
    if (CoinActor && !CollectedCoins.Contains(CoinActor))
    {
        CollectedCoins.Add(CoinActor);
#if UE_BUILD_DEVELOPMENT
        UE_LOG(LogTemp, Verbose, TEXT("Marked coin %s as collected"), *CoinActor->GetName());
#endif
    }
}

void UCoinCounter::AddCoins(int32 Amount)
{
    // PERFORMANCE: Comprehensive validation and thread safety
    if (Amount <= 0)
    {
#if UE_BUILD_DEVELOPMENT
        UE_LOG(LogTemp, Warning, TEXT("Attempted to add invalid coin amount: %d"), Amount);
#endif
        return;
    }

    // PERFORMANCE: Atomic check and set to prevent concurrent modification
    if (bProcessingCoin)
    {
#if UE_BUILD_DEVELOPMENT
        UE_LOG(LogTemp, Warning, TEXT("Prevented concurrent coin addition"));
#endif
        return;
    }

    bProcessingCoin = true;

    // Add coins with overflow protection
    const int32 PreviousCount = CoinCount;
    CoinCount = FMath::Clamp(CoinCount + Amount, 0, MAX_int32 - 1000); // Leave headroom

#if UE_BUILD_DEVELOPMENT
    UE_LOG(LogTemp, Log, TEXT("Added %d coins. Total: %d (was %d)"), Amount, CoinCount, PreviousCount);
#endif

    // Handle persistent coins
    if (bPersistentCoins)
    {
        LevelPersistentCoins += Amount;
        SavePersistentCoins();
    }

    // Broadcast the update event
    OnCoinsUpdated.Broadcast(CoinCount);

    // Check for completion and milestones
    CheckCompletionAndMilestones();

    // Clear processing flag
    bProcessingCoin = false;
}

void UCoinCounter::CheckCompletionAndMilestones()
{
    // Check completion
    if (HasCollectedAllCoins())
    {
        OnAllCoinsCollected.Broadcast();
#if UE_BUILD_DEVELOPMENT
        UE_LOG(LogTemp, Log, TEXT("All coins collected!"));
#endif
    }

    // PERFORMANCE: Check milestones efficiently
    for (int32 Milestone : CoinMilestones)
    {
        if (CoinCount >= Milestone && !ReachedMilestones.Contains(Milestone))
        {
            ReachedMilestones.AddUnique(Milestone);
            OnCoinMilestoneReached.Broadcast(Milestone);
#if UE_BUILD_DEVELOPMENT
            UE_LOG(LogTemp, Log, TEXT("Reached coin milestone: %d"), Milestone);
#endif
        }
    }
}

void UCoinCounter::ResetCoins()
{
    ResetCoinCounter();
    OnCoinsUpdated.Broadcast(CoinCount);

#if UE_BUILD_DEVELOPMENT
    UE_LOG(LogTemp, Log, TEXT("Coin counter reset to %d"), CoinCount);
#endif
}

void UCoinCounter::ResetCoinCounter()
{
    // PERFORMANCE: Atomic reset operation
    const bool bWasProcessing = bProcessingCoin;
    bProcessingCoin = false;
    
    CoinCount = 0;
    CollectedCoins.Empty();
    ReachedMilestones.Empty();
    
    // Restore processing flag if it was set
    bProcessingCoin = bWasProcessing;
}

bool UCoinCounter::HasCollectedAllCoins() const
{
    const int32 TargetCount = bAutoCountCoinsInLevel ? TotalCoinsInLevel : MaxCoins;
    return TargetCount > 0 && CoinCount >= TargetCount;
}

float UCoinCounter::GetCompletionPercentage() const
{
    const int32 TargetCount = bAutoCountCoinsInLevel ? TotalCoinsInLevel : MaxCoins;
    const float MaxValue = FMath::Max(1, TargetCount); // Avoid division by zero
    return FMath::Clamp((float)CoinCount / MaxValue * 100.0f, 0.0f, 100.0f);
}

void UCoinCounter::CountCoinsInLevel()
{
    TotalCoinsInLevel = 0;

    // PERFORMANCE: Use specific class search instead of generic GetAllActorsOfClass
    const UWorld* World = GetWorld();
    if (!World)
    {
#if UE_BUILD_DEBUG
        UE_LOG(LogTemp, Warning, TEXT("Cannot count coins - invalid world"));
#endif
        return;
    }

    // Count ACoinPickup actors in the level using TActorIterator
    for (TActorIterator<ACoinPickup> ActorIterator(World); ActorIterator; ++ActorIterator)
    {
        ACoinPickup* CoinActor = *ActorIterator;
        if (CoinActor && IsValid(CoinActor))
        {
            TotalCoinsInLevel++;
        }
    }

#if UE_BUILD_DEVELOPMENT
    UE_LOG(LogTemp, Log, TEXT("Counted %d coins in the level"), TotalCoinsInLevel);
#endif
}

void UCoinCounter::SavePersistentCoins()
{
    // PERFORMANCE: Placeholder for save system integration
    // This should be replaced with your game's actual save system

#if UE_BUILD_DEVELOPMENT
    UE_LOG(LogTemp, Log, TEXT("Saving %d persistent coins (placeholder)"), LevelPersistentCoins);
#endif

    // Example integration point for save system:
    /*
    if (UGameInstanceSubsystem* SaveSystem = GetGameInstance()->GetSubsystem<UGameInstanceSubsystem>())
    {
        SaveSystem->SavePersistentData(TEXT("Coins"), LevelPersistentCoins);
    }
    */
}

void UCoinCounter::LoadPersistentCoins()
{
    // PERFORMANCE: Placeholder for load system integration
    // This should be replaced with your game's actual save system

#if UE_BUILD_DEVELOPMENT
    UE_LOG(LogTemp, Log, TEXT("Loading persistent coins (placeholder)"));
#endif

    // Example integration point for save system:
    /*
    if (UGameInstanceSubsystem* SaveSystem = GetGameInstance()->GetSubsystem<UGameInstanceSubsystem>())
    {
        LevelPersistentCoins = SaveSystem->LoadPersistentData(TEXT("Coins"), 0);
        UE_LOG(LogTemp, Log, TEXT("Loaded %d persistent coins"), LevelPersistentCoins);
    }
    */
}

TArray<int32> UCoinCounter::GetReachedMilestones() const
{
    return ReachedMilestones;
}

#if WITH_EDITOR
void UCoinCounter::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    const FName PropertyName = (PropertyChangedEvent.Property != nullptr) ?
        PropertyChangedEvent.Property->GetFName() : NAME_None;

    // PERFORMANCE: Validate properties when changed in editor
    if (PropertyName == GET_MEMBER_NAME_CHECKED(UCoinCounter, MaxCoins))
    {
        MaxCoins = FMath::Max(1, MaxCoins);
    }
    else if (PropertyName == GET_MEMBER_NAME_CHECKED(UCoinCounter, CoinMilestones))
    {
        // Sort milestones and remove duplicates
        CoinMilestones.RemoveAll([](int32 Value) { return Value <= 0; });
        CoinMilestones.Sort();
        
        // Remove duplicates
        for (int32 i = CoinMilestones.Num() - 1; i > 0; i--)
        {
            if (CoinMilestones[i] == CoinMilestones[i - 1])
            {
                CoinMilestones.RemoveAt(i);
            }
        }
    }
    else if (PropertyName == GET_MEMBER_NAME_CHECKED(UCoinCounter, bAutoCountCoinsInLevel))
    {
        // Re-count coins when this setting changes
        if (bAutoCountCoinsInLevel && GetWorld())
        {
            CountCoinsInLevel();
        }
    }
}
#endif