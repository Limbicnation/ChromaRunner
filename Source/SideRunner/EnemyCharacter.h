#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnemyCharacter.generated.h"

/**
 * Base enemy class for Chroma Runner.
 * Supports patrol behavior, player damage on contact, and defeat via stomp.
 * Extend in Blueprint for specific enemy types and animations.
 */
UCLASS(Blueprintable, BlueprintType)
class SIDERUNNER_API AEnemyCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AEnemyCharacter();

    // Movement
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // Patrol behavior
    UFUNCTION(BlueprintCallable, Category = "Patrol")
    void StartPatrol();

    UFUNCTION(BlueprintCallable, Category = "Patrol")
    void StopPatrol();

    UFUNCTION(BlueprintCallable, Category = "Patrol")
    void SetPatrolPoints(FVector Start, FVector End);

    UFUNCTION(BlueprintPure, Category = "Patrol")
    bool IsPatrolling() const { return bIsPatrolling; }

    UFUNCTION(BlueprintPure, Category = "Patrol")
    FVector GetPatrolDirection() const { return PatrolDirection; }

    // Damage system
    UFUNCTION(BlueprintCallable, Category = "Damage")
    void DealDamageToPlayer(AActor* Player);

    UFUNCTION(BlueprintCallable, Category = "Damage")
    void OnDefeated();

    UFUNCTION(BlueprintImplementableEvent, Category = "Damage")
    void OnEnemyDefeated();

    UFUNCTION(BlueprintImplementableEvent, Category = "Damage")
    void OnEnemyDamagedPlayer(AActor* Player, int32 DamageAmount);

    // Collision detection — stomp zone (top of enemy)
    UFUNCTION()
    void OnPatrolPointReached();

protected:
    virtual void BeginPlay() override;

    // Patrol properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
    bool bIsPatrolling = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
    float PatrolSpeed = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
    FVector PatrolStart;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
    FVector PatrolEnd;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
    FVector PatrolDirection = FVector::ForwardVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
    float PatrolAcceptanceRadius = 50.0f;

    // Damage properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    int32 DamageAmount = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    float DamageCooldown = 1.0f;

    UPROPERTY()
    FTimerHandle PatrolTimerHandle;

    UFUNCTION()
    void OnOverlapBegin(
        UPrimitiveComponent* OverlappedComp,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult
    );

    UFUNCTION()
    void OnOverlapEnd(
        UPrimitiveComponent* OverlappedComp,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex
    );

    UFUNCTION()
    void ApplyDamageWithCooldown(AActor* Player);

    UFUNCTION()
    void ReversePatrolDirection();

private:
    bool bMovingToEnd = true;
    float DamageCooldownTimer = 0.0f;
    bool bCanDealDamage = true;
};
