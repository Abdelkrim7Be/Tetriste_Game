<div align="center">
  <h1>TETRISTE</h1>
  <p align="center">
    Tactical ring-buffer puzzle game built with C++11 and SFML.
  </p>
  <p align="center">
    <a href="LICENSE"><img alt="License: MIT" src="https://img.shields.io/badge/License-MIT-yellow.svg"></a>
    <img alt="C++11" src="https://img.shields.io/badge/C%2B%2B-11-00599C?logo=cplusplus&logoColor=white">
    <img alt="SFML 2.5+" src="https://img.shields.io/badge/SFML-2.5%2B-8CC445">
    <img alt="CMake" src="https://img.shields.io/badge/CMake-3.10%2B-064F8C?logo=cmake&logoColor=white">
    <img alt="nlohmann json" src="https://img.shields.io/badge/nlohmann-json-5E5CFF">
  </p>
</div>

<p align="center">
  <img src="docs/screenshots/06-playing-hud.png" alt="Tetriste gameplay HUD" width="760">
</p>

Tetriste is a cyberpunk logic puzzle where each piece enters a circular buffer from the head or tail. Create runs of three or more matching colors or shapes, trigger chain purges, and keep the buffer from overflowing.

## Contents

- [Features](#features)
- [Gameplay](#gameplay)
- [Screenshots](#screenshots)
- [Quick Start](#quick-start)
- [Project Structure](#project-structure)
- [Tech Stack](#tech-stack)
- [License](#license)

## Features

- Circular ring-buffer puzzle logic with head and tail insertion.
- Color and shape matching with cascade purges and combo scoring.
- Shape-ring and color-ring shifts for tactical board control.
- Match preview highlighting before committing a move.
- Recruit, Veteran, and Elite difficulty tiers.
- Operator login, avatar selection, menu, settings, protocols, pause, and game-over screens.
- Persistent users, scores, sessions, settings, and audio preferences.

## Gameplay

### Controls

| Key | Action |
| --- | --- |
| `J` / `A` | Insert next node at the head |
| `K` / `D` | Insert next node at the tail |
| `C` | Shift the shape ring |
| `S` | Shift the color ring |
| `P` | Open protocols overlay |
| `Esc` | Pause |
| `+` / `-` | Adjust volume |

### Rules

1. Match three or more consecutive nodes by color or shape.
2. When both color and shape match, the longer run is purged.
3. Purges increase score; cascades increase the combo multiplier.
4. Shape and color shifts can trigger purges without inserting a node.
5. A full buffer with no purge ends the session.

### Difficulty

| Tier | Buffer Capacity | Use Case |
| --- | ---: | --- |
| Recruit | 100% | Learning the system |
| Veteran | 80% | Standard play |
| Elite | 60% | High-pressure sessions |

## Screenshots

| Login | Main Menu |
| --- | --- |
| <img src="docs/screenshots/01-operator-login.png" alt="Operator login" width="390"> | <img src="docs/screenshots/02-main-menu.png" alt="Main menu" width="390"> |

| Protocols | Difficulty |
| --- | --- |
| <img src="docs/screenshots/04-protocols.png" alt="Protocols screen" width="390"> | <img src="docs/screenshots/05-difficulty-select.png" alt="Difficulty selection" width="390"> |

| Settings | Game Over |
| --- | --- |
| <img src="docs/screenshots/03-settings.png" alt="Settings screen" width="390"> | <img src="docs/screenshots/08-game-over.png" alt="Game over screen" width="390"> |

## Quick Start

### Prerequisites

- C++11 compiler
- CMake 3.10+
- SFML 2.5+ with `graphics`, `window`, `audio`, and `system`

Ubuntu / Debian:

```bash
sudo apt install build-essential cmake libsfml-dev
```

### Build And Run

```bash
git clone https://github.com/Abdelkrim7Be/Tetriste.git
cd Tetriste
mkdir -p build
cd build
cmake ..
make -j"$(nproc)"
./Tetriste
```

Run the game from `build/` so runtime assets resolve correctly.

### Tests

```bash
cd build
make test_logic test_userManager
./test_logic
./test_userManager
```

## Project Structure

Runtime assets are copied to `build/assets/` and should be resolved from the build directory.

```text
Tetriste/
├── assets/
├── docs/screenshots/
├── include/nlohmann/
├── src/core/
├── src/platform/
├── src/ui/
└── tests/
```

## Tech Stack

| Technology | Purpose |
| --- | --- |
| C++11 | Core game implementation |
| SFML 2.5+ | Rendering, input, windowing, and audio |
| CMake | Build configuration |
| nlohmann/json | User profile and settings persistence |
| JetBrains Mono | Terminal UI typography |

## License

Tetriste is released under the [MIT License](LICENSE).
