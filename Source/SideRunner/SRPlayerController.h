// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SRPlayerController.generated.h"

/**
 * Custom player controller class for handling movement and interactions
 */
UCLASS()
class SIDERUNNER_API ASRPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    ASRPlayerController();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

protected:
    virtual void SetupInputComponent() override;

    // Movement functions
    void MoveForward(float Value);
    void MoveRight(float Value);
    void Jump();

    // Interaction handling
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void InteractWithObject();

    // Movement properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float MovementSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float JumpForce = 500.0f;

private:
    bool bCanInteract;
};
