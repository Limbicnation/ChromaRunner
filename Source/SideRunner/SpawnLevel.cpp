#include "SpawnLevel.h"
#include "BaseLevel.h"
#include "SideRunner.h" // Custom log categories
#include "Engine/World.h"
#include "Components/BoxComponent.h"
#include "TimerManager.h"

// Sets default values
ASpawnLevel::ASpawnLevel()
{
    PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ASpawnLevel::BeginPlay()
{
    Super::BeginPlay();

    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        PlayerWeakPtr = PC->GetPawn();
        if (PlayerWeakPtr.IsValid())
        {
            SpawnInitialLevels();
        }
        else
        {
            UE_LOG(LogSideRunner, Warning, TEXT("Player pawn not found at BeginPlay. Spawning will be delayed."));
        }
    }
    else
    {
        UE_LOG(LogSideRunner, Warning, TEXT("No PlayerController found at BeginPlay."));
    }
}

void ASpawnLevel::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Clear all pending timers
    for (FTimerHandle& TimerHandle : PendingDestroyTimers)
    {
        GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
    }
    PendingDestroyTimers.Empty();

    // Unbind delegates from all remaining levels
    for (ABaseLevel* Level : LevelList)
    {
        if (IsValid(Level) && Level->GetTrigger())
        {
            Level->GetTrigger()->OnComponentBeginOverlap.RemoveDynamic(this, &ASpawnLevel::OnOverlapBegin);
        }
    }
    LevelList.Empty();

    Super::EndPlay(EndPlayReason);
}

void ASpawnLevel::TryAcquirePlayerPawn()
{
    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        PlayerWeakPtr = PC->GetPawn();
    }
}

// Called every frame
void ASpawnLevel::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Re-acquire player reference if invalid (handles respawn scenarios)
    if (!PlayerWeakPtr.IsValid())
    {
        TryAcquirePlayerPawn();
        if (PlayerWeakPtr.IsValid() && LevelList.Num() == 0)
        {
            SpawnInitialLevels();
        }
    }
}

void ASpawnLevel::SpawnInitialLevels()
{
    if (LevelList.Num() == 0)
    {
        for (int32 i = 0; i < 4; ++i)
        {
            SpawnLevel(i == 0);
        }
    }
}

void ASpawnLevel::SpawnLevel(bool IsFirst)
{
    FVector NewSpawnLocation = FVector(0.0f, 1000.0f, 0.0f);
    FRotator NewSpawnRotation = FRotator(0, 90, 0);

    if (!IsFirst && LevelList.Num() > 0)
    {
        // Find the last valid level in the list
        ABaseLevel* LastLevel = nullptr;
        for (int32 i = LevelList.Num() - 1; i >= 0; --i)
        {
            if (IsValid(LevelList[i]))
            {
                LastLevel = LevelList[i];
                break;
            }
            else
            {
                // Remove stale reference
                UE_LOG(LogSideRunner, Warning, TEXT("Removing invalid level reference at index %d"), i);
                LevelList.RemoveAt(i);
            }
        }

        if (LastLevel && LastLevel->GetSpawnLocation())
        {
            NewSpawnLocation = LastLevel->GetSpawnLocation()->GetComponentTransform().GetTranslation();
        }
        else
        {
            UE_LOG(LogSideRunner, Warning, TEXT("No valid last level found - using default spawn location"));
        }
    }

    int32 MaxLevels = 6;
    int32 RandomLevel = FMath::RandRange(1, MaxLevels);
    TSubclassOf<ABaseLevel> LevelClass = nullptr;

    switch (RandomLevel)
    {
    case 1: LevelClass = Level1; break;
    case 2: LevelClass = Level2; break;
    case 3: LevelClass = Level3; break;
    case 4: LevelClass = Level4; break;
    case 5: LevelClass = Level5; break;
    case 6: LevelClass = Level6; break;
    default: UE_LOG(LogSideRunner, Warning, TEXT("Invalid level number: %d"), RandomLevel); break;
    }

    if (LevelClass)
    {
        ABaseLevel* NewLevel = GetWorld()->SpawnActor<ABaseLevel>(LevelClass, NewSpawnLocation, NewSpawnRotation, FActorSpawnParameters());
        if (NewLevel)
        {
            if (NewLevel->GetTrigger())
            {
                NewLevel->GetTrigger()->OnComponentBeginOverlap.AddDynamic(this, &ASpawnLevel::OnOverlapBegin);
            }
            LevelList.Add(NewLevel);

            if (LevelList.Num() > MaxLevels)
            {
                DelayedDestroyOldestLevel();
            }
        }
    }
}

void ASpawnLevel::DelayedDestroyOldestLevel()
{
    if (LevelList.Num() == 0)
    {
        return;
    }

    // Capture the level to destroy and remove from array immediately
    TWeakObjectPtr<ABaseLevel> LevelToDestroy = LevelList[0];
    LevelList.RemoveAt(0);

    FTimerHandle NewTimerHandle;
    FTimerDelegate TimerDel;

    // Use lambda to capture the specific level instance
    TimerDel.BindLambda([this, LevelToDestroy, NewTimerHandle]()
    {
        if (LevelToDestroy.IsValid())
        {
            // Unbind delegate before destruction to prevent stale callbacks
            if (UBoxComponent* Trigger = LevelToDestroy->GetTrigger())
            {
                Trigger->OnComponentBeginOverlap.RemoveDynamic(this, &ASpawnLevel::OnOverlapBegin);
            }

            LevelToDestroy->Destroy();
            UE_LOG(LogSideRunner, Verbose, TEXT("Destroyed old level segment"));
        }

        // Clean up timer handle from array
        PendingDestroyTimers.Remove(NewTimerHandle);
    });

    GetWorld()->GetTimerManager().SetTimer(NewTimerHandle, TimerDel, LevelDestroyDelay, false);
    PendingDestroyTimers.Add(NewTimerHandle);
}

void ASpawnLevel::DestroyOldestLevel()
{
    // Legacy function kept for backward compatibility - now handled by lambda in DelayedDestroyOldestLevel
    if (LevelList.Num() > 0)
    {
        ABaseLevel* OldestLevel = LevelList[0];
        LevelList.RemoveAt(0);
        if (IsValid(OldestLevel))
        {
            if (UBoxComponent* Trigger = OldestLevel->GetTrigger())
            {
                Trigger->OnComponentBeginOverlap.RemoveDynamic(this, &ASpawnLevel::OnOverlapBegin);
            }
            OldestLevel->Destroy();
        }
    }
}

void ASpawnLevel::ResetLevelsForRespawn()
{
    // Guard clause: Ensure we have a valid world context
    if (!GetWorld())
    {
        UE_LOG(LogSideRunner, Warning, TEXT("ResetLevelsForRespawn: Called with no world, aborting."));
        return;
    }

    UE_LOG(LogSideRunner, Log, TEXT("ResetLevelsForRespawn: Clearing all levels for player respawn"));

    // Clear all pending destroy timers to prevent callbacks on destroyed levels
    for (FTimerHandle& TimerHandle : PendingDestroyTimers)
    {
        if (GetWorld())
        {
            GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
        }
    }
    PendingDestroyTimers.Empty();

    // Destroy all existing levels and unbind delegates
    for (ABaseLevel* Level : LevelList)
    {
        if (IsValid(Level))
        {
            if (UBoxComponent* Trigger = Level->GetTrigger())
            {
                Trigger->OnComponentBeginOverlap.RemoveDynamic(this, &ASpawnLevel::OnOverlapBegin);
            }
            Level->Destroy();
        }
    }
    LevelList.Empty();

    // Re-acquire player reference and spawn fresh levels
    TryAcquirePlayerPawn();
    if (PlayerWeakPtr.IsValid())
    {
        SpawnInitialLevels();
        UE_LOG(LogSideRunner, Log, TEXT("ResetLevelsForRespawn: Spawned %d fresh levels"), LevelList.Num());
    }
    else
    {
        UE_LOG(LogSideRunner, Warning, TEXT("ResetLevelsForRespawn: Player not found, levels will spawn when player becomes available"));
    }
}

void ASpawnLevel::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
    const FHitResult& SweepResult)
{
    if (!OtherActor)
    {
        return;
    }

    // Refresh player reference if stale (handles respawn)
    if (!PlayerWeakPtr.IsValid())
    {
        TryAcquirePlayerPawn();
    }

    // Check if overlapping actor is the current player pawn
    if (PlayerWeakPtr.IsValid() && OtherActor == PlayerWeakPtr.Get())
    {
        UE_LOG(LogSideRunner, Verbose, TEXT("Player triggered level spawn at %s"),
            OverlappedComp && OverlappedComp->GetOwner() ? *OverlappedComp->GetOwner()->GetName() : TEXT("Unknown"));
        SpawnLevel(false);
    }
}
