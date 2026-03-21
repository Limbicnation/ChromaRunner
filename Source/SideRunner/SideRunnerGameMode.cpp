#include "SideRunnerGameMode.h"
#include "SideRunner.h"
#include "SideRunner/RunnerCharacter.h"
#include "Components/Widget.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"

ASideRunnerGameMode::ASideRunnerGameMode()
{
    // No default pawn — let Blueprint subclasses define their own
    bPauseable = false;
}

void ASideRunnerGameMode::BeginPlay()
{
    Super::BeginPlay();

    // Bind to the player character's death delegate if one exists in this session
    if (UWorld* World = GetWorld())
    {
        for (TActorIterator<ARunnerCharacter> It(World); It; ++It)
        {
            ARunnerCharacter* Player = *It;
            if (Player && Player->HealthComponent)
            {
                Player->HealthComponent->OnDeath.AddUniqueDynamic(this, &ASideRunnerGameMode::OnPlayerDeath);
                UE_LOG(LogSideRunner, Log, TEXT("[GameMode] Bound to %s death delegate"), *Player->GetName());
            }
            break; // only one player character
        }
    }
}

void ASideRunnerGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Clean up any active widget
    if (CurrentWidget)
    {
        CurrentWidget->RemoveFromParent();
        CurrentWidget = nullptr;
    }
    Super::EndPlay(EndPlayReason);
}

void ASideRunnerGameMode::AddScore(int32 Points)
{
    if (Points <= 0) return;
    Score += Points;
    if (Score > HighScore)
    {
        HighScore = Score;
    }
    UE_LOG(LogSideRunner, Log, TEXT("[GameMode] Score: %d (+%d)"), Score, Points);
}

int32 ASideRunnerGameMode::GetScore() const
{
    return Score;
}

int32 ASideRunnerGameMode::GetHighScore() const
{
    return HighScore;
}

void ASideRunnerGameMode::OnPlayerDeath()
{
    UE_LOG(LogSideRunner, Log, TEXT("[GameMode] Player died — Score: %d"), Score);

    // Show the Game Over widget if one is configured
    if (GameOverWidgetClass)
    {
        if (UWorld* World = GetWorld())
        {
            CurrentWidget = CreateWidget<UUserWidget>(World, GameOverWidgetClass);
            if (CurrentWidget)
            {
                CurrentWidget->AddToViewport();

                // Attempt to call SetFinalScore if the widget implements it
                // (optional — the BP can handle this via BlueprintImplementableEvent too)
                UFunction* SetScoreFn = CurrentWidget->FindFunction(TEXT("SetFinalScore"));
                if (SetScoreFn)
                {
                    struct FSetScoreParams { int32 Score; };
                    CurrentWidget->ProcessEvent(SetScoreFn, &Score);
                }

                UE_LOG(LogSideRunner, Log, TEXT("[GameMode] GameOverWidget displayed"));
            }
        }
    }

    // Pause the game so physics/animation freezes
    UGameplayStatics::SetGamePaused(this, true);

    // Fire Blueprint event so the BP can do additional handling
    HandleGameOver();
}

void ASideRunnerGameMode::OnPlayerWon()
{
    UE_LOG(LogSideRunner, Log, TEXT("[GameMode] Player won! Final Score: %d"), Score);
    HandleGameWon();
}
