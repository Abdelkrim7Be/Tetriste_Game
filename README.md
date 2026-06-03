# Tetriste Game - Project Instructions

This document provides foundational mandates, architectural guidance, and development workflows for the Tetriste Game project.

## Project Overview
Tetriste is a terminal-based (and SFML-enhanced) game that utilizes complex linked list logic. The core mechanic involves managing pieces in a circular doubly-linked list with additional cross-links for colors and shapes.

## Architectural Mandates

### 1. Data Structures
- **Circular Linked List:** The primary collection of pieces is a circular linked list managed by the `Game` class.
- **Multi-threaded Lists:** Each `Piece` belongs to three distinct circular lists simultaneously:
  - `nextPiece`: The main sequence of pieces.
  - `shapePrev`/`shapeNext`: A list of all pieces sharing the same shape.
  - `colorPrev`/`colorNext`: A list of all pieces sharing the same color.
- **Memory Management:** The project uses modern memory management (`std::unique_ptr`). Ownership is handled by the `Game` class's `ownedPieces` collection.

### 2. Core Logic
- `updateGame`: Responsible for detecting matches and removing pieces. It correctly updates all three list "threads" when a piece is removed. Match detection is dynamic and supports any board size.
- `colorShifting` and `shapeShifting`: Implement cyclic shifts based on the next piece's attributes.

### 3. User & State Management
- **UserManager:** Handles user profiles and high scores. Data is stored in `assets/users.json` in a structured JSON format. It automatically migrates from legacy `.txt` files and supports a password-less "Local Profile" system.
- **Config Management:** `AssetManager` persists game settings (like volume) in `assets/config.json`.
- **GameState Machine:** The application flow is controlled by a state machine with the following states:
  - `MENU`: Main entry point with Play, Rules, and Exit options.
  - `PLAYING`: The active game state.
  - `PAUSED`: Game logic is suspended; accessible via the `Escape` key.
  - `GAME_OVER`: Triggered when the board reaches dynamic capacity.

### 4. Rendering & Assets
- **Dynamic Capacity:** The number of pieces allowed on board is calculated based on the current window size (`dynamicCapacity`).
- **AssetManager:** Loads and manages textures, fonts, and sound buffers. It can generate placeholder images if assets are missing.

## Engineering Standards

### Coding Style
- **Naming:** CamelCase for classes and methods (e.g., `updateGame`, `Piece`).
- **Headers:** Use `#ifndef` guards in all header files.
- **Portability:** Use portability guards for platform-specific headers. SFML is the primary graphics/audio library.

### Build System
- **CMake:** Use CMake for building. This ensures cross-platform compatibility and easy IDE integration.
- **Dependencies:** SFML (Graphics, Window, Audio, System), nlohmann/json.
- **Commands:**
  ```bash
  mkdir build && cd build
  cmake ..
  make
  ./Tetriste
  ```

## Development Workflows

### Bug Fixes
- **Verification:** Always reproduce the bug with a test case before applying a fix.
- **Regression Testing:** Verify linked list integrity (`piecesCount` consistency) after any logic changes.

### Feature Implementation
- **UI Changes:** Any new UI elements must be integrated into the `Renderer` class and respect the current `GameState`.
- **Logic Changes:** Ensure that any modification to piece placement or removal maintains the triple-linked structure.

## Known Constraints
- **Asset Management:** Automatically generates placeholder `.png` files in `assets/` if they are missing.
- **Sound:** Relies on SFML Audio. Volume persists in `config.json`.
- **Refactored Loop:** `main.cpp` uses modular helper functions (`handlePlayingInput`, `initializeNewGame`) for readability.

## Version History (Highlights)
- **v1.4:** 
  - Migrated build system to **CMake**.
  - Refactored `main.cpp` into modular helper functions for Clean Code.
  - Integrated `nlohmann/json` for robust user and config data persistence.
  - Refactored `updateGame` for dynamic piece limits using `std::vector`.
  - Modernized memory management with `std::unique_ptr` ownership in `Game`.
  - Implemented keyboard-based volume controls (+/-) with UI popups.
  - Enhanced About/Rules screen with detailed control instructions.
- **v1.3:** Completed Graphical UI Overhaul. Added GameState machine, Main Menu, Pause Menu, and Game Over screens.
- **v1.2:** Strengthened User Management. Implemented password-protected accounts and secure login/registration menu.
- **v1.1:** Fixed `colorShifting` and `shapeShifting` logic for correct cyclic shifts. Added comprehensive tests.
- **v1.0:** Initial stable version with fixed memory leaks and corrected match logic.
