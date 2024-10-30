// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NPCInteractionManager.generated.h"

USTRUCT(BlueprintType)
struct FNPCData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    float RelationshipValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    TArray<FString> DialogueHistory;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SIDERUNNER_API UNPCInteractionManager : public UActorComponent
{
    GENERATED_BODY()

public:    
    UNPCInteractionManager();

protected:
    virtual void BeginPlay() override;

public:    
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // NPC interaction functions
    UFUNCTION(BlueprintCallable, Category = "NPC")
    void InitializeNPC(const FString& NPCName);

    UFUNCTION(BlueprintCallable, Category = "NPC")
    void UpdateRelationship(const FString& NPCName, float ValueChange);

    UFUNCTION(BlueprintCallable, Category = "NPC")
    void AddDialogueEntry(const FString& NPCName, const FString& DialogueText);

    UFUNCTION(BlueprintPure, Category = "NPC")
    float GetRelationshipValue(const FString& NPCName) const;

    UFUNCTION(BlueprintPure, Category = "NPC")
    TArray<FString> GetDialogueHistory(const FString& NPCName) const;

protected:
    // Map to store NPC data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC")
    TMap<FString, FNPCData> NPCDatabase;

    // Maximum relationship value
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    float MaxRelationshipValue = 100.0f;

    // Maximum dialogue history entries per NPC
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    int32 MaxDialogueHistory = 20;

private:
    bool IsValidNPC(const FString& NPCName) const { return NPCDatabase.Contains(NPCName); }
};
