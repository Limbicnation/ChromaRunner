#include "CoinCounter.h"
#include "Kismet/GameplayStatics.h"
#include "CoinPickup.h"

// Sets default values for this component's properties
UCoinCounter::UCoinCounter()
{
    // Set this component to be initialized when the game starts, and to be ticked every frame
    PrimaryComponentTick.bCanEverTick = false; // Optimized: No need to tick every frame

    // Initialize coin count to zero
    CoinCount = 0;

    // Default max coins value
    MaxCoins = 100;

    // Initialize persistent coin count for the level
    LevelPersistentCoins = 0;

    // Initialize total coins to collect in this level
    TotalCoinsInLevel = 0;
}

// Called when the game starts
void UCoinCounter::BeginPlay()
{
    Super::BeginPlay();

    // Count total coins in the level if auto-counting is enabled
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

// Called every frame - disabled for performance
void UCoinCounter::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UCoinCounter::AddCoins(int32 Amount)
{
    if (Amount <= 0)
    {
        return;
    }

    CoinCount += Amount;

    // If we're using persistent coins, update and save them
    if (bPersistentCoins)
    {
        LevelPersistentCoins += Amount;
        SavePersistentCoins();
    }

    // Broadcast the event with the new coin count
    OnCoinsUpdated.Broadcast(CoinCount);

    // Check if we've collected all coins
    bool bAllCoinsCollected = HasCollectedAllCoins();
    if (bAllCoinsCollected)
    {
        OnAllCoinsCollected.Broadcast();
    }

    // Check if we've reached a milestone
    for (int32 Milestone : CoinMilestones)
    {
        if (CoinCount >= Milestone && !ReachedMilestones.Contains(Milestone))
        {
            ReachedMilestones.Add(Milestone);
            OnCoinMilestoneReached.Broadcast(Milestone);
        }
    }
}

void UCoinCounter::ResetCoins()
{
    CoinCount = 0;

    // Broadcast the event with the new coin count
    OnCoinsUpdated.Broadcast(CoinCount);

    // Clear reached milestones
    ReachedMilestones.Empty();
}

bool UCoinCounter::HasCollectedAllCoins() const
{
    if (bAutoCountCoinsInLevel)
    {
        return CoinCount >= TotalCoinsInLevel && TotalCoinsInLevel > 0;
    }

    return CoinCount >= MaxCoins;
}

float UCoinCounter::GetCompletionPercentage() const
{
    float MaxValue = bAutoCountCoinsInLevel ? FMath::Max(1, TotalCoinsInLevel) : FMath::Max(1, MaxCoins);
    return (float)CoinCount / MaxValue * 100.0f;
}

void UCoinCounter::CountCoinsInLevel()
{
    TotalCoinsInLevel = 0;
    TArray<AActor*> FoundCoins;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACoinPickup::StaticClass(), FoundCoins);
    TotalCoinsInLevel = FoundCoins.Num();

    UE_LOG(LogTemp, Log, TEXT("Found %d coins in the level"), TotalCoinsInLevel);
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
    return ReachedMilestones;
}