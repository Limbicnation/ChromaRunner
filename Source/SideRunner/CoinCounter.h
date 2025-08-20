#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CoinCounter.generated.h"

// Forward declare to avoid circular include
class ACoinPickup;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SIDERUNNER_API UCoinCounter : public UActorComponent
{
    GENERATED_BODY()
public:    
    // Sets default values for this component's properties
    UCoinCounter();
    
    // Track which coins have been collected to prevent double-counting
    UPROPERTY()
    TSet<AActor*> CollectedCoins;
    
    // Track if a coin is currently being processed (prevents recursive additions)
    UPROPERTY()
    bool bProcessingCoin;
    
    // Method to check if a coin has already been counted
    UFUNCTION(BlueprintCallable, Category = "Coins")
    bool HasCollectedCoin(AActor* CoinActor) const;
    
    // Method to mark a coin as collected
    UFUNCTION(BlueprintCallable, Category = "Coins")
    void MarkCoinAsCollected(AActor* CoinActor);
    
protected:
    // Called when the game starts
    virtual void BeginPlay() override;
public:    
    // Called every frame
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    
    // Current coin count
    UPROPERTY(BlueprintReadOnly, Category = "Coins")
    int32 CoinCount;
    
    // Maximum number of coins that can be collected (optional)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coins", meta = (EditCondition = "!bAutoCountCoinsInLevel"))
    int32 MaxCoins;
    
    // Should we automatically count coins in the level at BeginPlay?
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coins")
    bool bAutoCountCoinsInLevel = true;
    
    // Total number of coins in the level (filled automatically if bAutoCountCoinsInLevel is true)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Coins")
    int32 TotalCoinsInLevel;
    
    // Should we persist coins between level loads or game sessions?
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coins|Persistence")
    bool bPersistentCoins = false;
    
    // Persistent coin count for this level
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Coins|Persistence")
    int32 LevelPersistentCoins;
    
    // Milestones for coin collection (e.g., 10, 25, 50, 100)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coins|Milestones")
    TArray<int32> CoinMilestones;
    
    // Milestones that have been reached
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Coins|Milestones")
    TArray<int32> ReachedMilestones;
    
    // Function to add coins to the counter
    UFUNCTION(BlueprintCallable, Category = "Coins")
    void AddCoins(int32 Amount = 1);
    
    // Function to reset the coin counter
    UFUNCTION(BlueprintCallable, Category = "Coins")
    void ResetCoins();
    
    // Function to check if all coins have been collected
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Coins")
    bool HasCollectedAllCoins() const;
    
    // Function to get the percentage of coins collected
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Coins")
    float GetCompletionPercentage() const;
    
    // Function to count total coins in the level
    UFUNCTION(BlueprintCallable, Category = "Coins")
    void CountCoinsInLevel();
    
    // Function to save persistent coins
    UFUNCTION(BlueprintCallable, Category = "Coins|Persistence")
    void SavePersistentCoins();
    
    // Function to load persistent coins
    UFUNCTION(BlueprintCallable, Category = "Coins|Persistence")
    void LoadPersistentCoins();
    
    // Function to get the milestones that have been reached
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Coins|Milestones")
    TArray<int32> GetReachedMilestones() const;
    
    // Delegate that is broadcast when coins are added
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCoinsUpdated, int32, NewCoinCount);
    
    // Event that fires when coins are added
    UPROPERTY(BlueprintAssignable, Category = "Coins|Events")
    FOnCoinsUpdated OnCoinsUpdated;
    
    // Delegate that is broadcast when all coins are collected
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllCoinsCollected);
    
    // Event that fires when all coins are collected
    UPROPERTY(BlueprintAssignable, Category = "Coins|Events")
    FOnAllCoinsCollected OnAllCoinsCollected;
    
    // Delegate that is broadcast when a coin milestone is reached
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCoinMilestoneReached, int32, Milestone);
    
    // Event that fires when a coin milestone is reached
    UPROPERTY(BlueprintAssignable, Category = "Coins|Events")
    FOnCoinMilestoneReached OnCoinMilestoneReached;

private:
    // PERFORMANCE: Helper functions for cleaner code
    void ResetCoinCounter();
    void CheckCompletionAndMilestones();

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};