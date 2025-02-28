#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseLevel.generated.h"

class UBoxComponent;

/**
 * Delegate for level trigger events
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLevelTriggered, class ABaseLevel*, TriggeredLevel);

/**
 * BaseLevel is a fundamental building block for procedurally generated side-scrolling levels.
 * It contains a trigger for level loading/unloading and a spawn point for the next level segment.
 */
UCLASS()
class SIDERUNNER_API ABaseLevel : public AActor
{
    GENERATED_BODY()
    
public:    
    /** Sets default values for this actor's properties */
    ABaseLevel();
    
    /** Virtual destructor for proper cleanup */
    virtual ~ABaseLevel() override = default;

protected:
    /** Called when the game starts or when spawned */
    virtual void BeginPlay() override;

public:    
    /** Called every frame (only enabled when debug visualization is active) */
    virtual void Tick(float DeltaTime) override;

    /** Event triggered when a player enters this level segment */
    UPROPERTY(BlueprintAssignable, Category="Level Generation")
    FOnLevelTriggered OnLevelTriggered;
    
    /** Gets the trigger box component that detects when player enters this level segment */
    UFUNCTION(BlueprintCallable, Category="Level Generation")
    UBoxComponent* GetTrigger() const;
    
    /** Gets the spawn location box for new level segments */
    UFUNCTION(BlueprintCallable, Category="Level Generation")
    UBoxComponent* GetSpawnLocation() const;
    
    /** Activates all actors in this level segment */
    UFUNCTION(BlueprintCallable, Category="Level Generation")
    void ActivateLevel();
    
    /** Deactivates all actors in this level segment for performance */
    UFUNCTION(BlueprintCallable, Category="Level Generation")
    void DeactivateLevel();
    
protected:
    /** Callback for trigger box overlap events */
    UFUNCTION()
    void OnTriggerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                         UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
                         bool bFromSweep, const FHitResult& SweepResult);
    
    /** Trigger box that detects when player enters this level segment */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Level Generation", meta=(DisplayName="Trigger Box"))
    UBoxComponent* Trigger;
    
    /** Box that defines where the next level segment should be spawned */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Level Generation", meta=(DisplayName="Spawn Location"))
    UBoxComponent* SpawnLocation;
    
    /** List of actors that belong to this level segment (for activation/deactivation) */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Level Generation")
    TArray<AActor*> LevelActors;
    
    /** The length of this level segment, used for positioning the next segment */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Level Generation", meta=(UIMin=100.0, UIMax=10000.0))
    float LevelLength;
    
    /** Difficulty rating of this level segment (1-10) */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Level Generation", meta=(ClampMin=1, ClampMax=10))
    int32 DifficultyLevel;
    
    /** Whether this is an end-level segment (e.g., boss fight, checkpoint) */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Level Generation")
    bool bIsEndLevel;
    
    /** Whether to display debug visualization boxes */
    UPROPERTY(EditAnywhere, Category="Debug")
    bool bShowDebugBoxes;
};