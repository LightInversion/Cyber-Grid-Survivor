\# Cyber Grid Survivor



A deterministic, high-performance 2D arcade survival shooter and micro-engine built from scratch in C++ and low-level SDL2 primitives for the E-Cell SRMIST Technical Domain trial.



\## Technical Architecture \& Highlights

\- \*\*Deterministic 60 FPS Game Loop:\*\* Frame-delta capped with hardware VSync via SDL2.

\- \*\*Dual-Layer Input Architecture:\*\* Combines discrete event queue polling (`SDL\_PollEvent`) with continuous hardware scancode states (`SDL\_GetKeyboardState`) for zero-latency 8-way WASD movement and independent arrow-key shooting.

\- \*\*Custom Mathematical Solvers:\*\*

&#x20; - Axis-Aligned Bounding Box (AABB) intersection detection without third-party physics libraries.

&#x20; - Autonomous homing AI using Euclidean normalized pursuit vectors ($\\vec{v} = \\frac{\\vec{d}}{\\Vert{}\\vec{d}\\Vert{}} \\cdot s$).

\- \*\*Zero External Assets:\*\* 100% code-driven graphics via GPU hardware primitives (`SDL\_Renderer`), including a custom 7-segment LED digital score engine and dynamic screenshake.



\## Tech Stack

\- \*\*Language:\*\* C++ (STL vectors, algorithm, cmath)

\- \*\*Library:\*\* SDL2 (Simple DirectMedia Layer 2.x)

\- \*\*IDE:\*\* Visual Studio 2022 (x64)



\## Controls

\- `WASD`: 8-Directional Movement

\- `Arrow Keys`: Twin-Stick Directional Shooting

\- `Spacebar`: Detonate EMP Screen-Wipe (when supercharged)

\- `R`: Restart Game on Game Over

\- `Esc`: Quit

