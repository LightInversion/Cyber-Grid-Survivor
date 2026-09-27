# Cyber Grid Survivor

A deterministic, high-performance 2D arcade survival micro-engine built from scratch in modern C++ and SDL2. Engineered without third-party game engines or external asset dependencies, the project demonstrates low-level systems programming, real-time kinematics, custom collision algorithms, and hardware-accelerated procedural rendering.

Developed as a technical submission for the E-Cell SRMIST Technical Domain trial.

---

## Technical Highlights & Architecture

- **Deterministic 60 FPS Fixed Pipeline:** Combines high-resolution frame-time delta calculations (`SDL_GetTicks`) with thread suspension (`SDL_Delay`) and GPU VSync presentation to guarantee deterministic simulation rates and eliminate screen tearing.
- **Dual-Layer Input Architecture:** 
  - *Discrete Queue:* Processes OS-level events, EMP triggers, and game resets via `SDL_PollEvent`.
  - *Continuous Hardware Polling:* Polls raw scancode states every frame via `SDL_GetKeyboardState` for latency-free 8-directional movement and independent twin-stick projectile firing without OS key-repeat stutter.
- **Custom Mathematical Solvers:**
  - *AABB Collision Detection:* Real-time 4-axis bounding box overlap tests ($O(1)$ per entity pair) without external physics engines.
  - *Vector AI Tracking:* Autonomous enemy pursuit using Euclidean normalized direction vectors:
    $$\vec{v} = \frac{\vec{d}}{\Vert{}\vec{d}\Vert{}} \cdot s$$
  - *Trigonometric Particle Simulation:* Generates dynamic radial debris bursts by converting randomized polar velocity vectors to Cartesian space:
    $$v_x = \cos(\theta) \cdot s, \quad v_y = \sin(\theta) \cdot s$$
- **Zero-Asset Procedural Graphics Pipeline:** 
  - Complete elimination of image (`.png`, `.bmp`) and font (`.ttf`) files.
  - 100% GPU-accelerated drawing calls using `SDL_Renderer` primitives.
  - Custom procedural 7-segment digital font engine using bitmask lookup tables (`bool seg[10][7]`) to draw live digital scores and multipliers.
- **Memory Safety & Entity Lifecycle:** Manages hundreds of dynamic bullets, enemies, and particles across contiguous `std::vector` pools, purging expired entities every frame via the C++ Erase-Remove idiom (`std::remove_if`).

---

## Tech Stack

- **Language:** C++17 / C++20 (STL: `<vector>`, `<cmath>`, `<algorithm>`)
- **Graphics & Platform API:** SDL2 (Simple DirectMedia Layer 2.x)
- **Toolchain:** Microsoft Visual Studio 2022 (MSVC x64)
- **Dependencies:** Statically linked against `SDL2.lib`, dynamically loaded via `SDL2.dll`

---

## Game Controls

| Action | Key / Input |
| :--- | :--- |
| **Move** | `W`, `A`, `S`, `D` (8-Directional) |
| **Shoot** | `Arrow Keys` (Independent Twin-Stick) |
| **EMP Screen-Wipe** | `Spacebar` (Consumes full charge) |
| **Restart Game** | `R` (Available on Game Over) |
| **Quit** | `Esc` or Window Close Button |

---

## Building and Running Locally

### Prerequisites
- Visual Studio 2022 with the **Desktop development with C++** workload installed.
- SDL2 Development Libraries for VC++ (`SDL2-devel-2.x.x-VC.zip`).

### Setup Instructions
1. Clone this repository:
   ```bash
   git clone [https://github.com/YOUR_USERNAME/Cyber-Grid-Survivor.git](https://github.com/YOUR_USERNAME/Cyber-Grid-Survivor.git)
   cd Cyber-Grid-Survivor
