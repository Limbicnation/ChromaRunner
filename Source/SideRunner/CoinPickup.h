#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CoinPickup.generated.h"

// Actor Pool template for object pooling
template<class T>
class FActorPool
{
public:
    // Get an actor from the pool
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
    
    // Return an actor to the pool
    void ReturnActor(T* Actor, FName Tag = NAME_None)
    {
        if (Actor)
        {
            TArray<T*>& Pool = PooledActors.FindOrAdd(Tag);
            Pool.Add(Actor);
            ActiveActors.Remove(Actor);
        }
    }
    
    // Clear the pool
    void Clear()
    {
        PooledActors.Empty();
        ActiveActors.Empty();
    }
    
private:
    // Actors currently in the pool, organized by tag
    TMap<FName, TArray<T*>> PooledActors;
    
    // Actors currently in use
    TArray<T*> ActiveActors;
};

UCLASS()
class SIDERUNNER_API ACoinPickup : public AActor
{
    GENERATED_BODY()
    
public:    
    // Sets default values for this actor's properties
    ACoinPickup();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;
    
    // Called when the actor is being removed from a level
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // Static mesh component for the coin visual
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* CoinMesh;
    
    // Collision component for detecting overlap with player
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USphereComponent* CollisionSphere;
    
    // Particle system component for collection effect
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UParticleSystemComponent* CollectParticles;
    
    // Magnet component for pulling coins to player
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USphereComponent* CoinMagnet;
    
    // Sound to play when collected
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundBase* CollectSound;
    
    // Value of this coin (how many coins to add to counter)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coin")
    int32 CoinValue;
    
    // Whether the coin has been collected
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Coin")
    bool bIsCollected;
    
    // Replicated version of bIsCollected
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Coin")
    bool bCollected;
    
    // Function called when player overlaps with coin
    UFUNCTION()
    void OnPlayerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                         UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
                         bool bFromSweep, const FHitResult& SweepResult);
    
    // Function called when player enters the magnet radius
    UFUNCTION()
    void OnMagnetOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                         UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
                         bool bFromSweep, const FHitResult& SweepResult);
    
    // Function to handle collection logic
    UFUNCTION(BlueprintNativeEvent, Category = "Coin")
    void Collect(class ACharacter* Character);
    virtual void Collect_Implementation(class ACharacter* Character);
    
    // Function to respawn the coin after collection
    UFUNCTION(BlueprintCallable, Category = "Coin")
    void Respawn();
    
    // Function to return the coin to the object pool
    UFUNCTION(BlueprintCallable, Category = "Coin|Pooling")
    void ReturnToPool();
    
    // Static map of coin pools by world
    static TMap<UWorld*, FActorPool<ACoinPickup>> CoinPools;

public:    
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    // Rotation speed of the coin
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coin|Animation")
    float RotationSpeed;
    
    // Hover amplitude (how far up and down the coin moves)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coin|Animation")
    float HoverAmplitude;
    
    // Hover frequency (how fast the coin moves up and down)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coin|Animation")
    float HoverFrequency;
    
    // Initial position for hovering effect
    FVector InitialLocation;
    
    // Current time for the hover animation
    float CurrentTime;
    
    // Whether to disable tick when far from player
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coin|Optimization")
    bool bDisableTickWhenFar;
    
    // Distance at which to disable tick
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coin|Optimization", meta = (EditCondition = "bDisableTickWhenFar"))
    float TickDistance;
    
    // Whether to enable coin magnetism
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coin|Magnetism")
    bool bEnableMagnetism;
    
    // Speed at which the coin moves towards the player when magnetized
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coin|Magnetism", meta = (EditCondition = "bEnableMagnetism"))
    float MagnetismSpeed;
    
    // Whether magnetism is currently active
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Coin|Magnetism")
    bool bMagnetActivated;
    
    // Target actor for magnetism
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Coin|Magnetism")
    AActor* TargetActor;
    
    // Whether the coin can respawn after collection
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coin|Respawn")
    bool bCanRespawn;
    
    // Time to wait before respawning
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coin|Respawn", meta = (EditCondition = "bCanRespawn"))
    float RespawnTime;
    
    // Whether to show debug info
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coin|Debug")
    bool bShowDebugInfo;
    
    // Whether to use object pooling for coins
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coin|Pooling")
    bool UseActorPooling;
    
    // Tag for pool management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coin|Pooling", meta = (EditCondition = "UseActorPooling"))
    FName PoolTag;
    
    // Delegate for coin collected event
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCoinCollectedSignature, ACoinPickup*, Coin, class ACharacter*, Character);
    
    // Event that fires when the coin is collected
    UPROPERTY(BlueprintAssignable, Category = "Coin|Events")
    FOnCoinCollectedSignature OnCoinCollected;
    
    // Delegate for coin respawned event
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCoinRespawnedSignature, ACoinPickup*, Coin);
    
    // Event that fires when the coin respawns
    UPROPERTY(BlueprintAssignable, Category = "Coin|Events")
    FOnCoinRespawnedSignature OnCoinRespawned;
    
    // Static function to spawn a coin from the pool
    UFUNCTION(BlueprintCallable, Category = "Coin|Pooling", meta = (WorldContext = "World"))
    static ACoinPickup* SpawnFromPool(UWorld* World, TSubclassOf<ACoinPickup> CoinClass, 
                                   const FTransform& Transform, FName Tag = NAME_None);
    
    // Static function to clear the pool
    UFUNCTION(BlueprintCallable, Category = "Coin|Pooling", meta = (WorldContext = "World"))
    static void ClearPool(UWorld* World);
};