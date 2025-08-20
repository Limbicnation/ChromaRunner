#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CoinPickup.generated.h"

// PERFORMANCE: Simple actor pool template for efficient object reuse
template<class T>
class FActorPool
{
public:
    T* GetActor(FName Tag = NAME_None)
    {
        TArray<T*>& Pool = PooledActors.FindOrAdd(Tag);
        if (Pool.Num() > 0)
        {
            T* Actor = Pool.Pop();
            ActiveActors.Add(Actor);
            return Actor;
        }
        return nullptr;
    }
    
    void ReturnActor(T* Actor, FName Tag = NAME_None)
    {
        if (Actor)
        {
            TArray<T*>& Pool = PooledActors.FindOrAdd(Tag);
            Pool.Add(Actor);
            ActiveActors.Remove(Actor);
        }
    }
    
    void Clear()
    {
        PooledActors.Empty();
        ActiveActors.Empty();
    }
    
private:
    TMap<FName, TArray<T*>> PooledActors;
    TArray<T*> ActiveActors;
};

/**
 * Performance-optimized coin pickup with magnetism, animation, and pooling support.
 * Features distance-based tick optimization and efficient collection handling.
 */
UCLASS()
class SIDERUNNER_API ACoinPickup : public AActor
{
    GENERATED_BODY()
    
public:    
    ACoinPickup();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:    
    virtual void Tick(float DeltaTime) override;

    // PERFORMANCE: Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* CoinMesh;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USphereComponent* CollisionSphere;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UParticleSystemComponent* CollectParticles;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USphereComponent* CoinMagnet;
    
    // PERFORMANCE: Audio and Value
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coin")
    class USoundBase* CollectSound;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coin", meta = (ClampMin = "1", ClampMax = "100"))
    int32 CoinValue;
    
    // PERFORMANCE: State Management
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Coin")
    bool bIsCollected;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Coin")
    bool bCollected;
    
    // PERFORMANCE: Animation Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation", meta = (ClampMin = "0.0", ClampMax = "500.0"))
    float RotationSpeed;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation", meta = (ClampMin = "0.0", ClampMax = "50.0"))
    float HoverAmplitude;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float HoverFrequency;
    
    // PERFORMANCE: Optimization Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bDisableTickWhenFar;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization", meta = (EditCondition = "bDisableTickWhenFar", ClampMin = "500.0", ClampMax = "5000.0"))
    float TickDistance;
    
    // PERFORMANCE: Magnetism System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magnetism")
    bool bEnableMagnetism;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magnetism", meta = (EditCondition = "bEnableMagnetism", ClampMin = "100.0", ClampMax = "1000.0"))
    float MagnetismSpeed;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Magnetism")
    bool bMagnetActivated;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Magnetism")
    AActor* TargetActor;
    
    // PERFORMANCE: Respawn System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Respawn")
    bool bCanRespawn;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Respawn", meta = (EditCondition = "bCanRespawn", ClampMin = "1.0", ClampMax = "60.0"))
    float RespawnTime;
    
    // PERFORMANCE: Object Pooling
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pooling")
    bool UseActorPooling;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pooling", meta = (EditCondition = "UseActorPooling"))
    FName PoolTag;
    
    // Debug visualization
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowDebugInfo;

    // PERFORMANCE: Collection Events
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCoinCollectedSignature, ACoinPickup*, Coin, class ACharacter*, Character);
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnCoinCollectedSignature OnCoinCollected;
    
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCoinRespawnedSignature, ACoinPickup*, Coin);
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnCoinRespawnedSignature OnCoinRespawned;

    // PERFORMANCE: Core Functionality
    UFUNCTION(BlueprintNativeEvent, Category = "Coin")
    void Collect(class ACharacter* Character);
    virtual void Collect_Implementation(class ACharacter* Character);
    
    UFUNCTION(BlueprintCallable, Category = "Coin")
    void Respawn();
    
    UFUNCTION(BlueprintCallable, Category = "Pooling")
    void ReturnToPool();
    
    // PERFORMANCE: Static pooling functions
    UFUNCTION(BlueprintCallable, Category = "Pooling", meta = (WorldContext = "World"))
    static ACoinPickup* SpawnFromPool(UWorld* World, TSubclassOf<ACoinPickup> CoinClass, 
                                   const FTransform& Transform, FName Tag = NAME_None);
    
    UFUNCTION(BlueprintCallable, Category = "Pooling", meta = (WorldContext = "World"))
    static void ClearPool(UWorld* World);

protected:
    // Collision event handlers
    UFUNCTION()
    void OnPlayerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                         UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
                         bool bFromSweep, const FHitResult& SweepResult);
    
    UFUNCTION()
    void OnMagnetOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                         UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
                         bool bFromSweep, const FHitResult& SweepResult);

private:
    // PERFORMANCE: Internal state management
    FVector InitialLocation;
    float CurrentTime;
    
    // Static pool management
    static TMap<UWorld*, FActorPool<ACoinPickup>> CoinPools;

    // PERFORMANCE: Helper functions for cleaner code
    bool ShouldTickBasedOnDistance() const;
    void UpdateMagnetMovement(float DeltaTime);
    void UpdateCoinAnimation(float DeltaTime);
    void ResetCoinState();
    void HandleCollectionEffects();
    void UpdateCoinCounter(ACharacter* Character);
    void HandlePostCollection();

#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
    void DrawDebugInformation() const;
#endif
};