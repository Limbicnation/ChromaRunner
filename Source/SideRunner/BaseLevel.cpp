#include "BaseLevel.h"
#include "Components/BoxComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"

// Sets default values
ABaseLevel::ABaseLevel()
    : LevelLength(1000.0f)     // FIXED: Initialize in correct order
    , DifficultyLevel(1)
    , bIsEndLevel(false)
    , bLevelTriggered(false)  // NEW: Prevent multiple triggers
    , bShowDebugBoxes(false)
{
    // Disable tick by default for performance - enable only if needed
    PrimaryActorTick.bCanEverTick = false;

    // Initialize Trigger component with optimized collision settings
    Trigger = CreateDefaultSubobject<UBoxComponent>(TEXT("Trigger"));
    RootComponent = Trigger;

    if (Trigger)
    {
        Trigger->SetCollisionProfileName(TEXT("Trigger"));
        Trigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        Trigger->SetCollisionResponseToAllChannels(ECR_Ignore);
        Trigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
        
        // OPTIMIZATION: Reduce collision complexity for triggers
        Trigger->SetCollisionObjectType(ECC_WorldStatic);
        Trigger->SetGenerateOverlapEvents(true);
        
        // Set reasonable default size
        Trigger->SetBoxExtent(FVector(100.0f, 100.0f, 100.0f));
    }

    // Initialize SpawnLocation component
    SpawnLocation = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnLocation"));
    if (SpawnLocation)
    {
        SpawnLocation->SetupAttachment(RootComponent);
        SpawnLocation->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        SpawnLocation->SetGenerateOverlapEvents(false);
        
        // Set reasonable default size for spawn location
        SpawnLocation->SetBoxExtent(FVector(50.0f, 50.0f, 50.0f));
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
        
        // OPTIMIZATION: Set relative location for spawn location
        if (SpawnLocation)
        {
            SpawnLocation->SetRelativeLocation(FVector(LevelLength, 0.0f, 0.0f));
        }
    }

    if (SpawnLocation)
    {
        SpawnLocation->SetHiddenInGame(true);
    }

    // Enable tick only if debug visualization is needed
    SetActorTickEnabled(bShowDebugBoxes);
    
    // Reset trigger state
    bLevelTriggered = false;
}

// Called every frame (only when debug visualization is active)
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
    // OPTIMIZATION: Early exit if already triggered
    if (bLevelTriggered)
    {
        return;
    }

    // Check if the overlapping actor is a player character
    ACharacter* PlayerCharacter = Cast<ACharacter>(OtherActor);
    if (PlayerCharacter && PlayerCharacter->IsPlayerControlled())
    {
        // Mark as triggered to prevent spam
        bLevelTriggered = true;
        
        // Broadcast the level triggered event
        OnLevelTriggered.Broadcast(this);

        UE_LOG(LogTemp, Log, TEXT("Level %s triggered by player"), *GetName());
        
        // OPTIMIZATION: Add small delay before allowing retrigger
        FTimerHandle RetriggerTimer;
        GetWorldTimerManager().SetTimer(RetriggerTimer, this, &ABaseLevel::ResetTrigger, 1.0f, false);
    }
}

void ABaseLevel::ResetTrigger()
{
    bLevelTriggered = false;
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
    // OPTIMIZATION: Batch operations for better performance
    if (LevelActors.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Level %s has no actors to activate"), *GetName());
        return;
    }

    // Enable all actors in this level segment
    for (AActor* Actor : LevelActors)
    {
        if (IsValid(Actor))
        {
            Actor->SetActorHiddenInGame(false);
            Actor->SetActorEnableCollision(true);
            Actor->SetActorTickEnabled(true);
        }
    }

    // Log level activation
    UE_LOG(LogTemp, Log, TEXT("Level %s activated with %d actors"), *GetName(), LevelActors.Num());
}

void ABaseLevel::DeactivateLevel()
{
    // OPTIMIZATION: Batch operations for better performance
    if (LevelActors.Num() == 0)
    {
        return;
    }

    // Disable all actors in this level segment for performance
    for (AActor* Actor : LevelActors)
    {
        if (IsValid(Actor))
        {
            Actor->SetActorHiddenInGame(true);
            Actor->SetActorEnableCollision(false);
            Actor->SetActorTickEnabled(false);
        }
    }

    // Log level deactivation
    UE_LOG(LogTemp, Log, TEXT("Level %s deactivated with %d actors"), *GetName(), LevelActors.Num());
}

// NEW: Function to properly handle level streaming
void ABaseLevel::SetDebugVisualization(bool bEnabled)
{
    bShowDebugBoxes = bEnabled;
    SetActorTickEnabled(bEnabled);
}

// NEW: Get level bounds for optimization - FIXED function call
FBox ABaseLevel::GetLevelBounds() const
{
    FBox LevelBounds(ForceInit);
    
    if (Trigger)
    {
        LevelBounds += Trigger->Bounds.GetBox();
    }
    
    if (SpawnLocation)
    {
        LevelBounds += SpawnLocation->Bounds.GetBox();
    }
    
    // Include all level actors in bounds calculation - FIXED: Use correct function signature
    for (const AActor* Actor : LevelActors)
    {
        if (IsValid(Actor))
        {
            FVector Origin, BoxExtent;
            Actor->GetActorBounds(false, Origin, BoxExtent);  // FIXED: Use correct function signature
            LevelBounds += FBox(Origin - BoxExtent, Origin + BoxExtent);
        }
    }
    
    return LevelBounds;
}