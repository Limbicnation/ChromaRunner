#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HAL/CriticalSection.h"
#include "CoinCounter.generated.h"

// Delegate declarations for coin events
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCoinsUpdatedDelegate, int32, NewCoinCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllCoinsCollectedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCoinMilestoneReachedDelegate, int32, Milestone);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SIDERUNNER_API UCoinCounter : public UActorComponent
{
    GENERATED_BODY()

public:	
    // Sets default values for this component's properties
    UCoinCounter();

    // Called every frame - disabled for performance optimization
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Add coins to the counter
    UFUNCTION(BlueprintCallable, Category = "Coins")
    void AddCoins(int32 Amount);
    
    // Get current coin count (thread-safe)
    UFUNCTION(BlueprintPure, Category = "Coins")
    int32 GetCurrentCoinCount() const;
    
    // Reset coins to zero
    UFUNCTION(BlueprintCallable, Category = "Coins")
    void ResetCoins();
    
    // Check if a specific coin has been collected
    UFUNCTION(BlueprintPure, Category = "Coins")
    bool HasCollectedCoin(AActor* CoinActor) const;
    
    // Mark a specific coin as collected
    UFUNCTION(BlueprintCallable, Category = "Coins")
    void MarkCoinAsCollected(AActor* CoinActor);
    
    // Check if all coins have been collected
    UFUNCTION(BlueprintPure, Category = "Coins")
    bool HasCollectedAllCoins() const;
    
    // Get completion percentage (0-100%)
    UFUNCTION(BlueprintPure, Category = "Coins")
    float GetCompletionPercentage() const;
    
    // Get total coins available in this level
    UFUNCTION(BlueprintPure, Category = "Coins")
    int32 GetTotalCoinsInLevel() const;
    
    // Get milestones that have been reached
    UFUNCTION(BlueprintPure, Category = "Coins")
    TArray<int32> GetReachedMilestones() const;

protected:
    // Called when the game starts
    virtual void BeginPlay() override;

    // Current coin count
    UPROPERTY(BlueprintReadOnly, Category = "Coins")
    int32 CoinCount;
    
    // Maximum coins that can be collected (used when not auto-counting)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coins")
    int32 MaxCoins;
    
    // Whether to automatically count coins in the level
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coins")
    bool bAutoCountCoinsInLevel = true;
    
    // Whether coins should persist between level reloads
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coins")
    bool bPersistentCoins = false;
    
    // Milestones that trigger events when reached
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coins")
    TArray<int32> CoinMilestones;

public:
    // Delegate for when coins are updated
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnCoinsUpdatedDelegate OnCoinsUpdated;
    
    // Delegate for when all coins are collected
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnAllCoinsCollectedDelegate OnAllCoinsCollected;
    
    // Delegate for when a coin milestone is reached
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnCoinMilestoneReachedDelegate OnCoinMilestoneReached;

private:
    // Set of collected coin actors to prevent double collection
    UPROPERTY()
    TSet<AActor*> CollectedCoins;
    
    // Milestones that have been reached
    UPROPERTY()
    TArray<int32> ReachedMilestones;
    
    // Flag to prevent multiple coin additions at once
    bool bProcessingCoin;
    
    // Total coins found in the level
    int32 TotalCoinsInLevel;
    
    // Persistent coins across level reloads
    int32 LevelPersistentCoins;
    
    // NEW: Thread safety and optimization variables
    mutable FCriticalSection CoinMutex;
    bool bIsInitialized;
    float LastUpdateTime;
    float UpdateInterval;
    
    // Function to count coins in the level
    UFUNCTION()
    void CountCoinsInLevel();
    
    // Save/Load persistent coins (integrate with your save system)
    void SavePersistentCoins();
    void LoadPersistentCoins();
};