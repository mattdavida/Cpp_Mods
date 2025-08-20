# Stellar Blade Rewards Mod

A comprehensive modding tool for Stellar Blade that provides rewards, cheats, teleportation, and various game modifications through both C++ and Lua scripting.

## Features

### 🎁 Rewards System
- **Quick Rewards**: Drop valuable items instantly
  - Sealed Gold
  - VitCoins (currency)
  - Omnibolt (upgrade materials)
  - Drone Upgrade Modules
  - Tumbler Expansion Modules
- **Custom Rewards**: Input any reward group name manually
- **Skill Points**: Give 9999 skill points instantly

### 📦 Can Collection System
- **Individual Can Unlock**: Select and unlock specific cans (Can_001 to Can_049)
- **Bulk Unlock**: Unlock all 49 cans automatically with 3-second delays
- Progress tracking during bulk operations

### 🏪 Shop Access
- **16 Different Shops**: Access all in-game shops from anywhere
  - Camp Shops (Simple/Full)
  - Character Shops (Kaya, Digger, Clyde, etc.)
  - Special Shops (Hair Salon, Nest Camp, etc.)
- **Currency Information**: Shows required currency for each shop
- **Hair Salon**: Direct access to character customization

### 🗺️ Fast Travel System
- **Hierarchical Camp Selection**: Organized by regions
  - Eidos 7, Eidos 9, Xion City
  - Wasteland, Matrix 11, Great Desert
  - Altess Levoire, Abyss Levoire, Spire 4
  - Nest and Danger Areas
- **Full Camps vs Checkpoints**: Clear distinction between main camps and save points
- **89 Total Locations**: Complete coverage of all game areas

### 🏆 Trophy Management
- **Individual Trophy Unlock**: Select and unlock specific achievements
- **Unlock All Trophies**: Complete all 45 trophies at once
- **Hidden Trophy Support**: Includes story-sensitive achievements
- **Progress Tracking**: Uses proper SBProgressTrophy system

### 🎮 Player Cheats
- **Infinite Jump**: Set jump count to 1000 (reapply after area transitions)
- **God Mode**: Continuous health/energy regeneration every 1 second
  - Health: 9999
  - Beta Gauge: 9999
  - Burst Gauge: 9999
  - Max Tachy Gauge
- **Status Indicators**: Real-time display of active cheats

## Technical Architecture

### C++ Component (`main.dll`)
- **ImGui Interface**: Professional tabbed GUI integration
- **Deferred Execution**: Hook-based command processing for UI operations
- **Thread Safety**: Background processing for bulk operations
- **Memory Management**: Proper CheatManager caching and object handling
- **Error Handling**: Comprehensive exception handling and logging

### Lua Component (`main.lua`)
- **UE4SS Integration**: Uses UEHelpers for player access
- **Game Thread Safety**: All operations executed in proper thread context
- **Interval Functions**: Custom timing system for continuous effects
- **Player Stat Manipulation**: Direct character property modification

## Installation

### Prerequisites

**CRITICAL**: You must use the specific UE4SS build for Stellar Blade compatibility:
- Download **UE4SS Build 3.1.0-3 Experimental (Stellar Blade Demo)** from: [https://github.com/Chrisr0/RE-UE4SS/releases/tag/3.1.0-3](https://github.com/Chrisr0/RE-UE4SS/releases/tag/3.1.0-3)
- This is the **only known working version** for Stellar Blade
- Standard UE4SS releases will **NOT** work with this game

### Setup Steps

1. **Install UE4SS Stellar Blade Build**
   - Download the release from the link above
   - Extract to your Stellar Blade game directory
   - Verify UE4SS loads correctly with the game

2. **Install the Mod**
   - Place the `SBRewardsMod` folder in your UE4SS `Mods` directory
   - Enable the mod in your `mods.txt` file: `SBRewardsMod : 1`

3. **Launch and Verify**
   - Start Stellar Blade
   - Open UE4SS GUI
   - Navigate to "Stellar Blade Rewards" tab

## Usage

### GUI Access
- Using the UE4SS GUI
- Navigate to "Stellar Blade Rewards" tab
- Use collapsible sections to organize features

### Reward Dropping
1. Select desired reward category
2. Click corresponding button
3. Items appear directly in inventory

### Shop Access
1. Choose shop from dropdown
2. Click "Open Selected Shop"
3. Shop opens regardless of location
4. **Hair Salon Note**: Must be called from pause menu

### Fast Travel
1. Select region and camp from hierarchical menu
2. Click "Warp to Camp"
3. Instant teleportation to selected location

### Trophy Unlocking
1. Choose individual trophy or unlock all
2. Progress tracked through Steam/PSN
3. Hidden trophies included for completionists

## Supported Reward Groups

### Complete Reward Database

The mod includes `rewards.json` - a comprehensive mapping of **all 7,610 reward groups** extracted from the game's internal datatable. This file maps reward group names to their corresponding item aliases, providing complete coverage of every obtainable item in Stellar Blade.

**Usage Examples:**
```json
{
    "reward": "e_tumbler_core_test", 
    "itemAlias": "TumblerCore"
},
{
    "reward": "subGroup_b_destruct_ditCoin", 
    "itemAlias": "VitCoin"
},
{
    "reward": "Can_001_RewardGroup", 
    "itemAlias": "Money"
}
```

**What's Included:**
- **Legion Rewards**: All area-specific skill point rewards
- **Collection Items**: Every can, record, and collectible
- **Upgrade Materials**: All enhancement and crafting components  
- **Currency Types**: Every form of in-game money and tokens
- **Equipment**: Weapons, armor, and accessories
- **Consumables**: Potions, grenades, and temporary items
- **Special Items**: Quest items, keys, and story progression items

**For Developers:** Use this database to implement custom reward selection interfaces or to validate reward group names before calling `SBRewardGroupDropInven`.

### Quick Reference Groups
- `b_sealed_gold` - Sealed Gold currency
- `subGroup_b_destruct_ditCoin` - VitCoins
- `b_Xion_Box1` - Omnibolt materials
- `Records_LockedBox_DroneCore` - Drone upgrades
- `e_tumbler_core_test` - Tumbler expansion
- `Can_XXX_RewardGroup` - Collection cans (001-049)

### Shop Currencies
- `BetaCrystal` - Most shops
- `ETC_Item_VendingMachineCoin` - Spotter shop
- `FishingPoint` - Clyde's fishing shop
- `Money_Nier` - Emil's shop
- `Money_Nikke` - Bolt's shop

## Development Notes

### Memory Management
- CheatManager instance cached for performance
- Proper object lifetime management
- Thread-safe operations for UI commands

### Hook System
- ProcessEvent hooks for deferred execution
- Separate hooks for shop vs general commands
- Flag-based execution prevention

### Error Handling
- Graceful degradation on missing objects
- Comprehensive logging system
- Exception safety throughout

## Compatibility

- **Game Version**: Stellar Blade (latest)
- **UE4SS Version**: **REQUIRES** [UE4SS Build 3.1.0-3 Experimental](https://github.com/Chrisr0/RE-UE4SS/releases/tag/3.1.0-3)
  - Standard UE4SS releases are **incompatible** with Stellar Blade
  - This is the only known working build for the game
- **Platform**: PC (Steam)
- **Save Compatibility**: All modifications are non-destructive

## Known Limitations

- Infinite Jump requires reactivation after area transitions or player death
- Hair Salon must be accessed from pause menu
- Some danger areas may cause visual issues (all black environments)
- Trophy unlocking requires proper game state (save file loaded)

## Troubleshooting

### Common Issues
1. **CheatManager not found**: Ensure game is fully loaded
2. **Commands not working**: Check UE4SS console for errors
3. **GUI not appearing**: Verify mod is enabled in mods.txt
4. **Crashes**: Disable other mods to test compatibility

### Debug Information
- Enable verbose logging in UE4SS
- Check mod loading order in `load_order.txt`
- Verify all dependencies are present

## Credits

- Built on UE4SS framework
- Uses ImGui for interface
- Leverages Unreal Engine 4 reflection system
- Compatible with existing UE4SS mod ecosystem

## Version History

- **v1.0**: Initial release with full feature set
- Comprehensive reward system
- Complete fast travel implementation
- Trophy management system
- Player cheat integration

---

**Warning**: This mod is for single-player use only. Use of cheats and modifications may affect game progression and achievement unlock legitimacy. Always backup your save files before using any modifications.
