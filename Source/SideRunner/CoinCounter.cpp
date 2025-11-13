#include "CoinCounter.h"
#include "Kismet/GameplayStatics.h"
#include "CoinPickup.h"
#include "Engine/World.h"
#include "EngineUtils.h"

// Sets default values for this component's properties
UCoinCounter::UCoinCounter()
{
    // Set this component to be initialized when the game starts, and to be ticked every frame
    PrimaryComponentTick.bCanEverTick = false; // Optimized: No need to tick every frame

    // Initialize coin count to zero
    CoinCount = 0;
    bProcessingCoin = false;

    // Default max coins value
    MaxCoins = 100;

    // Initialize persistent coin count for the level
    LevelPersistentCoins = 0;

    // Initialize total coins to collect in this level
    TotalCoinsInLevel = 0;
    
    // NEW: Initialize optimization variables
    bIsInitialized = false;
    LastUpdateTime = 0.0f;
    UpdateInterval = 0.1f;  // Batch UI updates every 0.1 seconds
}

// Called when the game starts
void UCoinCounter::BeginPlay()
{
    Super::BeginPlay();

    // CRITICAL FIX: Use thread-safe initialization
    if (bIsInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("CoinCounter already initialized, preventing duplicate initialization"));
        return;
    }

    // HOTFIX: Force reset at game start with thread safety
    {
        FScopeLock Lock(&CoinMutex);
        CoinCount = 0;
        CollectedCoins.Empty();
        bProcessingCoin = false;
        ReachedMilestones.Empty();
        bIsInitialized = true;
    }

    // Log initial state - helps with debugging
    UE_LOG(LogTemp, Log, TEXT("CoinCounter RESET to %d coins"), CoinCount);

    // Broadcast initial value to update UI (delayed to avoid race conditions)
    if (UWorld* World = GetWorld())
    {
        FTimerHandle InitTimer;
        TWeakObjectPtr<UCoinCounter> WeakThis(this);
        World->GetTimerManager().SetTimer(InitTimer, [WeakThis]()
        {
            UCoinCounter* CoinCounter = WeakThis.Get();
            if (CoinCounter)
            {
                int32 CurrentCount;
                {
                    FScopeLock Lock(&CoinCounter->CoinMutex);
                    CurrentCount = CoinCounter->CoinCount;
                }
                CoinCounter->OnCoinsUpdated.Broadcast(CurrentCount);
            }
        }, 0.1f, false);  // Delay broadcast by 0.1 seconds
    }

    // Count total coins in the level if auto-counting is enabled
    if (bAutoCountCoinsInLevel)
    {
        // Delay this operation to avoid race conditions during level loading
        if (UWorld* World = GetWorld())
        {
            FTimerHandle CountTimer;
            World->GetTimerManager().SetTimer(CountTimer, this, &UCoinCounter::CountCoinsInLevel, 0.5f, false);
        }
    }

    // Load persistent coins if enabled
    if (bPersistentCoins)
    {
        LoadPersistentCoins();
    }
}

// Called every frame - disabled for performance
void UCoinCounter::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

bool UCoinCounter::HasCollectedCoin(AActor* CoinActor) const
{
    if (!CoinActor)
    {
        return false;
    }
    
    // Thread-safe access
    FScopeLock Lock(&CoinMutex);
    return CollectedCoins.Contains(CoinActor);
}

void UCoinCounter::MarkCoinAsCollected(AActor* CoinActor)
{
    if (!CoinActor)
    {
        return;
    }
    
    // Thread-safe access
    FScopeLock Lock(&CoinMutex);
    
    // Prevent duplicate marking
    if (CollectedCoins.Contains(CoinActor))
    {
        UE_LOG(LogTemp, Warning, TEXT("Coin %s already marked as collected"), *CoinActor->GetName());
        return;
    }
    
    CollectedCoins.Add(CoinActor);
    UE_LOG(LogTemp, Verbose, TEXT("Marked coin %s as collected"), *CoinActor->GetName());
}

void UCoinCounter::AddCoins(int32 Amount)
{
    // CRITICAL FIX: Thread-safe coin addition with better validation
    if (!bIsInitialized)
    {
        UE_LOG(LogTemp, Error, TEXT("CoinCounter not initialized, cannot add coins"));
        return;
    }
    
    if (Amount <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid coin amount: %d"), Amount);
        return;
    }

    // Thread-safe processing flag check
    {
        FScopeLock Lock(&CoinMutex);
        if (bProcessingCoin)
        {
            UE_LOG(LogTemp, Warning, TEXT("Prevented duplicate coin add! Amount: %d"), Amount);
            return;
        }
        bProcessingCoin = true;
    }

    // Store previous values and update coin count atomically with overflow protection
    int32 PreviousCoinCount;
    int32 NewCoinCount;
    {
        FScopeLock Lock(&CoinMutex);
        PreviousCoinCount = CoinCount;
        CoinCount = FMath::Clamp(CoinCount + Amount, 0, INT32_MAX);  // Prevent overflow
        NewCoinCount = CoinCount;
    }

    // Debug log (outside critical section)
    UE_LOG(LogTemp, VeryVerbose, TEXT("Added %d coins. New total: %d"), Amount, NewCoinCount);

    // If we're using persistent coins, update and save them
    if (bPersistentCoins)
    {
        LevelPersistentCoins += Amount;
        SavePersistentCoins();
    }

    // OPTIMIZATION: Batch UI updates to prevent spam
    bool bShouldBroadcast = true;
    if (UWorld* World = GetWorld())
    {
        float CurrentTime = World->GetTimeSeconds();
        if (CurrentTime - LastUpdateTime < UpdateInterval)
        {
            bShouldBroadcast = false;

            // Schedule a delayed update instead
            FTimerHandle DelayedUpdateTimer;
            TWeakObjectPtr<UCoinCounter> WeakThis(this);
            float UpdateIntervalCopy = UpdateInterval;  // Capture by value to avoid accessing member after destruction
            World->GetTimerManager().SetTimer(DelayedUpdateTimer, [WeakThis]()
            {
                UCoinCounter* CoinCounter = WeakThis.Get();
                if (CoinCounter)
                {
                    // Read current count safely for delayed broadcast
                    int32 CurrentCount;
                    {
                        FScopeLock Lock(&CoinCounter->CoinMutex);
                        CurrentCount = CoinCounter->CoinCount;
                    }
                    CoinCounter->OnCoinsUpdated.Broadcast(CurrentCount);
                }
            }, UpdateIntervalCopy, false);
        }
        else
        {
            LastUpdateTime = CurrentTime;
        }
    }

    // Broadcast the event with the new coin count
    if (bShouldBroadcast)
    {
        OnCoinsUpdated.Broadcast(NewCoinCount);
    }

    // Check if we've collected all coins (using thread-safe method)
    bool bAllCoinsCollected = HasCollectedAllCoins();
    if (bAllCoinsCollected && PreviousCoinCount != NewCoinCount)  // Only trigger once
    {
        OnAllCoinsCollected.Broadcast();
    }

    // Check if we've reached a milestone
    for (int32 Milestone : CoinMilestones)
    {
        if (NewCoinCount >= Milestone && PreviousCoinCount < Milestone && !ReachedMilestones.Contains(Milestone))
        {
            ReachedMilestones.Add(Milestone);
            OnCoinMilestoneReached.Broadcast(Milestone);
        }
    }

    // Clear processing flag
    {
        FScopeLock Lock(&CoinMutex);
        bProcessingCoin = false;
    }
}

void UCoinCounter::ResetCoins()
{
    // Thread-safe reset
    {
        FScopeLock Lock(&CoinMutex);
        CoinCount = 0;
        CollectedCoins.Empty();
        ReachedMilestones.Empty();
        bProcessingCoin = false;
    }

    // Broadcast the event with the new coin count (using known value 0)
    OnCoinsUpdated.Broadcast(0);

    UE_LOG(LogTemp, Log, TEXT("CoinCounter reset to 0 coins"));
}

bool UCoinCounter::HasCollectedAllCoins() const
{
    // Thread-safe read
    int32 CurrentCoinCount;
    int32 TotalCoins;
    {
        FScopeLock Lock(&CoinMutex);
        CurrentCoinCount = CoinCount;
        TotalCoins = TotalCoinsInLevel;
    }

    if (bAutoCountCoinsInLevel)
    {
        return CurrentCoinCount >= TotalCoins && TotalCoins > 0;
    }

    return CurrentCoinCount >= MaxCoins;
}

float UCoinCounter::GetCompletionPercentage() const
{
    // Thread-safe read
    int32 CurrentCoinCount;
    int32 TotalCoins;
    {
        FScopeLock Lock(&CoinMutex);
        CurrentCoinCount = CoinCount;
        TotalCoins = TotalCoinsInLevel;
    }

    float MaxValue = bAutoCountCoinsInLevel ? FMath::Max(1, TotalCoins) : FMath::Max(1, MaxCoins);
    return FMath::Clamp((float)CurrentCoinCount / MaxValue * 100.0f, 0.0f, 100.0f);
}

void UCoinCounter::CountCoinsInLevel()
{
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot count coins - World is null"));
        return;
    }

    // PERFORMANCE: Use TActorIterator instead of GetAllActorsOfClass to avoid memory allocation
    int32 NewTotal = 0;
    for (TActorIterator<ACoinPickup> It(GetWorld()); It; ++It)
    {
        if (IsValid(*It))
        {
            NewTotal++;
        }
    }

    // Thread-safe write
    {
        FScopeLock Lock(&CoinMutex);
        TotalCoinsInLevel = NewTotal;
    }

    UE_LOG(LogTemp, Log, TEXT("Found %d coins in the level"), NewTotal);
}

int32 UCoinCounter::GetCurrentCoinCount() const
{
    FScopeLock Lock(&CoinMutex);
    return CoinCount;
}

int32 UCoinCounter::GetTotalCoinsInLevel() const
{
    FScopeLock Lock(&CoinMutex);
    return TotalCoinsInLevel;
}

void UCoinCounter::SavePersistentCoins()
{
    // This is a simple implementation that should be expanded
    // with your game's save system
    UE_LOG(LogTemp, Log, TEXT("Saving %d persistent coins"), LevelPersistentCoins);

    // Here you would save to your game's save system
    // Example pseudocode (replace with your save game implementation):
    /*
    UMySaveGame* SaveGameInstance = Cast<UMySaveGame>(UGameplayStatics::CreateSaveGameObject(UMySaveGame::StaticClass()));
    if (SaveGameInstance)
    {
        SaveGameInstance->PersistentCoins = LevelPersistentCoins;
        UGameplayStatics::SaveGameToSlot(SaveGameInstance, "CoinSave", 0);
    }
    */
}

void UCoinCounter::LoadPersistentCoins()
{
    // This is a simple implementation that should be expanded
    // with your game's save system
    UE_LOG(LogTemp, Log, TEXT("Loading persistent coins"));

    // Here you would load from your game's save system
    // Example pseudocode (replace with your save game implementation):
    /*
    UMySaveGame* SaveGameInstance = Cast<UMySaveGame>(UGameplayStatics::LoadGameFromSlot("CoinSave", 0));
    if (SaveGameInstance)
    {
        LevelPersistentCoins = SaveGameInstance->PersistentCoins;
        UE_LOG(LogTemp, Log, TEXT("Loaded %d persistent coins"), LevelPersistentCoins);
    }
    */
}

TArray<int32> UCoinCounter::GetReachedMilestones() const
{
    FScopeLock Lock(&CoinMutex);
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