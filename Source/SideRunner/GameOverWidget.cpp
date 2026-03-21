#include "GameOverWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

void UGameOverWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Bind button clicks
    if (btn_Restart)
    {
        btn_Restart->OnClicked.AddDynamic(this, &UGameOverWidget::OnRestartClicked);
    }

    if (btn_MainMenu)
    {
        btn_MainMenu->OnClicked.AddDynamic(this, &UGameOverWidget::OnMainMenuClicked);
    }
}

void UGameOverWidget::SetFinalScore(int32 Score)
{
    FinalScore = Score;

    if (txt_FinalScore)
    {
        txt_FinalScore->SetText(FText::FormatOrdered(
            FText::FromString(TEXT("SCORE: {0}")),
            FText::AsNumber(Score)
        ));
    }
}

void UGameOverWidget::OnRestartClicked()
{
    // Restart the current level (TheGame)
    UKismetSystemLibrary::ExecuteConsoleCommand(this, TEXT("RestartLevel"), nullptr);

    // Fallback: reopen the same level
    UGameplayStatics::OpenLevel(this, FName(TEXT("TheGame")), false);
}

void UGameOverWidget::OnMainMenuClicked()
{
    // Close this widget first
    RemoveFromParent();

    // Navigate to main menu — replace "MainMenu" with your actual map name
    // If no main menu exists yet, this just restarts:
    UGameplayStatics::OpenLevel(this, FName(TEXT("TheGame")), false);
}
