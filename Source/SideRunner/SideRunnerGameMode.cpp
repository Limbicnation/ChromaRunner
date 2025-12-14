#include "SideRunnerGameMode.h"
#include "SideRunnerGameInstance.h"
#include "SideRunner.h" // Custom log categories
#include "SideRunnerPlayerController.h"
#include "GameHUDWidget.h"
#include "GameOverWidget.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

ASideRunnerGameMode::ASideRunnerGameMode()
{
	// Initialize state
	bGameOverActive = false;

	// CRITICAL FIX: Set PlayerController class to enable Exec debug commands
	// Exec commands only work in PlayerController classes in UE5.5
	PlayerControllerClass = ASideRunnerPlayerController::StaticClass();

	UE_LOG(LogSideRunner, Log, TEXT("SideRunnerGameMode: Using SideRunnerPlayerController for debug command support"));
}

void ASideRunnerGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Cache game instance reference
	CachedGameInstance = Cast<USideRunnerGameInstance>(GetGameInstance());

	if (!IsValid(CachedGameInstance))
	{
		UE_LOG(LogSideRunner, Error, TEXT("SideRunnerGameMode: Failed to get SideRunnerGameInstance!"));
		return;
	}

	// Bind to game instance delegates
	CachedGameInstance->OnGameWon.AddDynamic(this, &ASideRunnerGameMode::OnGameWonHandler);
	CachedGameInstance->OnGameLost.AddDynamic(this, &ASideRunnerGameMode::OnGameLostHandler);

	UE_LOG(LogSideRunner, Log, TEXT("SideRunnerGameMode: Delegates bound to GameInstance"));

	// Create and display HUD
	CreateGameHUD();

	// Ensure game input mode is active
	SetInputModeGame();
}

void ASideRunnerGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Unbind delegates to prevent stale references
	if (IsValid(CachedGameInstance))
	{
		CachedGameInstance->OnGameWon.RemoveAll(this);
		CachedGameInstance->OnGameLost.RemoveAll(this);
	}

	// Clean up widget references
	if (ActiveHUDWidget.IsValid())
	{
		ActiveHUDWidget->RemoveFromParent();
		ActiveHUDWidget.Reset();
	}

	if (ActiveGameOverWidget.IsValid())
	{
		ActiveGameOverWidget->RemoveFromParent();
		ActiveGameOverWidget.Reset();
	}

	Super::EndPlay(EndPlayReason);
}

void ASideRunnerGameMode::CreateGameHUD()
{
	// Validate widget class
	if (!GameHUDWidgetClass)
	{
		UE_LOG(LogSideRunner, Error, TEXT("SideRunnerGameMode: GameHUDWidgetClass not set! Assign WBP_GameHUD in Blueprint."));
		return;
	}

	// Get player controller
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!IsValid(PC))
	{
		UE_LOG(LogSideRunner, Error, TEXT("SideRunnerGameMode: PlayerController not found!"));
		return;
	}

	// Create widget instance
	UGameHUDWidget* HUDWidget = CreateWidget<UGameHUDWidget>(PC, GameHUDWidgetClass);
	if (!IsValid(HUDWidget))
	{
		UE_LOG(LogSideRunner, Error, TEXT("SideRunnerGameMode: Failed to create GameHUDWidget!"));
		return;
	}

	// Add to viewport
	HUDWidget->AddToViewport(0); // Z-order 0 (background)

	// Store weak reference
	ActiveHUDWidget = HUDWidget;

	UE_LOG(LogSideRunner, Log, TEXT("SideRunnerGameMode: GameHUD created and added to viewport"));
}

void ASideRunnerGameMode::ShowGameOverScreen(bool bWon)
{
	// CRITICAL: Prevent duplicate game over screens
	if (bGameOverActive)
	{
		UE_LOG(LogSideRunner, Warning, TEXT("SideRunnerGameMode: Game over already active, ignoring duplicate call"));
		return;
	}

	bGameOverActive = true;

	// Validate widget class. This must be set in the derived Blueprint.
	if (!ensureMsgf(GameOverWidgetClass, TEXT("GameOverWidgetClass is not set! Assign a valid UGameOverWidget subclass (like WBP_GameOver) in the BP_SideRunnerGameMode Blueprint.")))
	{
		return; // Prevent further execution if the class is not set
	}

	// Validate game instance
	if (!IsValid(CachedGameInstance))
	{
		UE_LOG(LogSideRunner, Error, TEXT("SideRunnerGameMode: GameInstance invalid, cannot show game over screen"));
		return;
	}

	// Get player controller
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!IsValid(PC))
	{
		UE_LOG(LogSideRunner, Error, TEXT("SideRunnerGameMode: PlayerController not found!"));
		return;
	}

	// Hide game HUD
	HideGameHUD();

	// Create game over widget
	UGameOverWidget* GameOverWidget = CreateWidget<UGameOverWidget>(PC, GameOverWidgetClass);
	if (!IsValid(GameOverWidget))
	{
		UE_LOG(LogSideRunner, Error, TEXT("SideRunnerGameMode: Failed to create GameOverWidget!"));
		return;
	}

	// Setup widget with game statistics
	const int32 FinalScore = CachedGameInstance->GetCurrentScore();
	const float DistanceMeters = CachedGameInstance->GetDistanceTraveled();
	const int32 HighScore = CachedGameInstance->GetHighScore();
	const int32 LivesUsed = CachedGameInstance->GetMaxLives() - CachedGameInstance->GetCurrentLives();

	GameOverWidget->SetupGameOverDisplay(bWon, FinalScore, DistanceMeters, HighScore, LivesUsed);

	// Add to viewport (high Z-order to overlay everything)
	GameOverWidget->AddToViewport(100);

	// Store weak reference
	ActiveGameOverWidget = GameOverWidget;

	// CRITICAL FIX: Pause the game to prevent level teardown
	UGameplayStatics::SetGamePaused(this, true);
	UE_LOG(LogSideRunner, Log, TEXT("SideRunnerGameMode: Game paused for game over screen"));

	// Switch to UI input mode and show cursor
	SetInputModeUI();

	UE_LOG(LogSideRunner, Log, TEXT("SideRunnerGameMode: Game Over screen displayed - Won: %s, Score: %d, Distance: %.1fm"),
		bWon ? TEXT("Yes") : TEXT("No"), FinalScore, DistanceMeters);
}

void ASideRunnerGameMode::HideGameHUD()
{
	if (ActiveHUDWidget.IsValid())
	{
		ActiveHUDWidget->RemoveFromParent();
		UE_LOG(LogSideRunner, Log, TEXT("SideRunnerGameMode: GameHUD hidden"));
	}
}

void ASideRunnerGameMode::OnGameWonHandler()
{
	UE_LOG(LogSideRunner, Log, TEXT("SideRunnerGameMode: OnGameWon delegate fired"));
	ShowGameOverScreen(true);
}

void ASideRunnerGameMode::OnGameLostHandler()
{
	UE_LOG(LogSideRunner, Log, TEXT("SideRunnerGameMode: OnGameLost delegate fired"));
	ShowGameOverScreen(false);
}

void ASideRunnerGameMode::SetInputModeUI()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (IsValid(PC))
	{
		FInputModeUIOnly InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = true;

		UE_LOG(LogSideRunner, Verbose, TEXT("SideRunnerGameMode: Input mode set to UI"));
	}
}

void ASideRunnerGameMode::SetInputModeGame()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (IsValid(PC))
	{
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = false;

		UE_LOG(LogSideRunner, Verbose, TEXT("SideRunnerGameMode: Input mode set to Game"));
	}
}
