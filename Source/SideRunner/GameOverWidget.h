#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameOverWidget.generated.h"

/**
 * Game Over screen widget for Chroma Runner.
 * Displays final score and offers Restart / Main Menu options.
 * Created by Blueprint to allow widget class assignment in BP_SideRunnerGameMode.
 */
UCLASS(Abstract, Blueprintable)
class SIDERUNNER_API UGameOverWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    /** Called by GameMode to pass in the final score before the widget is shown. */
    UFUNCTION(BlueprintCallable, Category = "UI")
    void SetFinalScore(int32 Score);

protected:
    /** Cached score value for restart button logic. */
    int32 FinalScore = 0;

    // ── Widget Bindings ────────────────────────────────────────────────
    // These use meta=(BindWidget) and require TextBlock/Button widgets
    // named exactly as shown in the Designer (txt_Title, txt_FinalScore,
    // btn_Restart, btn_MainMenu).

    UPROPERTY(meta = (BindWidgetOptional))
    class UTextBlock* txt_Title = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    class UTextBlock* txt_FinalScore = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    class UButton* btn_Restart = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    class UButton* btn_MainMenu = nullptr;

    // ── Native lifecycle ────────────────────────────────────────────────

    virtual void NativeConstruct() override;

    // ── Button handlers ─────────────────────────────────────────────────

    UFUNCTION()
    void OnRestartClicked();

    UFUNCTION()
    void OnMainMenuClicked();
};
