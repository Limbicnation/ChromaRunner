// Fill out your copyright notice in the Description page of Project Settings.

#include "SRPlayerController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

ASRPlayerController::ASRPlayerController()
{
    PrimaryActorTick.bCanEverTick = true;
    bCanInteract = true;
}

void ASRPlayerController::BeginPlay()
{
    Super::BeginPlay();
}

void ASRPlayerController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ASRPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    // Bind movement functions
    InputComponent->BindAxis("MoveForward", this, &ASRPlayerController::MoveForward);
    InputComponent->BindAxis("MoveRight", this, &ASRPlayerController::MoveRight);
    InputComponent->BindAction("Jump", IE_Pressed, this, &ASRPlayerController::Jump);
    InputComponent->BindAction("Interact", IE_Pressed, this, &ASRPlayerController::InteractWithObject);
}

void ASRPlayerController::MoveForward(float Value)
{
    if (Value != 0.0f && GetPawn())
    {
        GetPawn()->AddMovementInput(FVector::ForwardVector, Value * MovementSpeed);
    }
}

void ASRPlayerController::MoveRight(float Value)
{
    if (Value != 0.0f && GetPawn())
    {
        GetPawn()->AddMovementInput(FVector::RightVector, Value * MovementSpeed);
    }
}

void ASRPlayerController::Jump()
{
    if (ACharacter* Character = Cast<ACharacter>(GetPawn()))
    {
        Character->Jump();
    }
}

void ASRPlayerController::InteractWithObject()
{
    if (!bCanInteract) return;

    // Implementation for interaction with objects in the game world
    // This will be expanded based on specific interaction requirements
}
