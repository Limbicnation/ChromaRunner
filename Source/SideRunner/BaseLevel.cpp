#include "BaseLevel.h"
#include "Components/BoxComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"

#if WITH_EDITOR
#include "DrawDebugHelpers.h"
#endif

// Sets default values
ABaseLevel::ABaseLevel()
    : bShowDebugBoxes(false)
    , LevelLength(1000.0f)
    , DifficultyLevel(1)
    , bIsEndLevel(false)
{
    // PERFORMANCE: Disable tick by default - only enable when debug visualization is needed
    PrimaryActorTick.bCanEverTick = false;
    PrimaryActorTick.bStartWithTickEnabled = false;

    // Initialize Trigger component with optimal settings
    Trigger = CreateDefaultSubobject<UBoxComponent>(TEXT("Trigger"));
    RootComponent = Trigger;

    if (Trigger)
    {
        Trigger->SetCollisionProfileName(TEXT("Trigger"));
        Trigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        Trigger->SetCollisionResponseToAllChannels(ECR_Ignore);
        Trigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
        
        // PERFORMANCE: Optimize collision complexity
        Trigger->SetCollisionObjectType(ECC_WorldStatic);
        Trigger->SetGenerateOverlapEvents(true);
        Trigger->SetNotifyRigidBodyCollision(false); // Not needed for trigger
    }

    // Initialize SpawnLocation component
    SpawnLocation = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnLocation"));
    if (SpawnLocation)
    {
        SpawnLocation->SetupAttachment(RootComponent);
        SpawnLocation->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        SpawnLocation->SetCollisionResponseToAllChannels(ECR_Ignore);
    }
}

// Called when the game starts or when spawned
void ABaseLevel::BeginPlay()
{
    Super::BeginPlay();

    // PERFORMANCE: Hide components in game for optimal performance
    if (Trigger)
    {
        Trigger->SetHiddenInGame(true);
        Trigger->OnComponentBeginOverlap.AddDynamic(this, &ABaseLevel::OnTriggerOverlap);
    }

    if (SpawnLocation)
    {
        SpawnLocation->SetHiddenInGame(true);
    }

    // PERFORMANCE: Only enable tick when debug visualization is active
#if WITH_EDITOR
    PrimaryActorTick.bCanEverTick = bShowDebugBoxes;
#endif

    // PERFORMANCE: Pre-validate level actors array
    ValidateLevelActors();
}

void ABaseLevel::ValidateLevelActors()
{
    // PERFORMANCE: Remove null or invalid actors from the array
    LevelActors.RemoveAll([](const AActor* Actor)
    {
        return !IsValid(Actor);
    });

#if UE_BUILD_DEVELOPMENT
    UE_LOG(LogTemp, Log, TEXT("BaseLevel %s validated %d level actors"), *GetName(), LevelActors.Num());
#endif
}

// Called every frame
void ABaseLevel::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // PERFORMANCE: This should only run in editor with debug visualization
#if WITH_EDITOR
    if (bShowDebugBoxes)
    {
        DrawDebugVisualization();
    }
#endif
}

#if WITH_EDITOR
void ABaseLevel::DrawDebugVisualization()
{
    const UWorld* World = GetWorld();
    if (!World)
        return;

    // PERFORMANCE: Use const references and avoid repeated calculations
    const FVector TriggerLocation = Trigger ? Trigger->GetComponentLocation() : GetActorLocation();
    const FVector TriggerExtent = Trigger ? Trigger->GetScaledBoxExtent() : FVector(100.0f);
    const FQuat TriggerQuat = Trigger ? Trigger->GetComponentQuat() : GetActorQuat();

    const FVector SpawnLocation_Loc = SpawnLocation ? SpawnLocation->GetComponentLocation() : GetActorLocation();
    const FVector SpawnLocationExtent = SpawnLocation ? SpawnLocation->GetScaledBoxExtent() : FVector(50.0f);
    const FQuat SpawnLocationQuat = SpawnLocation ? SpawnLocation->GetComponentQuat() : GetActorQuat();

    // Draw trigger box in red
    DrawDebugBox(World, TriggerLocation, TriggerExtent, TriggerQuat, FColor::Red, false, -1.0f, 0, 2.0f);

    // Draw spawn location box in green
    DrawDebugBox(World, SpawnLocation_Loc, SpawnLocationExtent, SpawnLocationQuat, FColor::Green, false, -1.0f, 0, 2.0f);

    // Draw level information
    const FString InfoText = FString::Printf(TEXT("Level: %d | Length: %.0f | Actors: %d"), 
                                           DifficultyLevel, LevelLength, LevelActors.Num());
    DrawDebugString(World, GetActorLocation() + FVector(0, 0, 200), InfoText, nullptr, FColor::White, -1.0f, true);
}
#endif

void ABaseLevel::OnTriggerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    // PERFORMANCE: Quick validation and early exit
    if (!OtherActor)
        return;

    // Check if the overlapping actor is a player-controlled character
    const ACharacter* PlayerCharacter = Cast<ACharacter>(OtherActor);
    if (PlayerCharacter && PlayerCharacter->IsPlayerControlled())
    {
        // Broadcast the level triggered event
        OnLevelTriggered.Broadcast(this);

#if UE_BUILD_DEVELOPMENT
        UE_LOG(LogTemp, Log, TEXT("Level %s triggered by player"), *GetName());
#endif
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
    // PERFORMANCE: Use range-based for loop and validate actors
    int32 ActivatedCount = 0;
    for (AActor* Actor : LevelActors)
    {
        if (IsValid(Actor))
        {
            Actor->SetActorHiddenInGame(false);
            Actor->SetActorEnableCollision(true);
            Actor->SetActorTickEnabled(true);
            ActivatedCount++;
        }
    }

#if UE_BUILD_DEVELOPMENT
    UE_LOG(LogTemp, Log, TEXT("Level %s activated %d actors"), *GetName(), ActivatedCount);
#endif
}

void ABaseLevel::DeactivateLevel()
{
    // PERFORMANCE: Use range-based for loop and validate actors
    int32 DeactivatedCount = 0;
    for (AActor* Actor : LevelActors)
    {
        if (IsValid(Actor))
        {
            Actor->SetActorHiddenInGame(true);
            Actor->SetActorEnableCollision(false);
            Actor->SetActorTickEnabled(false);
            DeactivatedCount++;
        }
    }

#if UE_BUILD_DEVELOPMENT
    UE_LOG(LogTemp, Log, TEXT("Level %s deactivated %d actors"), *GetName(), DeactivatedCount);
#endif
}

float ABaseLevel::GetLevelLength() const
{
    return LevelLength;
}

int32 ABaseLevel::GetDifficultyLevel() const
{
    return DifficultyLevel;
}

bool ABaseLevel::IsEndLevel() const
{
    return bIsEndLevel;
}

#if WITH_EDITOR
void ABaseLevel::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    const FName PropertyName = (PropertyChangedEvent.Property != nullptr) ?
        PropertyChangedEvent.Property->GetFName() : NAME_None;

    // PERFORMANCE: Update tick state when debug visualization changes
    if (PropertyName == GET_MEMBER_NAME_CHECKED(ABaseLevel, bShowDebugBoxes))
    {
        PrimaryActorTick.bCanEverTick = bShowDebugBoxes;
        SetActorTickEnabled(bShowDebugBoxes);
    }
    // Validate difficulty level
    else if (PropertyName == GET_MEMBER_NAME_CHECKED(ABaseLevel, DifficultyLevel))
    {
        DifficultyLevel = FMath::Clamp(DifficultyLevel, 1, 10);
    }
    // Validate level length
    else if (PropertyName == GET_MEMBER_NAME_CHECKED(ABaseLevel, LevelLength))
    {
        LevelLength = FMath::Max(100.0f, LevelLength);
    }
    // Re-validate level actors when the array changes
    else if (PropertyName == GET_MEMBER_NAME_CHECKED(ABaseLevel, LevelActors))
    {
        ValidateLevelActors();
    }
}
#endif