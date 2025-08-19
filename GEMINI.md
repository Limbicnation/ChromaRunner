# GEMINI Analysis of ChromaRunner

## Project Overview

This is a 2.5D side-scrolling platformer game named ChromaRunner, built with Unreal Engine 5. The project is written in C++ and uses Unreal Engine's build system. The core gameplay revolves around a player character, `RunnerCharacter`, who can run, jump, and double jump through levels. The game features a state machine for character animations, collision detection with obstacles, and a level restart mechanism.

### Key Technologies

*   **Engine:** Unreal Engine 5
*   **Language:** C++ (C++20 standard)
*   **Core Modules:** `Core`, `CoreUObject`, `Engine`, `InputCore`

## Building and Running

### Building

To build the project, you will need to have Unreal Engine 5 installed. You can then generate project files for your IDE (e.g., Visual Studio, Rider) and build the project from there.

### Running

The project can be run from the Unreal Editor by opening the `SideRunner.uproject` file and clicking the "Play" button. Alternatively, you can build the project and run the standalone executable.

## Development Conventions

### Coding Style

The codebase follows the standard Unreal Engine C++ coding conventions. This includes using `PascalCase` for class names and `camelCase` for variables. The code is well-commented and organized into header and source files.

### Project Structure

The project is organized into the following main directories:

*   `Config/`: Contains the project's configuration files (`.ini` files).
*   `Content/`: Contains the game's assets, such as maps, materials, and blueprints.
*   `Source/`: Contains the C++ source code for the game.

### Core Classes

*   `ARunnerCharacter`: The main player character class, responsible for player movement, input, and collision.
*   `ASideRunnerGameMode`: The game mode class, which defines the rules of the game.
*   `ASpawnLevel`: A class responsible for spawning level segments.
*   `AObstacle`, `ASpikes`, `AWallSpike`: Classes representing different types of obstacles in the game.
*   `ACoinPickup`: A class for collectible coins.
*   `UCoinCounter`: A widget for displaying the coin count.
