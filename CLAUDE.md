# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build System

This is a pure MSBuild project — no package manager, no scripts.

**Visual Studio:** Open `catalyst.slnx`, select `Release | x64`, press F7 (Build Solution).

**Command line (MSBuild in PATH):**
```bash
msbuild catalyst/catalyst.vcxproj /p:Configuration=Release /p:Platform=x64
```

Output binary: `bin/nyality.exe`. Intermediates: `bin/intermediates/`.

There is no test suite, no linter, and no formatter configured.

## Architecture

Nyality is a CS2 game overlay: it reads CS2 process memory, builds a world model, runs combat/visual features, and renders a transparent DirectX 11 overlay on top of the game.

**Data flow:** game memory → systems → features → render

**Three concurrent threads:**
- `threads::game()` — updates entity state at ~1000 Hz (local player, entity list, collector, BVH on map change)
- `threads::combat()` — runs aimbot/triggerbot at 128 TPS
- `render::run()` — main thread, DirectX 11 render loop

Thread safety uses `std::shared_mutex` for entity/collector data and `std::atomic<T>` for local player state.

### Core Layers

**`core/systems/`** — pure data extraction from game memory, no logic:
- `entities` enumerates the entity list; `collector` aggregates into player/item/projectile structs
- `local` tracks the local player; `view` gives the camera matrix
- `bones`, `bounds`, `hitboxes` compute skeletal/screen-space data
- `bvh` builds a ray-tracing acceleration structure from map geometry for penetration checks
- `schemas` and `convars` do offset/convar lookups by FNV-1a hash

**`core/features/`** — logic that consumes system data:
- `combat/legit` — aimbot and triggerbot (FOV, smoothing, hitchance, autowall)
- `combat/shared` — penetration, spread, ballistics utilities
- `esp/player`, `esp/item`, `esp/projectile` — overlay rendering per entity type
- `misc/grenades` — grenade trajectory prediction/simulation
- `misc/movement` — bunny hop

**`core/render/`** — DirectX 11 device, swap chain, layered overlay window (`WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED`), render loop.

**`core/menu/`** — 3-tab settings UI (Combat / ESP / Misc) built on zdraw/zui.

### Utilities (`utilities/`)

| Module | Purpose |
|--------|---------|
| `memory/` | `read<T>()`, pattern scanning, RIP resolution on `cs2.exe` |
| `modules/` | Caches base addresses: `client.dll`, `engine2.dll`, `tier0.dll`, `schemasystem.dll`, `vphysics2.dll` |
| `offsets/` | Stores discovered offsets (`entity_list`, `view_matrix`, `global_vars`, etc.) |
| `math/` | `Vector2`, `Vector3`, `Quaternion`, `Matrix` with operators |
| `animation/` | Spring physics for smooth UI animations |
| `input/` | Keyboard/mouse event handling |
| `fnv1a.hpp` | Compile-time/runtime FNV-1a hashing (used for schema/convar lookups) |
| `random.hpp` | Valve RNG seeding for spread calculations |

### Global State

All singletons live in namespace `g::` (defined in `stdafx.hpp`). Settings are in `settings::g_combat`, `settings::g_esp`, `settings::g_misc` (defined in `core/settings.hpp`).

### Rendering (`external/zdraw/`)

Custom D3D11 wrapper. Fonts are embedded as binary HPP data (Inter, Mochi, Pixel7, Weapons icon font) and rasterized with FreeType 2 (`freetype.lib` is pre-built and linked statically).

### String Obfuscation

`external/xorstr.hpp` provides compile-time XOR string obfuscation. Use `xorstr_("...")` for strings that should not appear in plaintext in the binary.

## Entry Point

`entry.cpp` initializes subsystems in order: console → input → memory → modules → offsets → threads → render.
