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

/**
 * Performance-optimized spike actor with configurable movement patterns and damage system.
 * Features proximity triggering and editor visualization tools.
 */
UCLASS()
class SIDERUNNER_API ASpikes : public AActor
{
    GENERATED_BODY()

public:
    ASpikes();

    virtual void Tick(float DeltaTime) override;

    // Collision detection - optimized for performance
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

    // PERFORMANCE: Movement control
    UFUNCTION(BlueprintCallable, Category = "Spikes")
    void SetMovementEnabled(bool bEnabled);

#if WITH_EDITOR
    // Editor-only debug visualization
    void DrawDebugMovementPath();
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

protected:
    virtual void BeginPlay() override;

public:
    // PERFORMANCE: Component setup
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UBoxComponent* CollisionBox;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* SpikeMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UParticleSystemComponent* ImpactEffect;

    // PERFORMANCE: Movement Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "0.0", ClampMax = "1000.0"))
    float Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "0.0", ClampMax = "2000.0"))
    float MaxMovementOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    EMovementType MovementType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    bool bIsMoving;

    // PERFORMANCE: Gameplay Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay", meta = (ClampMin = "1.0", ClampMax = "1000.0"))
    float DamageAmount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float DamageCooldown;

    // PERFORMANCE: Proximity trigger system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
    bool bProximityTriggered;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger", meta = (EditCondition = "bProximityTriggered", ClampMin = "50.0", ClampMax = "1000.0"))
    float TriggerRadius;

    // PERFORMANCE: Audio/Effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    USoundBase* CollisionSound;

private:
    // PERFORMANCE: Cached state variables
    FVector InitialPosition;
    int32 MovementDirection;
    float DamageTimer;
    bool bIsTriggered;
    float CurrentTime;

    // PERFORMANCE: Optimized helper functions
    void CalculateMovementLocation(FVector& OutLocation, float DeltaTime);
    void CalculateZigzagMovement(FVector& OutLocation, float SpeedFactor);
    void ApplyDamageToPlayer(AActor* Player);
    void HandleCollisionEffects(const FVector& HitLocation);

#if WITH_EDITOR
    // PERFORMANCE: Editor-only debug drawing methods
    void DrawVerticalMovementPath(UWorld* World, const FVector& BasePosition, const FColor& Color, float Thickness, float LifeTime);
    void DrawHorizontalMovementPath(UWorld* World, const FVector& BasePosition, const FColor& Color, float Thickness, float LifeTime);
    void DrawDepthMovementPath(UWorld* World, const FVector& BasePosition, const FColor& Color, float Thickness, float LifeTime);
    void DrawCircularMovementPath(UWorld* World, const FVector& BasePosition, const FColor& Color, float Thickness, float LifeTime);
    void DrawZigzagMovementPath(UWorld* World, const FVector& BasePosition, const FColor& Color, float Thickness, float LifeTime);
#endif
};