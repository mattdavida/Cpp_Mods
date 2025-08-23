# MortalShellMod - Professional C++ Enhancement Suite

**Complete gameplay enhancement with dual-tab interface, real-time status indicators, and comprehensive modding features for Mortal Shell**

[![C++](https://img.shields.io/badge/language-C%2B%2B-blue.svg)](https://isocpp.org/)
[![UE4SS](https://img.shields.io/badge/framework-UE4SS-green.svg)](https://github.com/UE4SS-RE/RE-UE4SS)
[![Game](https://img.shields.io/badge/game-Mortal%20Shell-darkred.svg)](https://mortalshell.com/)
[![ImGui](https://img.shields.io/badge/UI-ImGui-orange.svg)](https://github.com/ocornut/imgui)

*Professional dual-tab interface with real-time status indicators and organized collapsible sections*

## 🎯 Overview

MortalShellMod is a **professional C++ UE4SS modification** for Mortal Shell, featuring a sophisticated dual-tab interface with real-time visual feedback. This represents the evolution of Mortal Shell modding from Lua scripts to a polished, production-quality enhancement suite.

Built using modern C++ patterns with ImGui integration, this mod provides comprehensive gameplay enhancements through an intuitive interface that rivals commercial software in terms of user experience and visual polish.

Perfect for players who want professional-grade modding tools with immediate visual feedback, organized feature access, and seamless integration with the game's UI system.

## ✨ Key Features

### 🖥️ Professional Interface System
- **Dual-Tab Architecture** - Weapons Mod & Player Mods cleanly separated
- **Real-Time Status Dashboard** - Live ON/OFF indicators for all toggle features
- **Collapsible Sections** - Organized feature grouping (One-Time Actions, Toggle Features, Advanced Features)
- **Contextual Notifications** - 3-second fade alerts appear directly under each button
- **Comprehensive Keybind Display** - Complete keyboard shortcut reference
- **Smart Visual Feedback** - Color-coded status (Green=ON, Gray=OFF)

### ⚔️ Weapons Tab - Combat Arsenal
- **Instant Weapon Switching** - Bypass weapon rack & fish chests completely
- **6 Complete Weapons** - All Mortal Shell weapons available anywhere
- **Super Balistazooka** - Toggle-based machine gun mode with status indicator
- **Bolt Management** - Max out bolts with notification feedback
- **Keybind Integration** - 3-0 keys for immediate weapon access
- **Status Tracking** - Visual indicators for all weapon features

### 🏃‍♂️ Player Mods Tab - Character Enhancement  
- **Stone Form Mastery** - No cooldown + super form with massive duration
- **Combat Enhancement** - Enhanced parry, god mode, unlimited resolve
- **Movement Systems** - 5x walk speed, unlimited stamina
- **Progression Tools** - Inventory maxing, shell unlocking, player functions
- **Advanced Features** - Fast travel, inventory exploits, system settings
- **Master Toggle Support** - F7 activates multiple features via Lua backend

### 🎮 User Experience Excellence
- **Backwards Compatibility** - All F1-F9 hotkeys work as expected
- **Visual Status Monitoring** - Always know what features are active
- **Professional Organization** - Logical feature grouping and clear descriptions
- **Immediate Feedback** - Every action provides contextual notification
- **Error-Resistant Design** - Graceful handling of edge cases

## 🚀 Quick Start

### Prerequisites
- **[UE4SS for Unreal Engine 4](https://github.com/UE4SS-RE/RE-UE4SS)** - Install the latest UE4SS release
- Mortal Shell (Steam/Epic Games/GOG)
- Visual Studio 2019+ or compatible C++ compiler (for building from source)

### Installation
1. **Install UE4SS** in your Mortal Shell game directory
   - Download latest UE4SS release
   - Extract to `Mortal Shell/Binaries/Win64/`
   - Ensure `UE4SS.dll` is in the Win64 folder
2. **Install MortalShellMod**
   - Download the compiled mod from releases OR build from source
   - Place `MortalShellMod.dll` in `Mortal Shell/Binaries/Win64/ue4ss/Mods/MortalShellMod/`
   - Place `main.lua` (Lua backend) in the same directory
3. **Configure UE4SS**
   - Add `MortalShellMod: 1` to your `mods.txt` file
   - Ensure GUI console is enabled in `UE4SS-settings.ini`
4. **Launch the game** - Mod interface will appear in UE4SS debug console

### Basic Usage
```
Open UE4SS GUI
Navigate to: "Weapons Mod" or "Player Mods" tabs
Use organized sections: Expand/collapse features as needed
Monitor status: Real-time indicators show active features
```

## 📋 Interface Overview

### Weapons Mod Tab
```
Weapons
├── "Bypass weapon rack & fish chests - instant weapon swap anywhere!"
├── Hallowed Sword       [5]
├── Smoldering Mace      [6] 
├── Hammer and Chisel    [7]
├── Martyr's Blade       [8]
├── Axatana Twin Blades  [9]
└── Axatana Axe          [0]

Balistazooka
├── [ON/OFF] Super Balistazooka Status
├── Toggle Super Balistazooka [4]
├── Max out Bolts [3]
└── ✓ Bolts Refreshed! (3-second notification)

Keybinds
├── "Keybind status: [ON/OFF]"
├── Toggle keybinds
└── Complete key mapping display
```

### Player Mods Tab
```
Status Dashboard (Always Visible)
├── Walk Fast: OFF | Super Stamina: OFF | Super Stone: OFF
└── Enhanced Parry: OFF | God Mode: OFF | Matt's Notes: OFF

One-Time Actions (Collapsible)
├── "Instant effects - use once per session"  
├── Stone Form No Cooldown
├── Max Resolve
├── Max Out Inventory
└── Unlock All Shells

Toggle Features (Collapsible)  
├── "On/Off switches - can be toggled multiple times"
├── Toggle Walk Fast
├── Toggle Super Stamina
├── Toggle Super Stone Form
├── Toggle Enhanced Parry
└── Toggle God Mode

Advanced Features (Collapsible)
├── "Powerful features and system settings"
├── Enable Matt's Notes
├── Always Allow Fast Travel
├── Unlock All Player Funcs
└── Exploit Inventory & Fast Travel

Player Keybinds (Collapsible)
├── "Keyboard shortcuts are always enabled"
├── One-Time Actions: F1, F5, F9, NUM_LOCK, BACKSPACE
├── Toggle Features: F2, F3, F4, F6, F8
├── Advanced Features: F7, HOME, PAGE_DOWN, CAPS_LOCK
└── System: PAGE_UP
```

## 🎯 Features Reference

### Weapons Tab Capabilities
| Feature | Key | Description | Status Indicator |
|---------|-----|-------------|------------------|
| **Hallowed Sword** | 5 | Instant weapon swap to starting weapon | ⚔️ |
| **Smoldering Mace** | 6 | Hadern's weapon without boss fight | 🔥 |
| **Hammer and Chisel** | 7 | Crucix's weapon without shell | 🔨 |
| **Martyr's Blade** | 8 | Tarsus's weapon without shell | ⚔️ |
| **Axatana Twin Blades** | 9 | Imrod's twin blade form | 🗡️🗡️ |
| **Axatana Axe** | 0 | Imrod's axe form | 🪓 |
| **Super Balistazooka** | 4 | Toggle machine gun mode | ✅ Visual ON/OFF |
| **Max Bolts** | 3 | 999 bolts instantly | ✓ Notification |

### Player Mods Tab Capabilities  
| Category | Features | Hotkeys | Visual Feedback |
|----------|----------|---------|-----------------|
| **One-Time Actions** | Stone Form Cooldown, Max Resolve, Max Inventory, Unlock Shells | F1, F5, F9, NUM_LOCK, BACKSPACE | ✓ Success notifications |
| **Toggle Features** | Walk Fast, Super Stamina, Super Stone Form, Enhanced Parry, God Mode | F2, F3, F4, F6, F8 | ✅ Live ON/OFF status |
| **Advanced Features** | Matt's Notes, Fast Travel, Player Functions, Inventory Exploits | F7, HOME, PAGE_DOWN, CAPS_LOCK | ✓ Activation confirmations |
| **Master Toggle** | All Matt's Mods (F1,F3,F4,F5,F6 + extras) | F7 | ⚠️ Multiple state updates |

## 🏗️ Technical Architecture

### C++ Core Implementation
```cpp
class MortalShellMod : public RC::CppUserModBase
{
private:
    // Visual indicators state
    bool m_super_balistazooka_enabled = false;
    bool m_walk_fast_enabled = false;
    bool m_super_stamina_enabled = false;
    // ... other state variables
    
    // Individual notification system
    struct ButtonNotification {
        bool show = false;
        float timer = 0.0f;
        std::string text = "";
        const float duration = 3.0f;
    };
    
public:
    // Dual-tab registration
    register_tab(STR("Weapons Mod"), [](CppUserModBase* instance) {
        auto mod = dynamic_cast<MortalShellMod*>(instance);
        if (mod) mod->RenderWeaponModUI();
    });
    
    register_tab(STR("Player Mods"), [](CppUserModBase* instance) {
        auto mod = dynamic_cast<MortalShellMod*>(instance);
        if (mod) mod->RenderPlayerModUI();
    });
};
```

### Lua Integration Backend
```cpp
void CallLuaFunction(const std::string& function_name)
{
    auto* lua_mod = UE4SSProgram::find_lua_mod_by_name(STR("MortalShellMod"));
    if (lua_mod)
    {
        std::string lua_call = std::format("{}()", function_name);
        lua_mod->lua().execute_string(lua_call);
    }
}
```

### Real-Time Status System
```cpp
// Status indicators for toggles
ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.2f, 1.0f), "Toggle Status:");

// Line 1: Walk Fast | Super Stamina | Super Stone Form
ImGui::TextColored(m_walk_fast_enabled ? ImVec4(0.0f, 1.0f, 0.0f, 1.0f) : ImVec4(0.7f, 0.7f, 0.7f, 1.0f), 
    "Walk Fast: %s", m_walk_fast_enabled ? "ON" : "OFF");
```

### Notification System
```cpp
void ShowButtonNotification(ButtonNotification& notif, const std::string& message)
{
    notif.text = message;
    notif.show = true;
    notif.timer = notif.duration;
}

void RenderButtonNotification(ButtonNotification& notif)
{
    if (notif.show && notif.timer > 0.0f)
    {
        float alpha = notif.timer / notif.duration;
        ImGui::TextColored(ImVec4(0.0f, 0.8f, 0.0f, alpha), notif.text.c_str());
        notif.timer -= ImGui::GetIO().DeltaTime;
    }
}
```

### Project Structure
```
MortalShellMod/
├── dllmain.cpp                 # Main C++ implementation (590+ lines)
├── xmake.lua                   # Build configuration
├── lua_scripts/
│   └── main.lua                # Lua backend integration
└── README.md                   # This documentation
```

## 🎨 Design Philosophy

### Modding Best Practices
- **UE4SS Ecosystem Friendly** - Uses standard tab registration
- **Backwards Compatible** - All legacy hotkeys preserved
- **Future Proof** - Designed for easy extension and modification
- **Community Focused** - Code patterns suitable for other modders to learn from

## ⚠️ Important Notes

### State Management
- **C++ manages UI state** - Visual indicators and notifications
- **Lua manages game state** - Actual game modifications and hooks
- **Automatic synchronization** - C++ updates based on user actions

### Performance Considerations
- **Efficient rendering** - Only active notifications consume CPU time
- **Memory conscious** - Minimal overhead for inactive features
- **Thread safe** - Proper synchronization with UE4SS main thread

### Backwards Compatibility
```
✅ ALL LEGACY HOTKEYS WORK:
• F1-F9: Core player modifications
• 3-0: Weapon switching (moved to new interface)
• Special keys: PAGE_UP, NUM_LOCK, HOME, etc.
• F7: Master toggle still activates multiple features
```

- **Code Modularization** - Split into header/implementation files

## ⚖️ License

This project is shared freely for educational and personal use. The code demonstrates advanced game modding techniques and serves as a foundation for professional-quality UE4SS mod development.

**Dependencies:**
- **UE4SS**: Open source UE4 scripting system  
- **ImGui**: Integrated via UE4SS for interface rendering
- **Lua Backend**: Custom game modification implementation

---

**Built with ❤️ and professional C++ practices for the Mortal Shell modding community**

*This mod represents the evolution from script-based modifications to production-quality enhancement suites. Perfect for learning advanced UE4SS development patterns and modern C++ game modding techniques.*
