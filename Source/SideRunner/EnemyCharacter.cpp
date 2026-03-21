#include "EnemyCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "RunnerCharacter.h"
#include "PlayerHealthComponent.h"

AEnemyCharacter::AEnemyCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // Configure movement for 2D side-scroller (XY plane)
    if (UCharacterMovementComponent* Move = GetCharacterMovement())
    {
        Move->bConstrainToPlane = true;
        Move->bSnapToPlaneAtStart = true;
        Move->MaxWalkSpeed = PatrolSpeed;
    }

    // Disable capsule rotation — sprite flips instead
    bUseControllerRotationYaw = false;

    // Enable overlap events on capsule
    GetCapsuleComponent()->SetGenerateOverlapEvents(true);
    GetCapsuleComponent()->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
}

void AEnemyCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Set up overlap delegates for player detection and stomp detection
    if (UCapsuleComponent* Capsule = GetCapsuleComponent())
    {
        Capsule->OnComponentBeginOverlap.AddDynamic(this, &AEnemyCharacter::OnOverlapBegin);
        Capsule->OnComponentEndOverlap.AddDynamic(this, &AEnemyCharacter::OnOverlapEnd);
    }
}

void AEnemyCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update damage cooldown
    if (!bCanDealDamage)
    {
        DamageCooldownTimer -= DeltaTime;
        if (DamageCooldownTimer <= 0.0f)
        {
            bCanDealDamage = true;
        }
    }

    // Simple patrol — move toward current target
    if (bIsPatrolling)
    {
        FVector Target = bMovingToEnd ? PatrolEnd : PatrolStart;
        FVector Direction = (Target - GetActorLocation()).GetSafeNormal2D();
        AddMovementInput(Direction, 1.0f);

        // Flip sprite to face patrol direction
        if (!Direction.IsNearlyZero())
        {
            FRotator NewRotation = GetActorRotation();
            NewRotation.Yaw = Direction.X > 0.0f ? 0.0f : 180.0f;
            SetActorRotation(NewRotation);
        }

        // Check if reached patrol point
        float Distance = FVector::Dist2D(GetActorLocation(), Target);
        if (Distance <= PatrolAcceptanceRadius)
        {
            bMovingToEnd = !bMovingToEnd;
            OnPatrolPointReached();
        }
    }
}

void AEnemyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    // Enemy doesn't need player input by default
}

void AEnemyCharacter::SetPatrolPoints(FVector Start, FVector End)
{
    PatrolStart = Start;
    PatrolEnd = End;
    bMovingToEnd = true;
}

void AEnemyCharacter::StartPatrol()
{
    bIsPatrolling = true;
    if (UCharacterMovementComponent* Move = GetCharacterMovement())
    {
        Move->MaxWalkSpeed = PatrolSpeed;
    }
}

void AEnemyCharacter::StopPatrol()
{
    bIsPatrolling = false;
    if (UCharacterMovementComponent* Move = GetCharacterMovement())
    {
        Move->MaxWalkSpeed = 0.0f;
    }
}

void AEnemyCharacter::OnOverlapBegin(
    UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    if (!OtherActor || OtherActor == this) return;

    // Check if the overlapping actor is the player
    if (ARunnerCharacter* Runner = Cast<ARunnerCharacter>(OtherActor))
    {
        // Detect stomp — player jumping on top of enemy
        float PlayerBottom = Runner->GetActorLocation().Z;
        float EnemyTop = GetActorLocation().Z + GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
        float RelativeZ = PlayerBottom - EnemyTop;

        if (RelativeZ > 0.0f)
        {
            // Player stomped on enemy — defeat enemy
            OnDefeated();
        }
        else
        {
            // Player hit enemy from side — damage player
            ApplyDamageWithCooldown(Runner);
        }
    }
}

void AEnemyCharacter::OnOverlapEnd(
    UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex)
{
    // Nothing needed here — damage cooldown handles repeat damage
}

void AEnemyCharacter::ApplyDamageWithCooldown(AActor* Player)
{
    if (!bCanDealDamage || !Player) return;

    DealDamageToPlayer(Player);
    bCanDealDamage = false;
    DamageCooldownTimer = DamageCooldown;
}

void AEnemyCharacter::DealDamageToPlayer(AActor* Player)
{
    if (!Player) return;

    ARunnerCharacter* Runner = Cast<ARunnerCharacter>(Player);
    if (Runner && Runner->HealthComponent && Runner->HealthComponent->IsFullyInitialized())
    {
        Runner->HealthComponent->TakeDamage(DamageAmount, EDamageType::EnemyMelee);
        OnEnemyDamagedPlayer(Player, DamageAmount);
    }
}

void AEnemyCharacter::OnDefeated()
{
    StopPatrol();
    OnEnemyDefeated();

    // Play death animation in Blueprint then destroy
    SetActorEnableCollision(false);
    SetLifeSpan(1.0f); // Let Blueprint handle animation, then auto-destroy
}

void AEnemyCharacter::OnPatrolPointReached()
{
    // Blueprint-implementable event for visual/audio feedback on patrol turnaround
    ReversePatrolDirection();
}

void AEnemyCharacter::ReversePatrolDirection()
{
    bMovingToEnd = !bMovingToEnd;
    PatrolDirection = (bMovingToEnd ? PatrolEnd : PatrolStart) - GetActorLocation();
    PatrolDirection.Z = 0.0f;
    PatrolDirection.Normalize();
}
