#include "BaseLevel.h"
#include "Components/BoxComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"

// Sets default values
ABaseLevel::ABaseLevel()
    : bShowDebugBoxes(false)
    , LevelLength(1000.0f)
    , DifficultyLevel(1)
    , bIsEndLevel(false)
{
    // Disable tick by default for performance - enable only if needed
    PrimaryActorTick.bCanEverTick = false;

    // Initialize Trigger component with proper collision settings
    Trigger = CreateDefaultSubobject<UBoxComponent>(TEXT("Trigger"));
    RootComponent = Trigger;

    if (Trigger)
    {
        Trigger->SetCollisionProfileName(TEXT("Trigger"));
        Trigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        Trigger->SetCollisionResponseToAllChannels(ECR_Ignore);
        Trigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    }

    // Initialize SpawnLocation component
    SpawnLocation = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnLocation"));
    if (SpawnLocation)
    {
        SpawnLocation->SetupAttachment(RootComponent);
        SpawnLocation->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
}

// Called when the game starts or when spawned
void ABaseLevel::BeginPlay()
{
    Super::BeginPlay();

    // Register overlap event
    if (Trigger)
    {
        Trigger->SetHiddenInGame(true);
        Trigger->OnComponentBeginOverlap.AddDynamic(this, &ABaseLevel::OnTriggerOverlap);
    }

    if (SpawnLocation)
    {
        SpawnLocation->SetHiddenInGame(true);
    }

    // Enable tick only if debug visualization is needed
    PrimaryActorTick.bCanEverTick = bShowDebugBoxes;
}

// Called every frame
void ABaseLevel::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Draw debug visualization if enabled
    if (bShowDebugBoxes && GetWorld())
    {
        if (Trigger)
        {
            DrawDebugBox(
                GetWorld(),
                Trigger->GetComponentLocation(),
                Trigger->GetScaledBoxExtent(),
                Trigger->GetComponentQuat(),
                FColor::Red,
                false,
                -1.0f,
                0,
                2.0f
            );
        }

        if (SpawnLocation)
        {
            DrawDebugBox(
                GetWorld(),
                SpawnLocation->GetComponentLocation(),
                SpawnLocation->GetScaledBoxExtent(),
                SpawnLocation->GetComponentQuat(),
                FColor::Green,
                false,
                -1.0f,
                0,
                2.0f
            );
        }
    }
}

void ABaseLevel::OnTriggerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    // Check if the overlapping actor is a player character
    ACharacter* PlayerCharacter = Cast<ACharacter>(OtherActor);
    if (PlayerCharacter && PlayerCharacter->IsPlayerControlled())
    {
        // Broadcast the level triggered event
        OnLevelTriggered.Broadcast(this);

        UE_LOG(LogTemp, Log, TEXT("Level %s triggered by player"), *GetName());
    }
}

UBoxComponent* ABaseLevel::GetTrigger() const
{
    return Trigger;
}

UBoxComponent* ABaseLevel::GetSpawnLocation() const
{
    return SpawnLocation;
}

void ABaseLevel::ActivateLevel()
{
    // Enable all actors in this level segment
    for (AActor* Actor : LevelActors)
    {
        if (Actor)
        {
            Actor->SetActorHiddenInGame(false);
            Actor->SetActorEnableCollision(true);
            Actor->SetActorTickEnabled(true);
        }
    }

    // Log level activation
    UE_LOG(LogTemp, Log, TEXT("Level %s activated"), *GetName());
}

void ABaseLevel::DeactivateLevel()
{
    // Disable all actors in this level segment for performance
    for (AActor* Actor : LevelActors)
    {
        if (Actor)
        {
            Actor->SetActorHiddenInGame(true);
            Actor->SetActorEnableCollision(false);
            Actor->SetActorTickEnabled(false);
        }
    }

    // Log level deactivation
    UE_LOG(LogTemp, Log, TEXT("Level %s deactivated"), *GetName());
}