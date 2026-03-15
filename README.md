# Nyality (fork of Catalyst)
- This is a CS2 software intended to get advantage over other players. 
- This may VAC-Live you, if you will setting it wrong, so be sure your config is safe to use.

> [!CAUTION]  
> - The executable changes it own name every time you launch it to avoid bans/associations with original name. Be sure to know that to not get worried.
> - The project was heavily modified under AI Agents with Anthropic Claude Opus 4.6. (CLAUDE.MD)
> - Be sure to star a original [project](https://github.com/nul1ex/catalyst), thanks!

### All edits below shows what was added into fork.
- Text/description of most features was made by AI to not waste time.

## Config System
- **Save/Load configs** - Named binary config files saved to `~/configs/`
- **Default config** - Mark a config as default to auto-load on startup
- **Reset to defaults** - Reset all settings back to code defaults
- **Delete configs** - Remove saved configs from disk

## Combat
- **Penetration crosshair** - 3D crosshair projected on surfaces showing whether your weapon can penetrate through walls (green = can penetrate, red = cannot); rendered as a quad on the surface with gradient shading; configurable per weapon group with separate colors for penetrate/no-penetrate states
- **Penetration calculation** (`penetration::can()`) - Wall penetration check using BVH ray tracing with full segment analysis, surface type handling, and damage falloff calculation
- **Humanization** - Just works, i guess?? Intended to be used against VAC-Live to make your moves more real, not sure if this works, be cautious.

## ESP
- **Combo box fix** - Fixed ESP combo dropdowns (box style, weapon display) not applying selections; the overlay stored a pointer to a stack-local that died each frame; now uses static locals with bidirectional sync

## Misc

### Watermark
- **Draggable watermark** - Drag the watermark anywhere on screen while the menu is open (2 styles)
- **Custom text** - Set custom watermark text from the menu
- **FPS display** - Shows actual in-game FPS (read from game memory, not overlay FPS)
- **Accent color line** - Watermark box has an accent-colored line matching the menu accent
- **Resolution change detection** - Watermark position resets when display resolution changes

### Movement
- **Bunny hop** - Auto bunny hop with configurable key
- **Quick stop** - Counter-strafes when you release movement keys to stop sliding; injects opposite direction inputs and releases them when velocity drops below threshold or after 150ms timeout

### Crosshair
- **Crosshair** - Just a crosshair perfectly centered, also have a option to be hidden while you don't have any sniper-gun (AWP/SSG)
- **Hitmarker** - Shows your hits at enemy, can be customized

## Indicators
- **Key Display** - Shows what buttons do you press (WASD/Space), can drag anywhere
- **Speed** - Shows speed of your player, ported from movement cheats, size can be edited

## Menu
- **Resizable menu** - Drag any corner to resize the menu; size is saved in config
- **Accent color picker** - Change the menu accent color from the misc tab
- **Fade-in/Scale animation** - Menu fades in smoothly after initialization
- **Anti-screenshare** - Hides all overlays and windows related to Nyality from OBS/Discord/NVIDIA/etc apps

### How to build?

## Building
- Download a [Visual Studio 2026](https://visualstudio.microsoft.com/thank-you-downloading-visual-studio/?sku=Community&channel=Stable&version=VS18), choose "Desktop development with C++"
- Open project using a catalyst.slnx file
- Press F7 or build through a button. (don't forget to choose a release/x64)
- After successful build, go to your folder and open "bin" folder, nyality.exe will be yours executable.

## Video showcase
[![showcase](https://img.youtube.com/vi/Ki2GGvMMMvs/maxresdefault.jpg)](https://www.youtube.com/watch?v=Ki2GGvMMMvs)
