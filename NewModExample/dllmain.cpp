#define NOMINMAX
#include <Windows.h>
#include <Mod/CppUserModBase.hpp>
#include <DynamicOutput/DynamicOutput.hpp>
#include <Unreal/UObjectGlobals.hpp>
#include <Unreal/UObject.hpp>
#include <Unreal/UClass.hpp>
#include <String/StringType.hpp>

using namespace RC;
using namespace RC::Unreal;

class NewModExampleMod : public RC::CppUserModBase
{
private:
    bool m_ModInitialized = false;
    int m_UpdateCounter = 0;

public:
    NewModExampleMod() : CppUserModBase()
    {
        ModName = STR("NewModExampleMod");
        ModVersion = STR("1.0");
        ModDescription = STR("A clean C++ modding foundation");
        ModAuthors = STR("Your Name");
    }

    auto on_unreal_init() -> void override
    {
        Output::send<LogLevel::Verbose>(STR("🚀 NewModExampleMod: Unreal Engine initialized!\n"));
        
        // Example: Find game-specific objects
        auto PlayerController = UObjectGlobals::FindFirstOf(STR("BP_PlayerControllerZion_C"));
        if (PlayerController)
        {
            Output::send<LogLevel::Verbose>(STR("✅ Found Player Controller: {}\n"), PlayerController->GetFullName());
        }
        
        m_ModInitialized = true;
        Output::send<LogLevel::Verbose>(STR("✅ NewModExampleMod initialization complete!\n"));
    }

    auto on_update() -> void override
    {
        if (!m_ModInitialized) return;
        
        // Heartbeat logging every 30 seconds
        m_UpdateCounter++;
        if (m_UpdateCounter % 1800 == 0)
        {
            Output::send<LogLevel::Verbose>(STR("💓 Mod heartbeat - frame {}\n"), m_UpdateCounter);
        }
        
        // F9 key trigger
        static bool bF9WasPressed = false;
        bool bF9IsPressed = GetAsyncKeyState(VK_F9) & 0x8000;
        
        if (bF9IsPressed && !bF9WasPressed)
        {
            Output::send<LogLevel::Verbose>(STR("🔥 F9 pressed! Mod functionality triggered.\n"));
            DoSomethingAwesome();
        }
        
        bF9WasPressed = bF9IsPressed;
    }

    void DoSomethingAwesome()
    {
        Output::send<LogLevel::Verbose>(STR("🎉 Doing something awesome!\n"));
        
        std::vector<UObject*> FoundObjects;
        UObjectGlobals::FindClassNamesContaining("PlayerController", FoundObjects);
        Output::send<LogLevel::Verbose>(STR("📊 Found {} PlayerController objects\n"), FoundObjects.size());
    }

    ~NewModExampleMod() override
    {
        Output::send<LogLevel::Verbose>(STR("👋 NewModExampleMod shutting down\n"));
    }
};

#define NEW_MOD_EXAMPLE_MOD_API __declspec(dllexport)
extern "C"
{
    MY_AWESOME_MOD_API RC::CppUserModBase* start_mod()
    {
        return new NewModExampleMod();
    }

    MY_AWESOME_MOD_API void uninstall_mod(RC::CppUserModBase* mod)
    {
        delete mod;
    }
}
