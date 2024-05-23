#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Sound/SoundBase.h"
#include "Spikes.generated.h"

UCLASS()
class SIDERUNNER_API ASpikes : public AActor
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    ASpikes();

    // Called every frame
    virtual void Tick(float DeltaTime) override;

    // Speed of spike movement, adjustable from Blueprints
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float Speed;

    // Maximum height offset the spikes should move up to from their initial position
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float MaxHeightOffset;

    // Sound to play when the player collides with the spikes
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    USoundBase* CollisionSound;

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    // Function to handle collision with the spikes
    UFUNCTION()
    void NotifyHit(
        UPrimitiveComponent* MyComp,
        AActor* Other,
        UPrimitiveComponent* OtherComp,
        bool bSelfMoved,
        const FHitResult& Hit
    );

private:
    // Store the initial Z position of the spikes
    float InitialZ; 

    // 1 means up, -1 means down
    int32 MovementDirection; 
};
