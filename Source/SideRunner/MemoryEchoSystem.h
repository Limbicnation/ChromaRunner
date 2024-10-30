#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MemoryEchoSystem.generated.h"

USTRUCT(BlueprintType)
struct FMemoryData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FRotator Rotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float Timestamp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FString MemoryContent;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SIDERUNNER_API UMemoryEchoSystem : public UActorComponent
{
    GENERATED_BODY()

public:    
    UMemoryEchoSystem();

protected:
    virtual void BeginPlay() override;

public:    
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Memory collection functions
    UFUNCTION(BlueprintCallable, Category = "Memory")
    void RecordMemory(const FString& Content);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void PlaybackMemory(int32 MemoryIndex);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void StopPlayback();

    UFUNCTION(BlueprintPure, Category = "Memory")
    TArray<FMemoryData> GetStoredMemories() const { return StoredMemories; }

    // Update playback function declaration
    void UpdatePlayback(float DeltaTime);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Memory")
    TArray<FMemoryData> StoredMemories;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Memory")
    bool bIsPlayingMemory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    int32 MaxMemories = 10;

private:
    float PlaybackTimer;
    int32 CurrentPlaybackIndex;
};
