#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PlayerHealthComponent.h"
#include "RunnerCharacter.generated.h"

// Forward declarations for better compilation performance
class AWallSpike;
class ASpikes;

// Define character animation states
UENUM(BlueprintType)
enum class ECharacterState : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Running UMETA(DisplayName = "Running"),
    Jumping UMETA(DisplayName = "Jumping"),
    Falling UMETA(DisplayName = "Falling"),
    DoubleJumping UMETA(DisplayName = "DoubleJumping"),
    Dead UMETA(DisplayName = "Dead")
};

/**
 * Performance-optimized runner character with state management and health system.
 * Features double-jump mechanics and efficient animation state handling.
 */
UCLASS()
class SIDERUNNER_API ARunnerCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    ARunnerCharacter();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // PERFORMANCE: Animation State System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    ECharacterState CurrentState;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation")
    ECharacterState PreviousState;
    
    // State management functions
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetCharacterState(ECharacterState NewState);
    
    // Blueprint events for state changes
    UFUNCTION(BlueprintImplementableEvent, Category = "Animation")
    void OnCharacterStateChanged(ECharacterState NewCharacterState, ECharacterState OldCharacterState);
    
    UFUNCTION(BlueprintImplementableEvent, Category = "Animation")
    void UpdateCharacterSprite();
    
    // Reference to visual component - configurable in Blueprint
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UMeshComponent* CharacterVisual;
    
    // PERFORMANCE: State query functions
    UFUNCTION(BlueprintPure, Category = "Animation")
    ECharacterState GetCharacterState() const { return CurrentState; }
    
    UFUNCTION(BlueprintPure, Category = "Animation")
    bool IsInState(ECharacterState StateToCheck) const { return CurrentState == StateToCheck; }
    
    UFUNCTION(BlueprintPure, Category = "Animation")
    float GetTimeInCurrentState() const { return StateTimer; }

    // PERFORMANCE: Jump Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jump")
    bool bCanDoubleJump;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jump", meta = (ClampMin = "200.0", ClampMax = "2000.0"))
    float DoubleJumpZVelocity;

    // Camera component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    class UCameraComponent* SideViewCamera;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (ClampMin = "90.0", ClampMax = "360.0"))
    float RotationRate = 180.0f;
    
    // PERFORMANCE: Health System
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health")
    UPlayerHealthComponent* HealthComponent;
    
    // Blueprint events for health system
    UFUNCTION(BlueprintImplementableEvent, Category = "Health")
    void OnHealthChanged(int32 CurrentHealth, int32 MaxHealth);
    
    UFUNCTION(BlueprintImplementableEvent, Category = "Health")
    void OnTakeDamage(int32 DamageAmount, EDamageType DamageType);
    
    // PERFORMANCE: Damage and death handling
    UFUNCTION(BlueprintCallable, Category = "Health")
    void ProcessDamage(float DamageAmount, AActor* DamageCauser);
    
    UFUNCTION()
    void HandlePlayerDeath(int32 TotalHitsTaken);
    
    UFUNCTION(BlueprintPure, Category = "Health")
    bool IsDead() const;

protected:
    // PERFORMANCE: Input handling
    virtual void Jump() override;
    void MoveRight(float Value);

    // Death handling
    UFUNCTION(BlueprintImplementableEvent, Category = "Health")
    void DeathOfPlayer();

public:
    // Level management
    UFUNCTION(BlueprintCallable, Category = "Game")
    void RestartLevel();

    // Collision handling
    UFUNCTION()
    void OnOverlapBegin(
        UPrimitiveComponent* OverlappedComp,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult
    );
    
    // Override TakeDamage for health component integration
    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, 
        class AController* EventInstigator, AActor* DamageCauser) override;

private:
    // PERFORMANCE: Cached positioning variables
    float zPosition;
    FVector TempPos;

    // PERFORMANCE: Cached game instance reference (avoid 60 casts/second in Tick)
    UPROPERTY()
    class USideRunnerGameInstance* CachedGameInstance;

    // PERFORMANCE: Movement state
    bool CanMove;
    bool CanJump;
    // Note: Double jump uses public bCanDoubleJump UPROPERTY instead
    
    // PERFORMANCE: State timing
    float StateTimer;
    
    // PERFORMANCE: Helper functions for cleaner code
    void UpdateCameraPosition();
    void HandleEnvironmentalDeath();
    void UpdateAnimationState();
    ECharacterState DetermineAirborneState() const;
    bool IsMovingHorizontally() const;
    void HandleWallSpikeOverlap(AWallSpike* WallSpike);
    void HandleRegularSpikeOverlap(ASpikes* RegularSpike);
};
