# UE4SS Utils - Battle-Tested Utility Library

## Overview
This utility library abstracts the most reliable patterns discovered through extensive UE4SS modding across multiple games (MGS Delta, Stellar Blade, Mortal Shell, ENDER MAGNOLIA, etc.). 

## Why This Library Exists

### The Problem
UE4SS modding involves many complex patterns that are easy to get wrong:
- Property access can crash if done incorrectly
- Function calls often fail without proper setup
- Object searching is unreliable with naive approaches
- Console commands need deferred execution to prevent crashes

### The Solution
Through dozens of hours of trial and error across multiple games, we've discovered the **exact patterns** that work reliably. This library abstracts those patterns into reusable utilities.

## Key Patterns Abstracted

### 1. Live View Property Access
**Why:** UE4SS's Live View is the most stable way to inspect UE objects. We replicate its exact methodology.

**How it works:**
```cpp
// WRONG (unreliable):
auto value = object->GetValuePtrByPropertyName(STR("PropertyName"));

// RIGHT (Live View method):
auto result = UE4SSUtils::GetProperty(object, "PropertyName");
if (result.Found) {
    // Use result.DataPtr safely
}
```

**The Live View Method:**
1. Get object's UClass: `object->GetClassPrivate()`
2. Iterate properties: `objectClass->ForEachProperty()`
3. Compare names: `to_string(property->GetName()) == targetName`
4. Check inheritance: `objectClass->ForEachSuperStruct()`
5. Access data: `property->ContainerPtrToValuePtr<T>(object)`

### 2. Deferred Console Execution
**Why:** Immediate `ProcessConsoleExec` calls often crash. Games expect commands on the main thread.

**How it works:**
```cpp
// WRONG (crashes frequently):
object->ProcessConsoleExec(command, outputDevice, executor);

// RIGHT (deferred pattern):
UE4SSUtils::ExecuteConsoleCommand("SetForceInvisible true", playerObject);
```

**The Deferred Pattern:**
1. Queue command for later execution
2. Register ProcessEvent hook
3. Execute on game thread via hook callback
4. Properly manage FUNC_Exec flags

### 3. Robust Object Discovery
**Why:** Direct `FindFirstOf` calls often return invalid objects or crash.

**How it works:**
```cpp
// WRONG (can return invalid objects):
auto player = UObjectGlobals::FindFirstOf(STR("PlayerClass"));

// RIGHT (validated search):
auto player = UE4SSUtils::FindPlayer("GameSpecificPlayerClass");
```

### 4. Safe TArray Access
**Why:** UE TArrays have specific memory layouts that must be respected.

**How it works:**
```cpp
// Access TArray safely
auto weaponsArray = UE4SSUtils::GetTArray<UObject*>(equipController, "m_currentWeapons");
if (weaponsArray && weaponsArray->ArrayNum > 0) {
    // Safely access weaponsArray->Data[i]
}
```

## Usage Examples

### Basic Setup
```cpp
#include "UE4SSUtils.hpp"

// In your mod's initialization:
UE4SSUtils::Initialize();
```

### Find and Inspect Player
```cpp
// Find player
auto player = UE4SSUtils::FindPlayer("AGsrPlayer"); // MGS-specific class
if (!player) {
    player = UE4SSUtils::FindPlayer(); // Try common patterns
}

// Log all player properties for debugging
UE4SSUtils::LogObjectInfo(player, true, 20);

// Get specific property
auto healthResult = UE4SSUtils::GetProperty(player, "Health");
if (healthResult.Found) {
    auto healthValue = *static_cast<float*>(healthResult.DataPtr);
    // Use healthValue...
}
```

### Execute Console Commands
```cpp
// Toggle invisibility
UE4SSUtils::ExecuteConsoleCommand("SetForceInvisible true", player);

// Change weapon
UE4SSUtils::ExecuteConsoleCommand("OnChangeWeapon 7", player);

// Set item
UE4SSUtils::ExecuteConsoleCommand("SetCurrentItemId 23", equipController);
```

### Access Complex Properties
```cpp
// Find equipment controller
UE4SSUtils::ObjectSearchConfig config;
config.ClassName = "BP_GsrEquipController_C";
config.NameContains = "BP_Player_C";
auto controllers = UE4SSUtils::FindObjects(config);

if (!controllers.empty()) {
    auto equipController = controllers[0];
    
    // Access weapons array
    auto weaponsArray = UE4SSUtils::GetTArray<UObject*>(equipController, "m_currentWeapons");
    if (weaponsArray) {
        for (int i = 0; i < weaponsArray->ArrayNum; i++) {
            auto weapon = weaponsArray->Data[i];
            if (UE4SSUtils::IsValidObject(weapon)) {
                // Process weapon...
            }
        }
    }
}
```

## Game-Specific Notes

### MGS Delta (Gsr prefix)
- Player class: `AGsrPlayer`
- Equipment controller: `UGsrEquipController`
- Weapons array: `m_currentWeapons`
- Console commands work reliably

### Stellar Blade (SB prefix)
- Uses CheatManager pattern
- Reward system via console commands
- Shop opening via deferred execution

### ENDER MAGNOLIA (EM prefix)
- Fast travel system via TArray manipulation
- Save state management
- Complex nested property access

## Implementation Notes

### Extraction Lessons Learned
During the extraction of these patterns from working mods, several key insights emerged:

1. **Keep it Simple**: UE4SS's `FProperty::GetName()` returns `StringType` (std::wstring), not `FName`. No complex conversions needed.
2. **Compilation Unit Isolation**: Code that works in main mod files may need adjustments when moved to separate compilation units.
3. **Type Assumptions**: Always verify return types rather than assuming. `auto` can hide important type information during refactoring.
4. **Incremental Testing**: Build and test after each small change when extracting working code.

### Key Implementation Details
```cpp
// Simple and correct property name comparison
auto propName = property->GetName();           // Returns StringType (std::wstring)
if (to_string(propName) == targetPropertyName) // Convert to std::string for comparison
{
    // Property found
}
```

## Troubleshooting

### Common Issues
1. **"Property not found"** - Use `GetAllProperties()` to see available properties
2. **"Function not found"** - Check object type and inheritance chain
3. **Crashes on command execution** - Ensure using deferred pattern
4. **Invalid objects** - Always use `IsValidObject()` validation

### Debugging Tips
```cpp
// Log everything about an object
UE4SSUtils::LogObjectInfo(mysterySbject, true);

// Find objects by pattern
UE4SSUtils::ObjectSearchConfig config;
config.ClassName = "SomeClass";
config.CustomFilter = [](UObject* obj) {
    return obj->GetFullName().find(L"Player") != std::string::npos;
};
auto objects = UE4SSUtils::FindObjects(config);
```

## Benefits of This Approach

1. **Reliability** - These patterns work across multiple games
2. **Consistency** - Same API regardless of underlying complexity
3. **Safety** - Built-in validation prevents crashes
4. **Debuggability** - Comprehensive logging and inspection tools
5. **Maintainability** - Centralized patterns, easier to update
6. **Reusability** - Works across your entire mod collection

## Integration with Existing Mods

### Minimal Changes Required
Replace direct UE4SS calls with utility calls:

```cpp
// Before:
auto player = UObjectGlobals::FindFirstOf(STR("AGsrPlayer"));
auto weaponsPtr = player->GetValuePtrByPropertyName(STR("m_currentWeapons"));

// After:
auto player = UE4SSUtils::FindPlayer("AGsrPlayer");
auto weaponsArray = UE4SSUtils::GetTArray<UObject*>(player, "m_currentWeapons");
```

## Future Enhancements

Potential additions as patterns emerge:
- Widget/UI manipulation utilities
- Save game manipulation helpers
- Animation and effect utilities
- Networking/multiplayer patterns
- Configuration management

---

*This library represents 32+ hours of research and testing across multiple UE games, plus additional hours of careful extraction and simplification. The patterns here have been battle-tested and proven reliable in production mods. The extraction process revealed important insights about UE4SS compilation unit isolation and the importance of keeping patterns simple and well-tested.*
