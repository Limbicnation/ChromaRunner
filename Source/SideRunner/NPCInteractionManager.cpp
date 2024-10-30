// Fill out your copyright notice in the Description page of Project Settings.

#include "NPCInteractionManager.h"

UNPCInteractionManager::UNPCInteractionManager()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UNPCInteractionManager::BeginPlay()
{
    Super::BeginPlay();
}

void UNPCInteractionManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UNPCInteractionManager::InitializeNPC(const FString& NPCName)
{
    if (!NPCDatabase.Contains(NPCName))
    {
        FNPCData NewNPCData;
        NewNPCData.NPCName = NPCName;
        NewNPCData.RelationshipValue = 0.0f;
        NPCDatabase.Add(NPCName, NewNPCData);
    }
}

void UNPCInteractionManager::UpdateRelationship(const FString& NPCName, float ValueChange)
{
    if (IsValidNPC(NPCName))
    {
        FNPCData& NPCData = NPCDatabase[NPCName];
        NPCData.RelationshipValue = FMath::Clamp(NPCData.RelationshipValue + ValueChange, 0.0f, MaxRelationshipValue);
    }
}

void UNPCInteractionManager::AddDialogueEntry(const FString& NPCName, const FString& DialogueText)
{
    if (IsValidNPC(NPCName))
    {
        FNPCData& NPCData = NPCDatabase[NPCName];
        
        // Remove oldest entry if at capacity
        if (NPCData.DialogueHistory.Num() >= MaxDialogueHistory)
        {
            NPCData.DialogueHistory.RemoveAt(0);
        }
        
        NPCData.DialogueHistory.Add(DialogueText);
    }
}

float UNPCInteractionManager::GetRelationshipValue(const FString& NPCName) const
{
    if (IsValidNPC(NPCName))
    {
        return NPCDatabase[NPCName].RelationshipValue;
    }
    return 0.0f;
}

TArray<FString> UNPCInteractionManager::GetDialogueHistory(const FString& NPCName) const
{
    if (IsValidNPC(NPCName))
    {
        return NPCDatabase[NPCName].DialogueHistory;
    }
    return TArray<FString>();
}

bool UNPCInteractionManager::IsValidNPC(const FString& NPCName) const
{
    return NPCDatabase.Contains(NPCName);
}
