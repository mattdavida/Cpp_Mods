# UE4SS C++ Mod Development

This repository contains C++ mods for Unreal Engine games using the UE4SS (Unreal Engine Scripting System) framework.

## 🚀 **Quick Setup**

### Prerequisites
1. **Download UE4SS** from [RE-UE4SS GitHub Releases](https://github.com/UE4SS-RE/RE-UE4SS/releases)
2. **Extract** the UE4SS files to `RE-UE4SS/` in this project root
3. **Install** Visual Studio 2022 with C++ development tools
4. **Install** xmake build system

### Project Structure
```
Cpp_Mods/
├── RE-UE4SS/              # ⚠️ Install UE4SS framework here
├── WuChangMod/            # 🎯 Advanced ImGui-based mod with Lua integration
├── StellarBladeMod/       # ⭐ Comprehensive rewards & fast travel system
├── MortalShellMod/        # ⚔️ Professional dual-tab enhancement suite
├── NewModExample/         # 📝 Clean C++ template for new mods
├── build-sln.ps1          # 🔧 PowerShell script to generate VS solution
├── xmake.lua              # 🏗️ Main build configuration
├── .gitignore             # 📝 Git exclusion rules
├── Binaries/              # 🚫 Build output (git ignored)
├── Intermediates/         # 🚫 Build cache (git ignored)
├── vsxmake2022/           # 🚫 Generated VS solution (git ignored)
└── README.md              # 📖 This file
```

### Building Mods
```bash
# Build all mods
xmake

# Build specific mod
xmake build WuChangeMod
```

### Generating Visual Studio Solution
```powershell
# Easy way - run the PowerShell script
.\build-sln.ps1

# Manual way
xmake project -k vsxmake2022
```

## 📁 **Mods in this Repository**

### `WuChangMod`
Advanced ImGui-based cheat mod featuring:
- ✅ ImGui user interface with collapsible sections
- ✅ Lua script integration and execution
- ✅ Player cheats (one-hit kills, money, skill points)
- ✅ Item unlocks (gems, tools, equipment, enchants)
- ✅ Save game manipulation capabilities
- ✅ Clean helper functions for mod interaction

### `StellarBladeMod`
Comprehensive Stellar Blade cheat mod featuring:
- ✅ ImGui-based rewards system (currency, materials, items)
- ✅ Fast travel to all 89 game locations
- ✅ Shop access from anywhere (16 different shops)
- ✅ Trophy/achievement management
- ✅ Player cheats (infinite jump, god mode, skill points)
- ✅ Lua script integration for enhanced functionality
- ✅ Complete rewards database (7,610+ items)

### `MortalShellMod`
Professional dual-tab enhancement suite for Mortal Shell featuring:
- ✅ **Dual-Tab Architecture** - Weapons & Player Mods cleanly separated
- ✅ **Real-Time Status Dashboard** - Live ON/OFF indicators for all features
- ✅ **Instant Weapon Switching** - All 6 weapons accessible anywhere (bypass weapon rack)
- ✅ **Combat Enhancements** - Super Balistazooka, stone form mastery, enhanced parry
- ✅ **Movement & Progression** - 5x walk speed, unlimited stamina, inventory maxing
- ✅ **Visual Feedback System** - 3-second contextual notifications under each button
- ✅ **Complete Lua Integration** - C++ UI with Lua backend for game modifications
- ✅ **Backwards Compatible** - All F1-F9 legacy hotkeys preserved

### `NewModExample` 
Clean C++ starter template featuring:
- ✅ Proper UE4SS integration
- ✅ State management and initialization
- ✅ Key input handling (F9 trigger)
- ✅ Object discovery examples
- ✅ Professional logging with UE4SS Output system
- ✅ Game-specific object finding (PlayerController examples)

## 🛠 **Creating New Mods**

1. **Copy** `NewModExample/` folder to `YourModName/`
2. **Rename** in `YourModName/xmake.lua`:
   ```lua
   local projectName = "YourModName"  -- Change this
   ```
3. **Rename** class and update metadata in `dllmain.cpp`:
   ```cpp
   class YourModNameMod : public RC::CppUserModBase  // Change class name
   {
       ModName = STR("YourModName");        // Update these
       ModDescription = STR("Your description");
       ModAuthors = STR("Your name");
   ```
4. **Add to root** `xmake.lua`:
   ```lua
   includes("RE-UE4SS")
   includes("WuChangMod")
   includes("StellarBladeMod") 
   includes("YourModName")  -- Add this line
   ```
5. **Regenerate** Visual Studio solution:
   ```bash
   xmake project -k vsxmake2022
   ```
6. **Open** `vsxmake2022/cpp_mods.sln` in Visual Studio
7. **Start coding** your mod functionality!

## 📚 **Useful Resources**

- [UE4SS Documentation](https://docs.ue4ss.com/)
- [UE4SS C++ API Reference](https://docs.ue4ss.com/dev/cpp-api.html)
- [Unreal Engine Documentation](https://docs.unrealengine.com/)

## ⚠️ **Important Notes**

- **Never commit** the `RE-UE4SS/` folder - it's a dependency
- **Always test** mods in development builds first
- **Use proper logging** instead of `printf()` for debugging
- **Follow UE4SS patterns** shown in the example mods

## 🤝 **Contributing**

When sharing mods:
1. Only commit your mod source code
2. Include clear setup instructions
3. Test on multiple game versions if applicable
4. Document any game-specific requirements