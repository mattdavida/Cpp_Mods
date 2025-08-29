#include "UE4SSUtils.hpp"

using namespace RC;
using namespace RC::Unreal;

// Static member initialization
UE4SSUtils::DeferredExecution UE4SSUtils::s_DeferredExec;

/**
 * ProcessEvent Hook Callback
 * 
 * This is the core of the deferred execution pattern.
 * It safely executes console commands on the game thread,
 * preventing the crashes that occur with immediate execution.
 */
auto UE4SSUtils::ProcessEventCallback(UObject*, UFunction*, void*) -> void
{
    if (s_DeferredExec.DoCall)
    {
        s_DeferredExec.DoCall = false;
        
        if (s_DeferredExec.Function)
        {
            // Set FUNC_Exec flag for console commands (critical for success)
            auto& function_flags = s_DeferredExec.Function->GetFunctionFlags();
            function_flags |= FUNC_Exec;
            
            Output::send(STR("Executing deferred command: {}\n"), s_DeferredExec.Command);
            
            bool success = s_DeferredExec.Instance->ProcessConsoleExec(
                s_DeferredExec.Command.c_str(), 
                s_DeferredExec.OutputDevice, 
                s_DeferredExec.Executor
            );
            
            Output::send(STR("Command result: {}\n"), success ? STR("SUCCESS") : STR("FAILED"));
            
            // Clear FUNC_Exec flag
            function_flags &= ~FUNC_Exec;
        }
    }
}

/**
 * Initialize the Utils System
 * 
 * Sets up the deferred execution hook and validates the UE4SS environment.
 * This must be called before using other utility functions.
 */
bool UE4SSUtils::Initialize()
{
    try
    {
        // Register the ProcessEvent hook for deferred execution
        if (!s_DeferredExec.IsHooked)
        {
            Hook::RegisterProcessEventPostCallback(ProcessEventCallback);
            s_DeferredExec.IsHooked = true;
            Output::send<LogLevel::Verbose>(STR("UE4SSUtils: Deferred execution hook registered\n"));
        }
        
        Output::send<LogLevel::Verbose>(STR("UE4SSUtils: Initialization complete\n"));
        return true;
    }
    catch (...)
    {
        Output::send<LogLevel::Error>(STR("UE4SSUtils: Failed to initialize\n"));
        return false;
    }
}

/**
 * Find Player Object
 * 
 * Uses multiple strategies to find the player, based on patterns that work
 * across different Unreal Engine games. Tries game-specific classes first,
 * then falls back to common patterns.
 */
UObject* UE4SSUtils::FindPlayer(const std::string& GameSpecificPlayerClass)
{
    std::vector<std::string> PlayerClassNames = {
        // Game-specific class (if provided)
        GameSpecificPlayerClass,
        
        // Common UE player patterns (from our experience)
        "BP_Player_C",
        "AGsrPlayer",        // MGS Delta
        "PlayerCharacter",   // Common UE pattern
        "ThirdPersonCharacter", // UE template
        "FirstPersonCharacter", // UE template
        "APlayerCharacter",
        "BP_PlayerCharacter_C",
        
        // Broader patterns
        "ACharacter",
        "APawn"
    };
    
    for (const auto& ClassName : PlayerClassNames)
    {
        if (ClassName.empty()) continue;
        
        try
        {
            UObject* Player = UObjectGlobals::FindFirstOf(to_wstring(ClassName));
            if (Player && IsValidObject(Player))
            {
                Output::send<LogLevel::Verbose>(STR("Found player: {} - {}\n"), 
                    to_wstring(ClassName), Player->GetFullName());
                return Player;
            }
        }
        catch (...)
        {
            // Continue to next class name
            continue;
        }
    }
    
    Output::send<LogLevel::Warning>(STR("Player not found with any known class pattern\n"));
    return nullptr;
}

/**
 * Find Objects by Criteria
 * 
 * Flexible object search that replicates Live View's object discovery.
 * This is much more reliable than direct UObjectGlobals calls.
 */
std::vector<UObject*> UE4SSUtils::FindObjects(const ObjectSearchConfig& Config)
{
    std::vector<UObject*> Results;
    
    try
    {
        if (!Config.ClassName.empty())
        {
            std::vector<UObject*> ClassObjects;
            UObjectGlobals::FindAllOf(to_wstring(Config.ClassName), ClassObjects);
            
            for (UObject* Object : ClassObjects)
            {
                if (!Object) continue;
                
                // Validate object if required
                if (Config.RequireValid && !IsValidObject(Object)) continue;
                
                // Check name contains filter
                if (!Config.NameContains.empty())
                {
                    auto FullName = to_string(Object->GetFullName());
                    if (FullName.find(Config.NameContains) == std::string::npos) continue;
                }
                
                // Apply custom filter
                if (Config.CustomFilter && !Config.CustomFilter(Object)) continue;
                
                Results.push_back(Object);
                
                // Check max results limit
                if (Config.MaxResults > 0 && Results.size() >= Config.MaxResults) break;
            }
        }
    }
    catch (...)
    {
        Output::send<LogLevel::Error>(STR("Exception in FindObjects\n"));
    }
    
    return Results;
}

/**
 * Get Property Using Live View Method
 * 
 * This exactly replicates Live View's property access pattern:
 * 1. Get UClass from object
 * 2. Iterate properties using ForEachProperty
 * 3. Check super structs for inheritance
 * 4. Use ContainerPtrToValuePtr for data access
 * 
 * This is the MOST reliable property access method.
 */
UE4SSUtils::PropertyResult UE4SSUtils::GetProperty(UObject* Object, const std::string& PropertyName)
{
    PropertyResult Result;
    
    if (!IsValidObject(Object))
    {
        return Result;
    }
    
    try
    {
        auto ObjectClass = Object->GetClassPrivate();
        if (!ObjectClass) return Result;
        
        // First, check properties directly on the class (like Live View)
        for (FProperty* Property : ObjectClass->ForEachProperty())
        {
            // Simple string comparison - GetName() already returns StringType
            auto PropName = Property->GetName();
            if (to_string(PropName) == PropertyName)
            {
                Result.Property = Property;
                Result.DataPtr = Property->ContainerPtrToValuePtr<void>(Object);
                Result.Found = true;
                Result.PropertyName = to_string(PropName);
                Result.PropertyType = to_string(Property->GetClass().GetName());
                return Result;
            }
        }
        
        // Check super structs for inherited properties (like Live View)
        for (UStruct* SuperStruct : ObjectClass->ForEachSuperStruct())
        {
            for (FProperty* Property : SuperStruct->ForEachProperty())
            {
                            // Simple string comparison - GetName() already returns StringType
            auto PropName = Property->GetName();
            if (to_string(PropName) == PropertyName)
                {
                    Result.Property = Property;
                    Result.DataPtr = Property->ContainerPtrToValuePtr<void>(Object);
                    Result.Found = true;
                    Result.PropertyName = to_string(PropName);
                    Result.PropertyType = to_string(Property->GetClass().GetName());
                    return Result;
                }
            }
        }
    }
    catch (...)
    {
        Output::send<LogLevel::Error>(STR("Exception in GetProperty for: {}\n"), to_wstring(PropertyName));
    }
    
    return Result;
}

/**
 * Get All Properties from Object
 * 
 * Enumerates all properties like Live View does, including inheritance chain.
 * This is invaluable for debugging and discovering available properties.
 */
std::vector<UE4SSUtils::PropertyResult> UE4SSUtils::GetAllProperties(UObject* Object, bool IncludeInherited)
{
    std::vector<PropertyResult> Results;
    
    if (!IsValidObject(Object)) return Results;
    
    try
    {
        auto ObjectClass = Object->GetClassPrivate();
        if (!ObjectClass) return Results;
        
        // Get properties directly from the class
        for (FProperty* Property : ObjectClass->ForEachProperty())
        {
            PropertyResult Result;
            Result.Property = Property;
            Result.DataPtr = Property->ContainerPtrToValuePtr<void>(Object);
            Result.Found = true;
            Result.PropertyName = to_string(Property->GetName());
            Result.PropertyType = to_string(Property->GetClass().GetName());
            Results.push_back(Result);
        }
        
        // Include inherited properties if requested
        if (IncludeInherited)
        {
            for (UStruct* SuperStruct : ObjectClass->ForEachSuperStruct())
            {
                for (FProperty* Property : SuperStruct->ForEachProperty())
                {
                    PropertyResult Result;
                    Result.Property = Property;
                    Result.DataPtr = Property->ContainerPtrToValuePtr<void>(Object);
                    Result.Found = true;
                    Result.PropertyName = to_string(Property->GetName()) + " [INHERITED]";
                    Result.PropertyType = to_string(Property->GetClass().GetName());
                    Results.push_back(Result);
                }
            }
        }
    }
    catch (...)
    {
        Output::send<LogLevel::Error>(STR("Exception in GetAllProperties\n"));
    }
    
    return Results;
}

/**
 * Execute Console Command with Deferred Pattern
 * 
 * This is the battle-tested deferred execution pattern that works reliably
 * across all games. It prevents crashes by executing commands safely on
 * the game thread via ProcessEvent hooks.
 */
bool UE4SSUtils::ExecuteConsoleCommand(const std::string& Command, 
                                     UObject* TargetObject, 
                                     const std::string& FunctionName)
{
    if (!IsValidObject(TargetObject))
    {
        Output::send<LogLevel::Warning>(STR("Invalid target object for console command\n"));
        return false;
    }
    
    try
    {
        // Determine function name (extract from command if not provided)
        std::string ActualFunctionName = FunctionName;
        if (ActualFunctionName.empty())
        {
            // Extract function name from command (first word)
            size_t SpacePos = Command.find(' ');
            ActualFunctionName = (SpacePos != std::string::npos) ? 
                Command.substr(0, SpacePos) : Command;
        }
        
        // Get the function from the target object
        auto Function = TargetObject->GetFunctionByNameInChain(to_wstring(ActualFunctionName).c_str());
        if (!Function)
        {
            Output::send<LogLevel::Warning>(STR("Function '{}' not found on object\n"), 
                to_wstring(ActualFunctionName));
            return false;
        }
        
        // Set up deferred execution
        s_DeferredExec.Command = to_wstring(Command);
        s_DeferredExec.Instance = TargetObject;
        s_DeferredExec.Function = Function;
        s_DeferredExec.Executor = TargetObject;
        
        Output::send<LogLevel::Verbose>(STR("Queueing console command: {}\n"), 
            s_DeferredExec.Command);
        
        // Trigger deferred execution
        s_DeferredExec.DoCall = true;
        
        return true;
    }
    catch (...)
    {
        Output::send<LogLevel::Error>(STR("Exception in ExecuteConsoleCommand\n"));
        return false;
    }
}

/**
 * Validate Object
 * 
 * Comprehensive object validation based on UE4SS patterns and our experience
 * with crashes. This checks multiple conditions that can cause instability.
 */
bool UE4SSUtils::IsValidObject(UObject* Object)
{
    if (!Object) return false;
    
    try
    {
        // Use UE4SS's IsReal method (most reliable)
        if (!UObject::IsReal(Object)) return false;
        
        // Additional validation: ensure object has a valid class
        auto ObjectClass = Object->GetClassPrivate();
        if (!ObjectClass) return false;
        
        // Check if we can safely get the object's name
        try
        {
            auto Name = Object->GetFullName();
            if (Name.empty()) return false;
        }
        catch (...)
        {
            return false;
        }
        
        return true;
    }
    catch (...)
    {
        return false;
    }
}

/**
 * Safe Property Write
 * 
 * Safely modifies object properties with validation and type checking.
 * This prevents many crashes that occur from direct memory manipulation.
 */
bool UE4SSUtils::SetProperty(UObject* Object, 
                           const std::string& PropertyName, 
                           void* Value, 
                           size_t ValueSize)
{
    auto PropResult = GetProperty(Object, PropertyName);
    if (!PropResult.Found || !PropResult.DataPtr)
    {
        Output::send<LogLevel::Warning>(STR("Property '{}' not found for writing\n"), 
            to_wstring(PropertyName));
        return false;
    }
    
    try
    {
        // Basic size validation (could be enhanced with type-specific checks)
        if (ValueSize == 0)
        {
            Output::send<LogLevel::Warning>(STR("Invalid value size for property write\n"));
            return false;
        }
        
        // Copy the value to the property location
        memcpy(PropResult.DataPtr, Value, ValueSize);
        
        Output::send<LogLevel::Verbose>(STR("Successfully wrote to property: {}\n"), 
            to_wstring(PropertyName));
        return true;
    }
    catch (...)
    {
        Output::send<LogLevel::Error>(STR("Exception writing to property: {}\n"), 
            to_wstring(PropertyName));
        return false;
    }
}

/**
 * Log Object Information
 * 
 * Comprehensive object logging similar to Live View's display.
 * This is invaluable for debugging and understanding object structure.
 */
void UE4SSUtils::LogObjectInfo(UObject* Object, bool LogProperties, int32_t MaxProperties)
{
    if (!IsValidObject(Object))
    {
        Output::send<LogLevel::Warning>(STR("Cannot log info for invalid object\n"));
        return;
    }
    
    try
    {
        auto FullName = Object->GetFullName();
        auto ClassName = Object->GetClassPrivate()->GetFullName();
        
        Output::send<LogLevel::Verbose>(STR("=== OBJECT INFO ===\n"));
        Output::send<LogLevel::Verbose>(STR("Full Name: {}\n"), FullName);
        Output::send<LogLevel::Verbose>(STR("Class: {}\n"), ClassName);
        Output::send<LogLevel::Verbose>(STR("Address: {:p}\n"), static_cast<void*>(Object));
        
        if (LogProperties)
        {
            auto Properties = GetAllProperties(Object, true);
            Output::send<LogLevel::Verbose>(STR("Properties ({} total):\n"), Properties.size());
            
            int32_t Count = 0;
            for (const auto& Prop : Properties)
            {
                if (MaxProperties > 0 && Count >= MaxProperties) break;
                
                Output::send<LogLevel::Verbose>(STR("  [{}] {} ({})\n"), 
                    Count, to_wstring(Prop.PropertyName), to_wstring(Prop.PropertyType));
                Count++;
            }
            
            if (MaxProperties > 0 && Properties.size() > MaxProperties)
            {
                Output::send<LogLevel::Verbose>(STR("  ... and {} more\n"), 
                    Properties.size() - MaxProperties);
            }
        }
        
        Output::send<LogLevel::Verbose>(STR("=== END OBJECT INFO ===\n"));
    }
    catch (...)
    {
        Output::send<LogLevel::Error>(STR("Exception logging object info\n"));
    }
}

/**
 * Utility conversion functions
 * 
 * Helper methods for common type conversions needed in UE4SS modding.
 */
// Removed FNameToString - use Property->GetName().ToString() directly

std::wstring UE4SSUtils::StringToWString(const std::string& str)
{
    return to_wstring(str);
}

std::string UE4SSUtils::WStringToString(const std::wstring& wstr)
{
    return to_string(wstr);
}



/**
 * Cleanup Utils System
 * 
 * Cleans up resources and unhooks callbacks during mod shutdown.
 */
void UE4SSUtils::Cleanup()
{
    // Note: UE4SS doesn't provide unhook functionality, so we just reset state
    s_DeferredExec = DeferredExecution{};
    Output::send<LogLevel::Verbose>(STR("UE4SSUtils: Cleanup complete\n"));
}
