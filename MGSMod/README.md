# MGS Delta Snake Eater Mod

A comprehensive C++ mod for Metal Gear Solid Delta: Snake Eater using UE4SS framework.

## Features

### Save Game Manipulation
Unlock all content and collectibles by directly modifying save data:

#### Camouflage Collection
- **Uniform (Camouflage)** - Unlocks all 66 camouflage uniforms for gameplay use
- **Face (Face Paint)** - Unlocks all face paint options  
- **Item (Collection Viewer)** - Unlocks all items in the camouflage collection viewer

#### Content Unlocks
- **All Rewards** - Unlocks all reward conditions, gained rewards, and displayed rewards
- **Secret Theater** - Unlocks all secret theater videos and dialogs
- **All Kerotan** - Marks all Kerotan frogs as collected
- **All Gako** - Marks all Gako lizards as collected  
- **All Record Titles** - Unlocks all record titles
- **Character Viewer** - Unlocks character viewer in extras menu
- **Guy Savage** - Unlocks Guy Savage mini-game from main menu
- **Demo Theater** - Unlocks all demo theater content
- **European Extreme** - Unlocks European Extreme difficulty

#### Steam Achievements
- **Unlock All Achievements** - Sets all Steam achievements to 100% completion
- ⚠️ **WARNING**: This affects your Steam profile and cannot be undone

### Direct Item Equipping
Quick access buttons for rare stealth items:
- **IT_Stealth (23)** - Equips stealth camouflage
- **IT_ATCamo (26)** - Equips active camouflage

## Installation

### Prerequisites
- **UE4SS** framework installed and working with MGS Delta
- **Visual Studio 2022** with C++ development tools (for building)
- **xmake** build system

### Setup
1. Place the mod files in your UE4SS mods directory:
   ```
   <Game Directory>/Binaries/Win64/Mods/MGSMod/
   ```

2. Build the mod:
   ```bash
   cd MGSMod
   xmake
   ```

3. Enable the mod in UE4SS configuration

## Usage

### Save Game Manipulation
1. **Load your save game** first - the mod works on the currently loaded save
2. **Use the unlock buttons** in the "Save Game Manipulation" section
3. **Trigger an in-game save** after using codes (e.g., enter a new area)
4. **Menu refresh**: If using from main menu, enter a new menu and return to refresh items

### Important Notes
- **Extras Menu**: Unlocks take effect immediately for extras menu content
- **New Game Bonuses**: Camouflage, rewards, and completion bonuses only apply when starting a new game
- **Current Save**: Your current playthrough save data is not affected by these unlocks

### Item Equipping
- Click the equip buttons for instant stealth item access
- Visual feedback shows when items are successfully equipped
- Items equip directly without menu navigation

## Technical Details

### Architecture
- **Base Class**: `CppUserModBase` from UE4SS framework
- **UI Framework**: ImGui for in-game interface
- **Utilities**: Custom `UE4SSUtils` for reliable UE object interaction
- **Memory Approach**: Direct save data manipulation

### Key Components
- **Save Game Objects**: Targets `UserProfileSaveGame` objects in memory
- **Property Access**: Uses UE4SS reflection system for type-safe property manipulation
- **Console Commands**: Deferred execution pattern for reliable item equipping
- **Notification System**: Visual feedback with 3-second fade effects

### Data Structures
- **TArray<bool>**: For simple boolean lists (Kerotan, Gako, etc.)
- **TMap<Enum, Status>**: For unlockable content with acquisition states
- **TArray<FAchievementData>**: For achievement progress tracking
- **Nested Structs**: Advanced property access for reward data

## Troubleshooting

### Common Issues
1. **"Player not found"** - Load a save game first, the mod needs an active game session
2. **Items not appearing** - Trigger an in-game save to persist changes
3. **Menu not refreshing** - Exit and re-enter menus to refresh unlocked content

### Build Issues
- Ensure UE4SS headers are properly linked
- Verify xmake configuration matches your UE4SS installation
- Check that all dependencies are available in your build environment

## Development

### File Structure
```
MGSMod/
├── dllmain.cpp           # Main mod implementation
├── MGS3Types.hpp         # Game-specific type definitions
├── README.md             # This documentation
└── xmake.lua            # Build configuration
```

### Dependencies
- **UE4SS Framework** - Core modding framework
- **UE4SSUtils** - Custom utility library for UE object interaction
- **ImGui** - Immediate mode GUI for mod interface

### Enum Definitions
All game-specific enums are centralized in `MGS3Types.hpp`:
- `ECamouflageType` (66 camouflage types)
- `EGsrExtraAcquiredStatus` (acquisition states)
- `ERewardCondition` / `ECobraRewardItem` (reward system)
- `EFacePaintType` / `EItemName` (cosmetic content)
- `EGsrSecretTheaterType` (secret theater content)
- `FAchievementData` (achievement structure)

## License

This mod is provided as-is for educational and entertainment purposes. Use at your own risk.

## Disclaimer

This mod modifies save game data and Steam achievements. Always backup your save files before use. The developers are not responsible for any data loss or Steam account issues.
