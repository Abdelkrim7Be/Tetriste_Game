The repo path is: `/home/abdelkrim/Documents/projects/Tetriste`  
The main branch is: `main`  
The remote to update is: `origin`

<div align="center">
  <h1>TETRISTE</h1>
  <p>
    A tactical C++ puzzle game built around circular buffers, chained match resolution, persistent profiles, and a custom SFML terminal-style interface.
  </p>
  <p>
    <a href="LICENSE"><img alt="License: MIT" src="https://img.shields.io/badge/License-MIT-yellow.svg"></a>
    <img alt="C++11" src="https://img.shields.io/badge/C%2B%2B-11-00599C?logo=cplusplus&logoColor=white">
    <img alt="SFML 2.5+" src="https://img.shields.io/badge/SFML-2.5%2B-8CC445">
    <img alt="CMake 3.10+" src="https://img.shields.io/badge/CMake-3.10%2B-064F8C?logo=cmake&logoColor=white">
    <img alt="nlohmann/json" src="https://img.shields.io/badge/nlohmann-json-5E5CFF">
  </p>
</div>

<p align="center">
  <img src="docs/screenshots/06-playing-hud.png" alt="Tetriste gameplay HUD" width="820">
</p>

## Project Overview

Tetriste is a cyberpunk logic puzzle game where the player manages a circular buffer of colored geometric nodes. On each turn, the next node can be inserted on the left or right side of the buffer, or used to trigger a color/shape shift. Matching three or more consecutive nodes by color or shape purges them from the board, increases the score, and can trigger combo chains.

The project is implemented as a complete C++ desktop game prototype with SFML rendering, keyboard input, procedural fallback audio/assets, local player profiles, achievements, settings, and standalone tests for the game logic and persistence layer.

## Screenshots

| Gameplay | Game Over |
| --- | --- |
| <img src="docs/screenshots/06-playing-hud.png" alt="Gameplay HUD" width="420"> | <img src="docs/screenshots/08-game-over.png" alt="Game over screen" width="420"> |

| Login | Main Menu |
| --- | --- |
| <img src="docs/screenshots/01-operator-login.png" alt="Operator login" width="420"> | <img src="docs/screenshots/02-main-menu.png" alt="Main menu" width="420"> |

| Settings | Difficulty |
| --- | --- |
| <img src="docs/screenshots/03-settings.png" alt="Settings screen" width="420"> | <img src="docs/screenshots/05-difficulty-select.png" alt="Difficulty selection" width="420"> |

| Protocols |
| --- |
| <img src="docs/screenshots/04-protocols.png" alt="Protocols screen" width="840"> |

## Highlights

- Custom puzzle mechanic based on a circular linked buffer rather than standard falling-block gameplay.
- Multi-ring node model: each piece belongs to the board order, a same-color ring, and a same-shape ring.
- Match detection for consecutive color or shape runs with purge resolution and combo scoring.
- Virtual match preview logic that can inspect a possible insertion without mutating the live board.
- Keyboard-driven SFML UI with login, avatar selection, menu, difficulty selection, settings, protocols/help, pause, achievements, and game-over states.
- Local JSON persistence for users, records, session stats, achievements, avatars, and volume.
- Procedural fallback generation for piece textures and simple audio tones when runtime assets need to be prepared.
- Standalone C++ tests for core gameplay behavior and user persistence.

## Requirements

- C++11-compatible compiler
- CMake 3.10 or newer
- SFML 2.5 or newer with `graphics`, `window`, `audio`, and `system`
- Linux, Windows, or another desktop environment supported by SFML

Ubuntu / Debian:

```bash
sudo apt update
sudo apt install build-essential cmake libsfml-dev
```

## Quick Start

The recommended path is CMake because it copies `assets/` next to the executable after the main game target is built.

```bash
git clone https://github.com/Abdelkrim7Be/Tetriste.git
cd Tetriste
mkdir -p build
cd build
cmake ..
cmake --build . -j
./Tetriste
```

Run the game from `build/` so the copied runtime assets are found at `build/assets/`.

## Build, Run, And Test

### CMake Build

```bash
mkdir -p build
cd build
cmake ..
cmake --build . -j
./Tetriste
```

### Make Build

```bash
make
./Tetriste
```

The Makefile is a lightweight alternative. It builds the executable in the repository root and expects the root-level `assets/` directory to be available.

### Tests

```bash
mkdir -p build
cd build
cmake ..
cmake --build . --target test_logic test_userManager
cd ..
./build/test_logic
./build/test_userManager
```

The test binaries are run from the repository root because the persistence tests write temporary files under `assets/`.

## Usage Workflow

1. Launch the game.
2. Enter an operator name.
3. Create or verify a 4-digit local PIN.
4. Select an avatar for a new profile.
5. Choose a difficulty: Recruit, Veteran, or Elite.
6. Insert nodes with `J` / `A` for left and `K` / `D` for right.
7. Use `C` and `S` to rotate attributes through color and shape rings.
8. Create runs of three or more matching colors or shapes to purge nodes.
9. Keep the circular buffer from reaching its capacity.
10. Review records, stats, achievements, and settings from the menu.

### Controls

| Key | Action |
| --- | --- |
| `J` / `A` | Insert the next node on the left |
| `K` / `D` | Insert the next node on the right |
| `C` | Shift the color ring matching the next node |
| `S` | Shift the shape ring matching the next node |
| `P` | Open the protocols/help overlay |
| `Esc` | Pause or return |
| `F11` | Toggle between full-size and 1280x720 window |
| `+` / `-` | Adjust audio volume |

### Difficulty

Difficulty changes the board capacity calculated from the current playfield size.

| Mode | Capacity | Purpose |
| --- | ---: | --- |
| Recruit | 100% | Learning the mechanics |
| Veteran | 80% | Standard pressure |
| Elite | 60% | Smaller buffer and faster failure pressure |

## Architecture

Tetriste is split into three main layers:

| Layer | Files | Responsibility |
| --- | --- | --- |
| Core | `src/core/` | Piece model, circular buffer, insertion, shifting, matching, scoring |
| Platform | `src/platform/` | Asset loading, generated audio buffers, config persistence, user profiles |
| UI | `src/ui/` | SFML drawing, screens, HUD, visual effects, layout constants |

`src/main.cpp` wires those layers together. It creates the SFML window, resolves assets, initializes managers, owns the application state machine, handles keyboard input, forwards gameplay actions into `Game`, and asks `Renderer` to draw the active state.

### Core Data Model

Each `Piece` stores:

- `nextPiece` for circular board traversal
- `colorPrev` / `colorNext` for nodes with the same color
- `shapePrev` / `shapeNext` for nodes with the same shape
- `T_Color` and `T_Shape` enum values

`Game` owns pieces with `std::vector<std::unique_ptr<Piece>>`. The circular links are non-owning raw pointers, while the `Game` object remains the ownership boundary.

### Match And Shift Logic

`Game::updateGame` scans the circular board for consecutive runs, purges matches of three or more, updates affected ring pointers, removes owned nodes, and repeats while cascades continue. `globalComboMultiplier` and run size determine score growth.

`Game::colorShifting` rotates shapes through the ring of pieces sharing a color. `Game::shapeShifting` rotates colors through the ring of pieces sharing a shape. After each shift, the affected auxiliary rings are rebuilt so future shifts and match checks stay consistent.

### Persistence

`UserManager` uses `assets/users.json` for local profile data. It supports current JSON profile objects and migration from an older `scores.txt` format. `AssetManager` uses `assets/config.json` for volume settings.

## Repository Layout

```text
Tetriste/
|-- assets/
|   |-- config.json
|   |-- users.json
|   |-- fonts/
|   `-- *.png
|-- docs/
|   `-- screenshots/
|-- include/
|   `-- nlohmann/json.hpp
|-- src/
|   |-- core/
|   |-- platform/
|   |-- ui/
|   `-- main.cpp
|-- tests/
|   |-- test_logic.cpp
|   `-- test_userManager.cpp
|-- CMakeLists.txt
|-- Makefile
|-- LICENSE
`-- README.md
```

## Configuration Notes

- `assets/config.json` stores the current volume value.
- `assets/users.json` stores local profiles and may contain migrated legacy records.
- CMake copies `assets/` into `build/assets/` after building `Tetriste`.
- The executable searches `assets`, `build/assets`, and `../assets` so it can run from common development locations.
- PIN storage is intentionally simple in this prototype and should be hashed before treating it as production authentication.

## Testing And Verification

The current tests cover:

- circular list consistency across board, color, and shape relationships
- head and tail insertion behavior
- color and shape shifting
- match detection and purge scoring
- profile creation and PIN verification
- leaderboard ordering
- session statistics persistence
- legacy score migration

Recommended local verification before pushing changes:

```bash
mkdir -p build
cd build
cmake ..
cmake --build . -j
cd ..
./build/test_logic
./build/test_userManager
```

## Future Improvements

- Add a GitHub Actions workflow for build and test validation.
- Replace plain local PIN storage with hashed credentials.
- Add release packaging for Linux and Windows.
- Move standalone tests into a structured test framework such as Catch2 or GoogleTest.
- Add deterministic replay tests for gameplay sessions.
- Add configurable key bindings.
- Add static analysis and sanitizer builds for pointer-heavy game logic.

## License

Tetriste is released under the [MIT License](LICENSE).
