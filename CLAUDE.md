# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

ChromaRunner is a 2.5D side-scrolling platformer built with Unreal Engine 5.5. The project implements a character-based runner game with health system, procedural level generation, coin collection mechanics, and obstacle avoidance gameplay.

## Build Commands

### Development Build
```bash
# Build the game for development
make SideRunner

# Build the editor
make SideRunnerEditor

# Build for shipping
make SideRunner-Linux-Shipping
```

### Debug Build
```bash
# Debug build for game
make SideRunner-Linux-DebugGame

# Debug build for editor
make SideRunnerEditor-Linux-DebugGame
```

### Project Configuration
```bash
# Regenerate project files
make configure
```

## Core Architecture

### C++ Module Structure
- **Module Name**: `SideRunner` (defined in `SideRunner.Build.cs`)
- **Target Configuration**: Uses Unreal Engine 5.5 with C++20 standard
- **Primary Dependencies**: Core, CoreUObject, Engine, InputCore

### Key Components

#### Character System (`RunnerCharacter`)
- **Animation State Machine**: Enum-based system with states: Idle, Running, Jumping, Falling, DoubleJumping, Dead
- **Health Integration**: Uses `PlayerHealthComponent` for damage handling
- **Movement**: Side-scrolling with double-jump mechanics
- **Location**: `Source/SideRunner/RunnerCharacter.h/.cpp`

#### Health System (`PlayerHealthComponent`)
- **Damage Types**: Spikes, EnemyMelee, EnemyProjectile, EnvironmentalHazard
- **Features**: Invulnerability frames, damage tracking, delegate-based events
- **Location**: `Source/SideRunner/PlayerHealthComponent.h/.cpp`

#### Level Generation (`BaseLevel`)
- **Procedural System**: Trigger-based level segment spawning
- **Components**: Trigger boxes for player detection, spawn locations for next segments
- **Features**: Level activation/deactivation, difficulty scaling, debug visualization
- **Location**: `Source/SideRunner/BaseLevel.h/.cpp`

#### Game Objects
- **Obstacles**: `Obstacle`, `Spikes`, `WallSpike` classes for hazards
- **Collectibles**: `CoinPickup` with `CoinCounter` for scoring
- **Level Management**: `SpawnLevel` for procedural generation
- **UI**: `HealthBarWidget` for health display

### Blueprint Integration
- **Main Character**: `BP_RunnerCharacter` (extends `ARunnerCharacter`)
- **Game Mode**: `BP_SideRunner_GameMode` 
- **UI Widgets**: `WBP_CoinHUD`, `WBP_HealthBar`
- **Level Blueprints**: `BP_Level1-4`, `BP_SpawnLevel`

### Content Organization
- **Art Assets**: `Content/Art/` - Character sprites, backgrounds, textures
- **Audio**: `Content/Audio/` - Music, SFX with sound cues
- **Blueprints**: `Content/Blueprints/` - Game logic blueprints
- **Materials**: `Content/Materials/`, `Content/SideRunner/Materials/` - Rendering materials
- **Maps**: `Content/Maps/` - Level geometry (`Level_1.umap`, `TheGame.umap`)

### Enabled Plugins
- **PaperZD**: 2D animation system for sprite-based characters
- **HoudiniEngine**: Procedural content generation
- **ModelingToolsEditorMode**: Level design tools

## Development Workflow

### Testing
The project uses Unreal Engine's built-in testing framework. No external test runners are configured.

### Building
Use the provided Makefile commands for all build operations. The Makefile is auto-generated and should not be modified directly.

### Code Standards
- **Language**: C++20 with Unreal Engine 5.5 coding standards
- **Naming**: Unreal's naming conventions (A/U/F/E prefixes)
- **Architecture**: Component-based design with Blueprint integration points