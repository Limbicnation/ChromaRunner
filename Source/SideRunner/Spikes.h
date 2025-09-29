#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Sound/SoundBase.h"
#include "Spikes.generated.h"

UENUM(BlueprintType)
enum class EMovementType : uint8
{
    UpDown UMETA(DisplayName = "Up/Down"),
    LeftRight UMETA(DisplayName = "Left/Right"),
    FrontBack UMETA(DisplayName = "Front/Back"),
    Static UMETA(DisplayName = "No Movement"),
    Circular UMETA(DisplayName = "Circular"),
    Zigzag UMETA(DisplayName = "Zigzag")
};

UCLASS()
class SIDERUNNER_API ASpikes : public AActor
{
    GENERATED_BODY()
    
public:
    // Sets default values for this actor's properties
    ASpikes();
    
    // Called every frame
    virtual void Tick(float DeltaTime) override;
    
    // Called when collision happens - must match AActor's signature exactly
    virtual void NotifyHit(
        UPrimitiveComponent* MyComp,
        AActor* Other,
        UPrimitiveComponent* OtherComp,
        bool bSelfMoved,
        FVector HitLocation,
        FVector HitNormal,
        FVector NormalImpulse,
        const FHitResult& Hit
    ) override;
    
    // Enable or disable movement
    UFUNCTION(BlueprintCallable, Category = "Spikes")
    void SetMovementEnabled(bool bEnabled);
    
    
#if WITH_EDITOR
    // Draw movement path for easier editing
    void DrawDebugMovementPath();
    
    // Handle property changes in editor
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

public:
    // Components
    
    // Box collision component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UBoxComponent* CollisionBox;
    
    // Static mesh component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* SpikeMesh;
    
    // Particle effect for impact
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UParticleSystemComponent* ImpactEffect;
    
    // NEW: Audio component for better sound management
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* AudioComponent;
    
    // Movement Properties
    
    // Speed of spike movement, adjustable from Blueprints
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float Speed;
    
    // Maximum offset the spikes should move from their initial position
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float MaxMovementOffset;
    
    // Type of movement pattern
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    EMovementType MovementType;
    
    // Whether spikes are currently moving
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    bool bIsMoving;
    
    // Gameplay Properties
    
    // Amount of damage to apply to the player (used by RunnerCharacter for damage calculation)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    float DamageAmount;
    
    // Whether spikes are triggered by proximity
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
    bool bProximityTriggered;
    
    // Radius for proximity triggering
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger", meta = (EditCondition = "bProximityTriggered"))
    float TriggerRadius;
    
    // FX Properties
    
    // Sound to play when the player collides with the spikes
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    USoundBase* CollisionSound;
    
protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;
    
    // NEW: Optimized overlap detection
    UFUNCTION()
    void OnSpikeOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
    // NEW: Optimized player proximity checking
    void CheckPlayerProximity();
    
    // NEW: Centralized movement update
    void UpdateMovement();
    
    // NEW: Better audio management
    void PlayCollisionSound();
    
    // Store the initial position of the spikes
    FVector InitialPosition;
    
    // 1 means positive direction, -1 means negative direction
    int32 MovementDirection;
    
    // Whether spikes are currently triggered
    bool bIsTriggered;
    
    // Current time for smooth oscillation-based movements
    float CurrentTime;
    
    // NEW: Performance optimization variables
    float LastPlayerCheckTime;
    float PlayerCheckInterval;
};