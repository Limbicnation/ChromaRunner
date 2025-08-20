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
 * Performance-optimized BaseLevel for procedurally generated side-scrolling levels.
 * Features efficient trigger detection and actor management for level streaming.
 */
UCLASS()
class SIDERUNNER_API ABaseLevel : public AActor
{
    GENERATED_BODY()
    
public:    
    ABaseLevel();
    
    /** Virtual destructor for proper cleanup */
    virtual ~ABaseLevel() override = default;

protected:
    virtual void BeginPlay() override;

public:    
    /** Called every frame (only when debug visualization is active) */
    virtual void Tick(float DeltaTime) override;

    // PERFORMANCE: Event system
    UPROPERTY(BlueprintAssignable, Category="Level Generation")
    FOnLevelTriggered OnLevelTriggered;
    
    // PERFORMANCE: Component access functions
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Level Generation")
    UBoxComponent* GetTrigger() const;
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Level Generation")
    UBoxComponent* GetSpawnLocation() const;
    
    // PERFORMANCE: Level management
    UFUNCTION(BlueprintCallable, Category="Level Generation")
    void ActivateLevel();
    
    UFUNCTION(BlueprintCallable, Category="Level Generation")
    void DeactivateLevel();
    
    // PERFORMANCE: Property accessors
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Level Generation")
    float GetLevelLength() const;
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Level Generation")
    int32 GetDifficultyLevel() const;
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Level Generation")
    bool IsEndLevel() const;
    
protected:
    // PERFORMANCE: Collision detection
    UFUNCTION()
    void OnTriggerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                         UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
                         bool bFromSweep, const FHitResult& SweepResult);
    
    // PERFORMANCE: Core components
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Level Generation", meta=(DisplayName="Trigger Box"))
    UBoxComponent* Trigger;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Level Generation", meta=(DisplayName="Spawn Location"))
    UBoxComponent* SpawnLocation;
    
    // PERFORMANCE: Level configuration
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Level Generation")
    TArray<AActor*> LevelActors;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Level Generation", meta=(ClampMin=100.0, ClampMax=10000.0))
    float LevelLength;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Level Generation", meta=(ClampMin=1, ClampMax=10))
    int32 DifficultyLevel;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Level Generation")
    bool bIsEndLevel;

private:
    // PERFORMANCE: Debug visualization (editor only)
    UPROPERTY(EditAnywhere, Category="Debug", meta=(DisplayName="Show Debug Boxes"))
    bool bShowDebugBoxes;
    
    // PERFORMANCE: Helper functions
    void ValidateLevelActors();
    
#if WITH_EDITOR
    void DrawDebugVisualization();
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};