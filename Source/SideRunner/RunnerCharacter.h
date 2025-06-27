#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PlayerHealthComponent.h"
#include "RunnerCharacter.generated.h"

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

    // Animation State System
    
    // Current animation state of the character
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    ECharacterState CurrentState;
    
    // Last state before the current one (for transitions)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation")
    ECharacterState PreviousState;
    
    // Function to set the character's animation state
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetCharacterState(ECharacterState NewState);
    
    // Blueprint event that fires when the character state changes
    // Using different parameter names to avoid shadowing
    UFUNCTION(BlueprintImplementableEvent, Category = "Animation")
    void OnCharacterStateChanged(ECharacterState NewCharacterState, ECharacterState OldCharacterState);
    
    // Function to update the sprite based on state - implemented in Blueprint
    UFUNCTION(BlueprintImplementableEvent, Category = "Animation")
    void UpdateCharacterSprite();
    
    // Reference to the visual component (Sprite or Mesh) - set in Blueprint
    // Works with any mesh component type (Skeletal Mesh, Static Mesh, or Paper2D Flipbook)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UMeshComponent* CharacterVisual;
    
    // Blueprint-accessible function to get the current state
    UFUNCTION(BlueprintPure, Category = "Animation")
    ECharacterState GetCharacterState() const { return CurrentState; }
    
    // Check if character is in a specific state
    UFUNCTION(BlueprintPure, Category = "Animation")
    bool IsInState(ECharacterState StateToCheck) const { return CurrentState == StateToCheck; }
    
    // Get time spent in current state (useful for transitions)
    UFUNCTION(BlueprintPure, Category = "Animation")
    float GetTimeInCurrentState() const { return StateTimer; }

    // Jump related properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jump")
    bool bCanDoubleJump;

    // initialize DoubleJumpZVelocity
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jump")
    float DoubleJumpZVelocity;

    // Camera component
    UPROPERTY(VisibleAnywhere)
    class UCameraComponent* SideViewCamera;

    UPROPERTY(EditAnywhere)
    float RotationRate = 180.0f;

    float JumpZVelocity;
    
    // Health System
    
    // Health component to manage player health
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health")
    UPlayerHealthComponent* HealthComponent;
    
    // Blueprint event that fires when health changes
    UFUNCTION(BlueprintImplementableEvent, Category = "Health")
    void OnHealthChanged(int32 CurrentHealth, int32 MaxHealth);
    
    // Blueprint event that fires when player takes damage
    UFUNCTION(BlueprintImplementableEvent, Category = "Health")
    void OnTakeDamage(int32 DamageAmount, EDamageType DamageType);
    
    // Process damage from a damage actor
    UFUNCTION(BlueprintCallable, Category = "Health")
    void ProcessDamage(float DamageAmount, AActor* DamageCauser);
    
    // Handle game over from health component
    UFUNCTION()
    void HandlePlayerDeath(int32 TotalHitsTaken);
    
    // Check if player is dead based on health
    UFUNCTION(BlueprintPure, Category = "Health")
    bool IsDead() const;

protected:
    // Override Jump function
    virtual void Jump() override;

    void MoveRight(float Value);

    // Handle death logic
    UFUNCTION(BlueprintImplementableEvent)
    void DeathOfPlayer();
    
    // Update the animation state based on character movement
    void UpdateAnimationState();

public:
    void RestartLevel();

    UFUNCTION()
    void OnOverlapBegin(
        UPrimitiveComponent* OverlappedComp,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult
    );
    
    // Override TakeDamage to integrate with health component
    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, 
        class AController* EventInstigator, AActor* DamageCauser) override;

private:
    float zPosition;
    FVector TempPos = FVector();

    bool CanMove;
    bool CanJump;
    bool CanDoubleJump;
    
    // Timer to track how long we've been in the current state
    float StateTimer;
};
