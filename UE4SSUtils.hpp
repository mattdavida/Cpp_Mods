#pragma once

/**
 * UE4SS Utilities - Shared utility class for UE4SS C++ modding
 * 
 * This class abstracts the most reliable patterns discovered through extensive
 * modding work, particularly with MGS Delta and other Unreal Engine games.
 * 
 * Key Patterns Abstracted:
 * 1. Live View Property Access - Replicates UE4SS Live View's property inspection
 * 2. Deferred Function Execution - Safe ProcessConsoleExec with proper hooks
 * 3. Object Discovery - Reliable player/object finding with validation
 * 4. TArray Manipulation - Direct memory access to Unreal Engine arrays
 * 
 * Rationale:
 * - Live View property access is the MOST reliable way to inspect UE objects
 * - Deferred execution prevents crashes and ensures proper game thread execution
 * - Console commands are often more reliable than direct function calls
 * - These patterns work across multiple UE games (MGS, Stellar Blade, Mortal Shell, etc.)
 */

#define NOMINMAX
#include <DynamicOutput/DynamicOutput.hpp>
#include <Unreal/UObjectGlobals.hpp>
#include <Unreal/UObject.hpp>
#include <Unreal/UFunction.hpp>
#include <Unreal/UClass.hpp>
#include <Unreal/FProperty.hpp>
#include <Unreal/UStruct.hpp>
#include <Unreal/FString.hpp>
#include <Unreal/FOutputDevice.hpp>
#include <Unreal/UnrealFlags.hpp>
#include <Unreal/Hooks.hpp>
#include <Unreal/NameTypes.hpp>
#include <Unreal/Property/FStructProperty.hpp>
#include <Unreal/UScriptStruct.hpp>
#include <String/StringType.hpp>
#include <Helpers/String.hpp>
#include <vector>
#include <string>
#include <functional>
#include <unordered_map>

using namespace RC;
using namespace RC::Unreal;

/**
 * UE4SS Utilities Class
 * 
 * Provides battle-tested utilities for UE4SS modding based on patterns
 * that have proven reliable across multiple games and mod projects.
 */
class UE4SSUtils
{
public:
    /**
     * TArray Structure for direct memory access
     * 
     * This mirrors Unreal Engine's internal TArray structure:
     * - Data: Pointer to the actual array elements
     * - ArrayNum: Current number of elements
     * - ArrayMax: Maximum capacity before reallocation
     */
    template<typename T>
    struct TArrayStruct {
        T* Data;
        int32_t ArrayNum;
        int32_t ArrayMax;
    };

    /**
     * Property Access Result
     * Contains both the property metadata and the actual data pointer
     */
    struct PropertyResult {
        FProperty* Property = nullptr;
        void* DataPtr = nullptr;
        bool Found = false;
        std::string PropertyName;
        std::string PropertyType;
    };

    /**
     * Object Search Configuration
     * Allows flexible object searching with various criteria
     */
    struct ObjectSearchConfig {
        std::string ClassName;
        std::string NameContains;
        std::function<bool(UObject*)> CustomFilter;
        bool RequireValid = true;
        int32_t MaxResults = -1; // -1 = unlimited
    };

private:
    // Deferred execution state (like MGS mod pattern)
    struct DeferredExecution {
        bool DoCall = false;
        UObject* Instance = nullptr;
        StringType Command;
        FOutputDevice OutputDevice;
        UFunction* Function = nullptr;
        UObject* Executor = nullptr;
        bool IsHooked = false;
    };

    static DeferredExecution s_DeferredExec;
    
    // Hook callback for deferred execution
    static auto ProcessEventCallback(UObject*, UFunction*, void*) -> void;

public:
    /**
     * Find Player Object
     * 
     * Attempts to find the player using common patterns across UE games.
     * This method tries multiple approaches that have proven reliable:
     * 1. Game-specific player class names
     * 2. Common UE player patterns
     * 3. Fallback to generic searches
     * 
     * @param GameSpecificPlayerClass - Optional game-specific class name (e.g., "AGsrPlayer")
     * @return Player object if found, nullptr otherwise
     */
    static UObject* FindPlayer(const std::string& GameSpecificPlayerClass = "");

    /**
     * Find Objects by Criteria
     * 
     * Flexible object search using Live View patterns.
     * This replicates the object discovery methods used in Live View.
     * 
     * @param Config - Search configuration
     * @return Vector of matching objects
     */
    static std::vector<UObject*> FindObjects(const ObjectSearchConfig& Config);

    /**
     * Get Property Using Live View Method
     * 
     * This exactly replicates how Live View accesses properties:
     * 1. Get the object's UClass
     * 2. Iterate through properties using ForEachProperty()
     * 3. Check super structs for inherited properties
     * 4. Use ContainerPtrToValuePtr for actual data access
     * 
     * This is the MOST reliable way to access UE object properties.
     * 
     * @param Object - Target UObject
     * @param PropertyName - Name of the property to find
     * @return PropertyResult with property info and data pointer
     */
    static PropertyResult GetProperty(UObject* Object, const std::string& PropertyName);

    /**
     * Get All Properties from Object
     * 
     * Returns all properties from an object, including inherited ones.
     * Useful for debugging and discovery. Based on Live View enumeration.
     * 
     * @param Object - Target UObject
     * @param IncludeInherited - Whether to include properties from super structs
     * @return Vector of all property results
     */
    static std::vector<PropertyResult> GetAllProperties(UObject* Object, bool IncludeInherited = true);

    /**
     * Access TArray Property
     * 
     * Safely accesses TArray properties using the Live View method.
     * This provides direct access to UE arrays without crashes.
     * 
     * @param Object - Object containing the array property
     * @param PropertyName - Name of the TArray property
     * @return Pointer to TArrayStruct if successful, nullptr otherwise
     */
    template<typename T>
    static TArrayStruct<T>* GetTArray(UObject* Object, const std::string& PropertyName)
    {
        auto propResult = GetProperty(Object, PropertyName);
        if (!propResult.Found || !propResult.DataPtr) {
            return nullptr;
        }
        
        return static_cast<TArrayStruct<T>*>(propResult.DataPtr);
    }

    /**
     * Execute Console Command with Deferred Pattern
     * 
     * This replicates the deferred execution pattern that has proven most reliable
     * for console commands. It:
     * 1. Sets up the command for deferred execution
     * 2. Registers ProcessEvent hook if needed
     * 3. Executes safely on the game thread
     * 4. Properly manages FUNC_Exec flags
     * 
     * This pattern works consistently across all games tested.
     * 
     * @param Command - Console command to execute
     * @param TargetObject - Object to execute command on
     * @param FunctionName - Optional specific function name (auto-detected if empty)
     * @return True if command was queued successfully
     */
    static bool ExecuteConsoleCommand(const std::string& Command, 
                                    UObject* TargetObject, 
                                    const std::string& FunctionName = "");

    /**
     * Validate Object
     * 
     * Comprehensive object validation using UE4SS patterns.
     * Checks both UE4SS validity and common crash conditions.
     * 
     * @param Object - Object to validate
     * @return True if object is safe to use
     */
    static bool IsValidObject(UObject* Object);

    /**
     * Safe Property Write
     * 
     * Safely writes to object properties with type checking and validation.
     * This prevents many common crashes when modifying UE objects.
     * 
     * @param Object - Target object
     * @param PropertyName - Property to modify
     * @param Value - New value (as void* for type flexibility)
     * @param ValueSize - Size of the value in bytes
     * @return True if write was successful
     */
    static bool SetProperty(UObject* Object, 
                          const std::string& PropertyName, 
                          void* Value, 
                          size_t ValueSize);

    /**
     * Log Object Information
     * 
     * Comprehensive object logging for debugging purposes.
     * Outputs object details in a format similar to Live View.
     * 
     * @param Object - Object to log
     * @param LogProperties - Whether to log all properties
     * @param MaxProperties - Maximum number of properties to log (-1 = all)
     */
    static void LogObjectInfo(UObject* Object, bool LogProperties = false, int32_t MaxProperties = 10);

    /**
     * Convert UE Types to Standard Types
     * 
     * Helper methods for converting between UE and standard C++ types.
     * These handle the common conversions needed in modding.
     */
    // Removed FNameToString - use Property->GetName().ToString() directly
    static std::wstring StringToWString(const std::string& str);
    static std::string WStringToString(const std::wstring& wstr);

    /**
     * Get TMap Property Within Struct Property
     * 
     * Accesses a TMap property that is nested within a struct property.
     * This is common in save game data where structs contain multiple TMaps.
     * 
     * Example:
     *   struct RewardsData { TMap<ERewardType, bool> RewardsGained; }
     *   UObject has: RewardsData RewardsData;
     *   
     * Usage:
     *   auto* mapPtr = GetStructTMapProperty<ERewardType, bool>(saveGame, "RewardsData", "RewardsGained");
     * 
     * @param object The UObject containing the struct property
     * @param structPropertyName Name of the struct property (e.g., "RewardsData")
     * @param tmapPropertyName Name of the TMap within the struct (e.g., "RewardsGained")
     * @return Pointer to the TMap data, or nullptr if not found
     */
    template<typename TMapKeyType, typename TMapValueType>
    static void* GetStructTMapProperty(UObject* object, const std::string& structPropertyName, const std::string& tmapPropertyName)
    {
        try
        {
            // Step 1: Get the struct property (e.g., "RewardsData")
            auto structPropertyResult = GetProperty(object, structPropertyName);
            if (!structPropertyResult.Found || !structPropertyResult.DataPtr)
            {
                Output::send<LogLevel::Warning>(STR("Struct property '{}' not found\n"), to_wstring(structPropertyName));
                return nullptr;
            }
            
            // Step 2: Cast to FStructProperty to access struct definition
            auto structProperty = CastField<FStructProperty>(structPropertyResult.Property);
            if (!structProperty)
            {
                Output::send<LogLevel::Warning>(STR("Property '{}' is not a struct property\n"), to_wstring(structPropertyName));
                return nullptr;
            }
            
            auto structDef = structProperty->GetStruct();
            if (!structDef)
            {
                Output::send<LogLevel::Warning>(STR("Struct definition for '{}' not available\n"), to_wstring(structPropertyName));
                return nullptr;
            }
            
            // Step 3: Find the TMap property within the struct
            FProperty* tmapProperty = nullptr;
            for (FProperty* property : structDef->ForEachProperty())
            {
                auto propName = WStringToString(property->GetName());
                if (propName == tmapPropertyName)
                {
                    tmapProperty = property;
                    break;
                }
            }
            
            if (!tmapProperty)
            {
                Output::send<LogLevel::Warning>(STR("TMap property '{}' not found in struct '{}'\n"), 
                    to_wstring(tmapPropertyName), to_wstring(structPropertyName));
                return nullptr;
            }
            
            // Step 4: Get the actual TMap data pointer
            void* tmapDataPtr = tmapProperty->ContainerPtrToValuePtr<void>(structPropertyResult.DataPtr);
            return tmapDataPtr;
        }
        catch (const std::exception& e)
        {
            Output::send<LogLevel::Error>(STR("Exception in GetStructTMapProperty: {}\n"), to_wstring(e.what()));
            return nullptr;
        }
    }

    /**
     * Initialize Utils System
     * 
     * Must be called once before using other utilities.
     * Sets up the deferred execution system and validates UE4SS state.
     * 
     * @return True if initialization was successful
     */
    static bool Initialize();

    /**
     * Cleanup Utils System
     * 
     * Cleans up resources and unhooks callbacks.
     * Should be called during mod shutdown.
     */
    static void Cleanup();
};
