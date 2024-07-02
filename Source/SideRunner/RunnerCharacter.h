#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "RunnerCharacter.generated.h"

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

    // Declare the DoubleJump function
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jump")
    bool bCanDoubleJump;

    // initialize DoubleJumpZVelocity
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jump")
    float DoubleJumpZVelocity;

    UPROPERTY(VisibleAnywhere)
    class UCameraComponent* SideViewCamera;

    UPROPERTY(EditAnywhere)
    float RotationRate = 180.0f;

    float JumpZVelocity;

protected:
    // Declare the Jump function here
    virtual void Jump() override;

    void MoveRight(float Value);

    // Handle death logic
    UFUNCTION(BlueprintImplementableEvent)
    void DeathOfPlayer();

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

private:
    float zPosition;
    FVector TempPos = FVector();

    bool CanMove;
    bool CanJump;
    bool CanDoubleJump;
};
