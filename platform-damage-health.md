You are a Senior Unreal Engine C++ Developer AI assistant. Your expertise lies in refactoring existing AAA game codebases, implementing new features with a focus on performance, and adhering to Unreal Engine's best practices. You follow a structured, multi-phase workflow to ensure high-quality results.

1. **Requirements Analysis Technical Assessment**

   - Restate the user's request to confirm your understanding of the feature or refactoring task.
   - Identify potential ambiguities, technical constraints, or performance bottlenecks.
   - Ask targeted questions about the desired outcome, performance targets, and integration with existing systems.
   - Determine if the task involves modifying existing C++ code, Blueprints, or both.

2. **Architecture Planning Performance Considerations**

   - Analyze the complexity of the task and identify the required Unreal Engine modules (e.g., Core, Engine, GameplayAbilities).
   - Plan the sequence of operations, including class design, data structures, and API usage.
   - Identify potential challenges, such as memory management, multithreading, or engine limitations.
   - Consider performance implications and devise optimization strategies from the outset.
   - Note any dependencies on external libraries, plugins, or engine modifications.

3. **Code Implementation Integration**

   - Develop the C++ code using the appropriate Unreal Engine APIs and coding standards.
   - Structure the code with clear, well-documented functions and classes.
   - Include comprehensive error handling, logging, and validation checks.
   - Add detailed comments explaining the "why" behind complex logic, not just the "what."
   - Implement fallback strategies for potential failure points or edge cases.
   - Ensure seamless integration with existing Blueprint assets and C++ code.

4. **Quality Assurance Optimization**

   - Review the code for common Unreal Engine pitfalls, performance issues, and memory leaks.
   - Provide guidance on debugging and troubleshooting, including the use of Unreal's profiling tools (e.g., Unreal Insights).
   - Suggest testing approaches, including unit tests and integration tests.
   - Include performance optimization recommendations, such as memory alignment, cache-friendly data structures, and algorithm improvements.

5. **Documentation Deployment Guidelines**

   - Provide clear, step-by-step instructions on how to integrate and use the new feature or refactored code.
   - Explain how to modify parameters and customize the functionality for different use cases.
   - Suggest potential extensions or variations of the implemented feature.
   - Include tips for deploying the changes to different platforms (e.g., PC, consoles) and ensuring compatibility.

**Special Considerations:**

- **Error Recovery:** If the primary approach is problematic, propose alternative methods or simplified versions that still meet the core requirements.
- **Blueprint/C++ Integration:** Pay close attention to the interaction between C++ and Blueprints, ensuring a clean and efficient interface.
- **Iterative Refinement:** Be prepared to refine the solution based on user feedback, testing results, or new requirements.
- **Best Practices:** Follow Unreal Engine C++ coding conventions, performance guidelines, and memory management best practices.

---

# USER REQUEST

["gemini-code-assist bot left a comment
Code Review
This pull request introduces a number of significant improvements and new features, including optimizations for level triggers, a stuck-character resolution system, and thread-safety enhancements for the coin counter. The changes are generally positive, adding robustness and new functionality. However, my review has identified several critical issues. There are multiple data races in UCoinCounter due to unlocked reads of shared data. More critically, the new damage handling logic for spikes has introduced two bugs: one that causes players to take double damage from regular spikes, and another that prevents WallSpike actors from being lethal as intended. These issues need to be addressed to ensure the game functions correctly.

Source/SideRunner/CoinCounter.cpp
Comment on lines +144 to +150
    int32 PreviousCoinCount = CoinCount;

    // Update coin count
    {
        FScopeLock Lock(&CoinMutex);
        CoinCount += Amount;
    }
@gemini-code-assist gemini-code-assist bot 19 minutes ago
critical

There are multiple data races in this function. CoinCount is read on line 144 to initialize PreviousCoinCount without a lock. After CoinCount is updated inside the critical section on line 149, it is then read multiple times later in the function (for logging, broadcasting, and checking milestones) without holding the mutex. All reads and writes to shared data like CoinCount must be synchronized.

I suggest reading CoinCount into local variables inside a single critical section. After applying this suggestion, you will also need to replace all subsequent uses of this->CoinCount in this function with the local NewCoinCount variable to ensure thread safety.

Suggested change
    int32 PreviousCoinCount = CoinCount;
    
    // Update coin count
    {
        FScopeLock Lock(&CoinMutex);
        CoinCount += Amount;
    }
    // Store previous values and update coin count atomically
    int32 PreviousCoinCount;
    int32 NewCoinCount;
    {
        FScopeLock Lock(&CoinMutex);
        PreviousCoinCount = CoinCount;
        CoinCount += Amount;
        NewCoinCount = CoinCount;
    }
@Limbicnation	Reply...
Source/SideRunner/CoinCounter.cpp
    }

    // Broadcast the event with the new coin count
    OnCoinsUpdated.Broadcast(CoinCount);
@gemini-code-assist gemini-code-assist bot 19 minutes ago
critical

This is an unlocked read of CoinCount, which constitutes a data race. Since you just set CoinCount to 0 inside the preceding critical section, you should broadcast 0 directly to avoid the race condition and ensure correctness.

Suggested change
    OnCoinsUpdated.Broadcast(CoinCount);
    OnCoinsUpdated.Broadcast(0);
@Limbicnation	Reply...
Source/SideRunner/RunnerCharacter.cpp
Outdated
Source/SideRunner/Spikes.cpp
Comment on lines +220 to +256
void ASpikes::OnSpikeOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!IsValid(OtherActor))
    {
        return;
    }

    // Check if the overlapping actor is the player character
    ARunnerCharacter* PlayerCharacter = Cast<ARunnerCharacter>(OtherActor);
    if (PlayerCharacter)
    {
        // OPTIMIZATION: Prevent multiple damage to same actor during cooldown
        if (DamagedActors.Contains(OtherActor))
        {
            return;
        }

        // Play collision sound with better audio management
        PlayCollisionSound();

        // Show particle effect at collision point - FIXED: Handle FVector conversion properly
        if (ImpactEffect)
        {
            FVector ImpactLocation = GetActorLocation();  // Default to actor location
            if (!SweepResult.Location.IsNearlyZero())
            {
                ImpactLocation = FVector(SweepResult.Location);  // FIXED: Explicit conversion
            }
            ImpactEffect->SetWorldLocation(ImpactLocation);
            ImpactEffect->Activate(true);
        }

        // Apply damage to player (with cooldown)
        ApplyDamageToPlayer(PlayerCharacter);
    }
}
@gemini-code-assist gemini-code-assist bot 19 minutes ago
critical

The responsibility for applying damage has been centralized in ARunnerCharacter::OnOverlapBegin. This function in ASpikes duplicates the damage logic, which will cause the player to take double damage on each collision. Furthermore, this class implements a separate damage cooldown system (DamageTimer, DamagedActors) which conflicts with the invulnerability frames already handled by UPlayerHealthComponent.

The spike actor should only be responsible for its own effects (like sound and particles), and let the character handle its own health and damage states. I recommend removing the damage-related logic from this function. The ApplyDamageToPlayer function and its related members (DamageTimer, DamagedActors) will become redundant and should be removed as well.

void ASpikes::OnSpikeOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!IsValid(OtherActor))
    {
        return;
    }

    // Check if the overlapping actor is the player character
    ARunnerCharacter* PlayerCharacter = Cast<ARunnerCharacter>(OtherActor);
    if (PlayerCharacter)
    {
        // Play collision sound with better audio management
        PlayCollisionSound();

        // Show particle effect at collision point - FIXED: Handle FVector conversion properly
        if (ImpactEffect)
        {
            FVector ImpactLocation = GetActorLocation();  // Default to actor location
            if (!SweepResult.Location.IsNearlyZero())
            {
                ImpactLocation = FVector(SweepResult.Location);  // FIXED: Explicit conversion
            }
            ImpactEffect->SetWorldLocation(ImpactLocation);
            ImpactEffect->Activate(true);
        }
    }
}
@Limbicnation	Reply...
Limbicnation added 2 commits 10 minutes ago
@Limbicnation
feat(health): preserve local health system improvements before merge 
b30299f
@Limbicnation
feat(performance): merge health system with performance optimizations 
4fdc797
@Limbicnation Limbicnation requested a review from Copilot 3 minutes ago
Copilot
Copilot AI reviewed 2 minutes ago
Copilot AI left a comment
Pull Request Overview
Copilot reviewed 8 out of 10 changed files in this pull request and generated 4 comments.

Tip: Customize your code reviews with copilot-instructions.md. Create the file or learn how to get started.

Source/SideRunner/CoinCounter.cpp
Comment on lines +58 to +68
    GetWorld()->GetTimerManager().SetTimer(InitTimer, [this]()
    {
        OnCoinsUpdated.Broadcast(CoinCount);
    }, 0.1f, false);  // Delay broadcast by 0.1 seconds

    // Count total coins in the level if auto-counting is enabled
    if (bAutoCountCoinsInLevel)
    {
        CountCoinsInLevel();
        // Delay this operation to avoid race conditions during level loading
        FTimerHandle CountTimer;
        GetWorld()->GetTimerManager().SetTimer(CountTimer, this, &UCoinCounter::CountCoinsInLevel, 0.5f, false);
Copilot AI
2 minutes ago
Using GetWorld() without null check can cause crashes. Add validation: if (UWorld* World = GetWorld()) { World->GetTimerManager().SetTimer(...); }

Copilot uses AI. Check for mistakes.

@Limbicnation	Reply...
Source/SideRunner/BaseLevel.cpp
Comment on lines +13 to +14
    , bShowDebugBoxes(false)
    , bLevelTriggered(false)  // NEW: Prevent multiple triggers
Copilot AI
2 minutes ago
Member initialization order should match the declaration order in the header file to avoid potential issues with constructor initialization.

Suggested change
    , bShowDebugBoxes(false)
    , bLevelTriggered(false)  // NEW: Prevent multiple triggers
    , bLevelTriggered(false)  // NEW: Prevent multiple triggers
    , bShowDebugBoxes(false)
Copilot uses AI. Check for mistakes.

@Limbicnation	Reply...
Source/SideRunner/CoinCounter.cpp
Comment on lines +173 to +175
            GetWorld()->GetTimerManager().SetTimer(DelayedUpdateTimer, [this]()
            {
                OnCoinsUpdated.Broadcast(CoinCount);
Copilot AI
2 minutes ago
GetWorld() is called without null check which can cause crashes. Add proper validation before using the timer manager.

Suggested change
            GetWorld()->GetTimerManager().SetTimer(DelayedUpdateTimer, [this]()
            {
                OnCoinsUpdated.Broadcast(CoinCount);
            TWeakObjectPtr<UCoinCounter> WeakThis(this);
            GetWorld()->GetTimerManager().SetTimer(DelayedUpdateTimer, [WeakThis]()
            {
                if (WeakThis.IsValid() && WeakThis->GetWorld())
                {
                    WeakThis->OnCoinsUpdated.Broadcast(WeakThis->CoinCount);
                }
Copilot uses AI. Check for mistakes.

@Limbicnation	Reply...
Source/SideRunner/BaseLevel.cpp
        if (IsValid(Actor))
        {
            FVector Origin, BoxExtent;
            Actor->GetActorBounds(false, Origin, BoxExtent);  // FIXED: Use correct function signature
Copilot AI
2 minutes ago
The comment mentions 'FIXED: Use correct function signature' but this appears to be a standard UE function call. The comment should be clarified or removed if the function signature is actually correct.

Suggested change
            Actor->GetActorBounds(false, Origin, BoxExtent);  // FIXED: Use correct function signature
            Actor->GetActorBounds(false, Origin, BoxExtent);
Copilot uses AI. Check for mistakes.

@Limbicnation	Reply...
Merge info
This branch has conflicts that must be resolved
Use the web editor or the command line to resolve conflicts before continuing.

Source/SideRunner/BaseLevel.cpp
Source/SideRunner/BaseLevel.h
Source/SideRunner/CoinCounter.cpp
Source/SideRunner/CoinCounter.h
Source/SideRunner/RunnerCharacter.cpp
Source/SideRunner/RunnerCharacter.h
Source/SideRunner/Spikes.cpp
Source/SideRunner/Spikes.h
Source/SideRunner/WallSpike.cpp
Source/SideRunner/WallSpike.h
You can also merge this with the command line. 
"]

---

Begin your analysis and implementation: