#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseLevel.generated.h"

class UBoxComponent;

UCLASS()
class SIDERUNNER_API ABaseLevel : public AActor
{
    GENERATED_BODY()
    
public:    
    // Sets default values for this actor's properties
    ABaseLevel();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

public:    
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category="My Triggers")
    UBoxComponent* GetTrigger() const;

    UFUNCTION(BlueprintCallable, Category="My Triggers")
    UBoxComponent* GetSpawnLocation() const;

protected:
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="My Triggers")
    UBoxComponent* Trigger;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="My Triggers")
    UBoxComponent* SpawnLocation;
};
