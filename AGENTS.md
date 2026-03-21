# ChromaRunner - AI Agent Guide

This file provides comprehensive guidance for AI coding agents working with the ChromaRunner codebase.

## Project Overview

**ChromaRunner** is a 2.5D side-scrolling endless runner platformer built with **Unreal Engine 5.5**. The game features precision platforming mechanics, a health and lives system, procedural level generation, and a complete UMG-based UI system.

### Key Facts
- **Engine**: Unreal Engine 5.5
- **Language**: C++20
- **Project Type**: Single-module game project
- **Primary Genre**: Endless runner platformer
- **Target Platforms**: Windows, Linux
- **License**: MIT

---

## Technology Stack

### Core Engine
- Unreal Engine 5.5 with C++20 standard
- Uses Unreal Build Tool (UBT) for compilation
- Enhanced Input System for controls
- UMG (Unreal Motion Graphics) for UI

### Enabled Plugins
| Plugin | Purpose |
|--------|---------|
| **PaperZD** | 2D animation system for sprite-based characters |
| **HoudiniEngine** | Procedural content generation |
| **ModelingToolsEditorMode** | Level design tools (Editor only) |

### Module Dependencies
```csharp
Public: Core, CoreUObject, Engine, InputCore, UMG, Slate, SlateCore
Private: AudioMixer
```

---

## Project Structure

```
ChromaRunner/
├── Source/SideRunner/           # C++ source code (18 .cpp, 20 .h files)
│   ├── RunnerCharacter.h/cpp    # Player character (movement, animation states)
│   ├── PlayerHealthComponent.h/cpp   # Health/damage system
│   ├── SideRunnerGameInstance.h/cpp  # Persistent game state, scoring, lives
│   ├── SideRunnerGameMode.h/cpp      # Game flow orchestration, UI lifecycle
│   ├── SideRunnerPlayerController.h/cpp  # Debug console commands
│   ├── GameHUDWidget.h/cpp      # In-game HUD (lives, score, distance)
│   ├── GameOverWidget.h/cpp     # Win/loss screen with restart/quit
│   ├── BaseLevel.h/cpp          # Procedural level generation base class
│   ├── SpawnLevel.h/cpp         # Level spawning manager
│   ├── CoinPickup.h/cpp         # Collectible coins
│   ├── CoinCounter.h/cpp        # Coin scoring logic
│   ├── Spikes.h/cpp             # Spike obstacle
│   ├── WallSpike.h/cpp          # Wall-mounted spike obstacle
│   ├── SimpleEnemy.h/cpp        # Basic enemy AI
│   ├── ProceduralLevelBuilder.h/cpp  # Procedural generation system
│   ├── DifficultyScaler.h/cpp   # Dynamic difficulty adjustment
│   ├── HealthBarWidget.h/cpp    # Health bar UI component
│   ├── EndlessRunnerTypes.h     # Shared structs and enums
│   └── ActorPool.h              # Object pooling for performance
├── Content/                     # Game assets (managed by Git LFS)
│   ├── SideRunner/              # Project-specific assets
│   │   ├── Blueprints/          # BP_RunnerCharacter, WBP_GameHUD, etc.
│   │   ├── Materials/           # Rendering materials
│   │   ├── Textures/Sprites/    # Character sprites, HUD elements
│   │   ├── Audio/SFX/           # Sound effects
│   │   └── Input/               # Input Action and Mapping Context
│   ├── Maps/                    # Level_1.umap, TheGame.umap
│   ├── Art/                     # Background art, character sprites
│   ├── Characters/              # Mannequin assets (UE5 template)
│   └── StarterContent/          # UE5 starter assets
├── Config/                      # Configuration files
│   ├── DefaultEngine.ini        # Engine settings
│   ├── DefaultGame.ini          # Game settings
│   └── DefaultInput.ini         # Input bindings
├── Plugins/                     # Plugin directory (mostly gitignored)
└── Documentation/               # Additional documentation
```

---

## Build System

### Build Commands (Makefile)

All builds require Unreal Engine 5.5 to be installed. The Makefile is auto-generated and should NOT be modified.

```bash
# Development builds (most common)
make SideRunner              # Build game (Linux Development)
make SideRunnerEditor        # Build editor (Linux Development)

# Debug builds
make SideRunner-Linux-DebugGame         # Debug game build
make SideRunnerEditor-Linux-DebugGame   # Debug editor build

# Shipping builds
make SideRunner-Linux-Shipping          # Optimized shipping build

# Project configuration
make configure               # Regenerate project files
```

### Windows Build
1. Right-click `SideRunner.uproject` → "Generate Visual Studio project files"
2. Open `SideRunner.sln` in Visual Studio 2022
3. Build Solution (Ctrl+Shift+B)

### Build Configuration
- **Module**: `SideRunner` (defined in `SideRunner.Build.cs`)
- **PCH Usage**: Explicit or Shared PCHs for faster builds
- **Unity Build**: Enabled for Development and Shipping configurations
- **Code Optimization**: InShippingBuildsOnly

---

## Core Architecture

### Game Framework Classes

| Class | Responsibility | Key Features |
|-------|---------------|--------------|
| `ARunnerCharacter` | Player pawn | Double-jump, state machine, health integration |
| `USideRunnerGameInstance` | Persistent state | Score, distance, lives, high score, delegates |
| `ASideRunnerGameMode` | Game rules | UI lifecycle, input mode switching |
| `ASideRunnerPlayerController` | Input routing | Debug console commands (Exec functions) |
| `UPlayerHealthComponent` | Health system | Damage types, invulnerability, death handling |

### Character State Machine
```cpp
UENUM(BlueprintType)
enum class ECharacterState : uint8
{
    Idle,
    Running,
    Jumping,
    Falling,
    DoubleJumping,
    Dead
};
```

### Damage Types
```cpp
UENUM(BlueprintType)
enum class EDamageType : uint8
{
    Spikes,
    EnemyMelee,
    EnemyProjectile,
    EnvironmentalHazard
};
```

### Key Delegates (Event-Driven Architecture)
All major systems use delegates for loose coupling:
- `FOnScoreUpdated` - Score changes
- `FOnDistanceUpdated` - Distance changes
- `FOnLivesUpdated` - Lives changes
- `FOnGameWon` / `FOnGameLost` - Game end conditions
- `FOnHealthChanged` / `FOnPlayerDeath` - Health events

---

## Code Style Guidelines

### Naming Conventions (Unreal Engine Standard)
- **Classes**: `PascalCase` with prefix (`ARunnerCharacter`, `UPlayerHealthComponent`)
- **Variables**: Standard types use regular naming, UE types follow conventions
  - `b` prefix for booleans: `bCanDoubleJump`, `bIsFacingRight`
  - `E` prefix for enums: `ECharacterState`
  - `F` prefix for structs: `FPlatformPlacement`
- **Functions**: `PascalCase` for both declaration and definition
- **Constants**: `UPPER_CASE` or `PascalCase` in namespaces
- **Files**: Match class name exactly (e.g., `RunnerCharacter.h/cpp`)

### Prefix Reference
| Prefix | Use For | Example |
|--------|---------|---------|
| `A` | AActor-derived classes | `ARunnerCharacter` |
| `U` | UObject-derived classes | `UPlayerHealthComponent` |
| `F` | Structs | `FPlatformPlacement` |
| `E` | Enums | `ECharacterState` |
| `I` | Interfaces | (none currently) |
| `T` | Template classes | (rare in this codebase) |

### Comment Style
- Use `//` for single-line comments
- Use `/* */` for multi-line comments
- Document all public UFUNCTIONs with `/** */` doc comments
- Include `@param` and `@return` in function documentation

### Code Organization
```cpp
// Header layout:
#pragma once
#include "CoreMinimal.h"           // Always first
#include "GameFramework/Character.h" // Engine headers
#include "PlayerHealthComponent.h"   // Project headers
#include "RunnerCharacter.generated.h" // Generated header last

// Class declaration:
UCLASS()
class SIDERUNNER_API ARunnerCharacter : public ACharacter
{
    GENERATED_BODY()
public:
    // Constructors first
    ARunnerCharacter();
    
protected:
    // Override functions
    virtual void BeginPlay() override;
    
public:
    // Public interface
    UFUNCTION(BlueprintCallable, Category = "Category")
    void PublicFunction();
    
protected:
    // Protected members
    
private:
    // Private members last
};
```

---

## Testing Strategy

### Manual Testing
The project uses manual testing with debug console commands:

| Command | Description |
|---------|-------------|
| `TeleportToDistance 5000` | Teleport to test win condition |
| `KillPlayer` | Instant death (test lives system) |
| `DebugSetScore 1000` | Set score for testing |
| `DebugAddLives 5` | Add lives for testing |
| `DebugTriggerGameOver` | Force game over |

### Testing Checklist (from Testing_Guide.md)
1. **Win Condition**: `TeleportToDistance 5000` → Game Over screen appears
2. **Loss Condition**: `KillPlayer` ×3 → Game Over screen appears
3. **Restart**: Click RESTART button → Level reloads, stats reset
4. **HUD Updates**: Lives, score, distance display correctly
5. **Input Modes**: Mouse cursor shows/hides appropriately

### Performance Testing
```bash
# In-game console commands
stat fps          # Show FPS counter
stat game         # Game thread time
stat unit         # Overall frame time
stat memory       # Memory usage
```

---

## Git LFS Configuration

This repository uses **Git LFS** for large binary files.

### Tracked File Types
```
*.uasset, *.umap    # Unreal assets and maps
*.png, *.jpg, *.tga # Images
*.wav, *.mp3, *.ogg # Audio
*.fbx, *.obj        # 3D models
*.mp4, *.avi        # Video
```

### Troubleshooting LFS Issues
```bash
# If "repository exceeded its LFS budget":
git lfs install --skip-smudge
git pull

# Restore LFS when budget available:
git lfs install --force
git lfs pull

# Check LFS files:
git lfs ls-files
```

---

## Common Development Workflows

### Adding a New Feature
1. Create header (.h) and source (.cpp) files in `Source/SideRunner/`
2. Follow naming conventions and class prefix rules
3. Add appropriate UPROPERTY and UFUNCTION macros
4. Use delegates for event communication
5. Update Blueprint bindings if UI-related
6. Test with debug console commands
7. Run `make SideRunnerEditor` to compile

### Creating New Widgets (UI)
1. Create C++ class extending `UUserWidget`
2. Add `BindWidgetOptional` UPROPERTYs for UI elements
3. Implement `NativeConstruct()` and `NativeDestruct()`
4. Create Blueprint widget inheriting from C++ class
5. Name widgets exactly matching C++ bindings
6. Implement update functions with `UFUNCTION(BlueprintCallable)`

### Blueprint Integration Points
C++ classes expose these integration points:
- `BlueprintImplementableEvent` - Override in Blueprint
- `BlueprintNativeEvent` - C++ default + Blueprint override
- `BlueprintCallable` - Call from Blueprint
- `BlueprintPure` - Getter with no side effects
- `BlueprintReadOnly/Write` - Property access

---

## Security Considerations

### Debug Commands
- All `UFUNCTION(Exec)` commands are automatically stripped from Shipping builds
- Debug commands are centralized in `ASideRunnerPlayerController`
- Never place sensitive logic in Exec functions

### Input Validation
- All editable UPROPERTYs use `ClampMin`/`ClampMax` where applicable
- Validate pointers with `IsValid()` before dereferencing
- Check `IsFullyInitialized()` for components before use

### Memory Safety
- Use `TWeakObjectPtr` for cached widget references (prevents GC issues)
- Always unbind delegates in `EndPlay()` or destructors
- Use `FTimerHandle` member variables (never stack-allocated)

---

## Debugging Tips

### Logging Categories
```cpp
// Use these instead of LogTemp:
UE_LOG(LogSideRunner, Log, TEXT("Message"));        // General gameplay
UE_LOG(LogSideRunnerScoring, Warning, TEXT("...")); // Scoring system
UE_LOG(LogSideRunnerCombat, Warning, TEXT("..."));  // Combat/damage
```

### Common Issues
1. **Widget binding failures**: Check UMG widget names match C++ exactly (case-sensitive)
2. **Delegate not firing**: Ensure binding happens in `BeginPlay()` and unbinding in `EndPlay()`
3. **Input not working**: Verify Input Mapping Context is active
4. **Memory leaks**: Check delegates are unbound, timers cleared

### VS Code Configuration
The project includes `.vscode/` with:
- `c_cpp_properties.json` - IntelliSense configuration
- `compile_commands.json` - Build commands for clangd

---

## Documentation References

- `README.md` - Project overview and quick start
- `CLAUDE.md` - Build commands and Git LFS troubleshooting
- `Testing_Guide.md` - Comprehensive testing procedures
- `Blueprint_UMG_Setup_Guide.md` - UI widget creation guide
- `C++_Widget_Blueprint_Setup_Guide.md` - C++/Blueprint integration
- `PHASE2_SUMMARY.md` - Health system documentation
- `PHASE3_IMPLEMENTATION_SUMMARY.md` - Lives system documentation

---

## Quick Reference

### Opening the Project
```bash
# Linux
make SideRunnerEditor
./Binaries/Linux/SideRunnerEditor

# Or open in Unreal Editor directly
cd /path/to/UnrealEngine/Engine/Binaries/Linux
./UnrealEditor /mnt/f/UnrealEngine_Projects_2024/GitHub_UE_2024/ChromaRunner/SideRunner.uproject
```

### Controls (In-Game)
- **Spacebar / W**: Jump (press twice for double-jump)
- **A / D**: Limited horizontal movement
- **ESC**: Pause menu
- **` (tilde)**: Debug console (Development builds only)

### Win Condition
Reach 5000 meters ( configurable in `SideRunnerGameInstance` )

### Lives System
- Start with 3 lives
- Lose 1 life per death
- Respawn with full health if lives remain
- Game Over when lives reach 0

---

*Document Version: 1.0*
*Last Updated: 2026-03-18*
*For Unreal Engine 5.5*
