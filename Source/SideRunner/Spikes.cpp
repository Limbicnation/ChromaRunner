#include "Spikes.h"
#include "Kismet/GameplayStatics.h"
#include "RunnerCharacter.h"

// Sets default values
ASpikes::ASpikes()
{
    // Set this actor to call Tick() every frame. You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    // Initialize default values
    Speed = 100.0f;
    MaxHeightOffset = 100.0f;
    MovementDirection = 1;
}

// Called when the game starts or when spawned
void ASpikes::BeginPlay()
{
    Super::BeginPlay();
    InitialZ = GetActorLocation().Z;
    if (!CollisionSound)
    {
        UE_LOG(LogTemp, Warning, TEXT("CollisionSound is not set in the editor!"));
    }
}

// Called every frame
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

// Override the NotifyHit function to detect collisions
void ASpikes::NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, bool bSelfMoved, const FHitResult& Hit)
{
    // Debug message to confirm collision detection
    UE_LOG(LogTemp, Warning, TEXT("Hit detected with Spikes!"));

    // Check if the colliding actor is the player character
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
