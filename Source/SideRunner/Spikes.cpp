#include "Spikes.h"
#include "Kismet/GameplayStatics.h"
#include "RunnerCharacter.h"

ASpikes::ASpikes()
{
    PrimaryActorTick.bCanEverTick = true;
    Speed = 100.0f;
    MaxHeightOffset = 100.0f;
    MovementDirection = 1;
}

void ASpikes::BeginPlay()
{
    Super::BeginPlay();
    InitialZ = GetActorLocation().Z;
    if (!CollisionSound)
    {
        UE_LOG(LogTemp, Warning, TEXT("CollisionSound is not set in the editor!"));
    }
}

void ASpikes::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    FVector NewLocation = GetActorLocation();
    NewLocation.Z += Speed * DeltaTime * MovementDirection;

    if ((MovementDirection == 1 && NewLocation.Z >= InitialZ + MaxHeightOffset) ||
        (MovementDirection == -1 && NewLocation.Z <= InitialZ))
    {
        MovementDirection *= -1;
    }

    SetActorLocation(NewLocation);
}

// Updated NotifyHit function with the correct signature
void ASpikes::NotifyHit(
    UPrimitiveComponent* MyComp,
    AActor* Other,
    UPrimitiveComponent* OtherComp,
    bool bSelfMoved,
    FVector HitLocation,
    FVector HitNormal,
    FVector NormalImpulse,
    const FHitResult& Hit
)
{
    UE_LOG(LogTemp, Warning, TEXT("Hit detected with Spikes!"));

    if (ARunnerCharacter* PlayerCharacter = Cast<ARunnerCharacter>(Other))
    {
        if (CollisionSound)
        {
            UGameplayStatics::PlaySoundAtLocation(this, CollisionSound, GetActorLocation());
            UE_LOG(LogTemp, Warning, TEXT("Playing CollisionSound."));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("CollisionSound is nullptr."));
        }
    }
}
