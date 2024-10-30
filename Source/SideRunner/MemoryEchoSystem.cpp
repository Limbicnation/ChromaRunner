// Fill out your copyright notice in the Description page of Project Settings.

#include "MemoryEchoSystem.h"

UMemoryEchoSystem::UMemoryEchoSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    bIsPlayingMemory = false;
    PlaybackTimer = 0.0f;
    CurrentPlaybackIndex = 0;
}

void UMemoryEchoSystem::BeginPlay()
{
    Super::BeginPlay();
}

void UMemoryEchoSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bIsPlayingMemory)
    {
        UpdatePlayback(DeltaTime);
    }
}

void UMemoryEchoSystem::RecordMemory(const FString& Content)
{
    FMemoryData NewMemory;
    NewMemory.Location = GetOwner()->GetActorLocation();
    NewMemory.Rotation = GetOwner()->GetActorRotation();
    NewMemory.Timestamp = GetWorld()->GetTimeSeconds();
    NewMemory.MemoryContent = Content;

    // Add memory to array, removing oldest if at capacity
    if (StoredMemories.Num() >= MaxMemories)
    {
        StoredMemories.RemoveAt(0);
    }
    StoredMemories.Add(NewMemory);
}

void UMemoryEchoSystem::PlaybackMemory(int32 MemoryIndex)
{
    if (StoredMemories.IsValidIndex(MemoryIndex))
    {
        bIsPlayingMemory = true;
        CurrentPlaybackIndex = MemoryIndex;
        PlaybackTimer = 0.0f;

        // Initial playback setup
        FMemoryData& Memory = StoredMemories[CurrentPlaybackIndex];
        GetOwner()->SetActorLocation(Memory.Location);
        GetOwner()->SetActorRotation(Memory.Rotation);
    }
}

void UMemoryEchoSystem::StopPlayback()
{
    bIsPlayingMemory = false;
    PlaybackTimer = 0.0f;
}

void UMemoryEchoSystem::UpdatePlayback(float DeltaTime)
{
    PlaybackTimer += DeltaTime;

    // Add playback logic here
    // This could include interpolation between memory points,
    // triggering visual effects, or other memory replay mechanics
}
