#define NOMINMAX
#include <Windows.h>
#include <Mod/CppUserModBase.hpp>
#include <DynamicOutput/DynamicOutput.hpp>
#include <Unreal/UObjectGlobals.hpp>
#include <Unreal/UObject.hpp>
#include <Unreal/UClass.hpp>
#include <String/StringType.hpp>
#include <UE4SSProgram.hpp>
#include <Mod/LuaMod.hpp>
#include <LuaType/LuaUObject.hpp>
#include <imgui.h>

using namespace RC;
using namespace RC::Unreal;

class MortalShellMod : public RC::CppUserModBase
{
private:
    bool m_ModInitialized = false;
    int m_UpdateCounter = 0;

    // Visual indicators state
    bool m_super_balistazooka_enabled = false;
    bool m_show_bolts_notification = false;
    float m_bolts_notification_timer = 0.0f;
    const float m_bolts_notification_duration = 3.0f;
    bool m_keybinds_enabled = false;
    
    // Player mod toggle states
    bool m_walk_fast_enabled = false;
    bool m_super_stamina_enabled = false;
    bool m_enhanced_parry_enabled = false;
    bool m_god_mode_enabled = false;
    bool m_matts_notes_enabled = false;
    bool m_super_stone_form_enabled = false;

    
    // Individual notification system for each button
    struct ButtonNotification {
        bool show = false;
        float timer = 0.0f;
        std::string text = "";
        const float duration = 3.0f;
    };
    
    ButtonNotification m_stone_form_notif;
    ButtonNotification m_super_stone_notif;
    ButtonNotification m_max_resolve_notif;
    ButtonNotification m_max_inventory_notif;
    ButtonNotification m_unlock_shells_notif;
    ButtonNotification m_walk_fast_notif;
    ButtonNotification m_super_stamina_notif;
    ButtonNotification m_enhanced_parry_notif;
    ButtonNotification m_god_mode_notif;
    ButtonNotification m_matts_notes_notif;
    ButtonNotification m_fast_travel_notif;
    ButtonNotification m_unlock_funcs_notif;
    ButtonNotification m_exploit_notif;
    ButtonNotification m_player_keybinds_notif;


public:
    MortalShellMod() : CppUserModBase()
    {
        ModName = STR("MortalShellMod");
        ModVersion = STR("1.0");
        ModDescription = STR("A clean C++ modding foundation");
        ModAuthors = STR("Your Name");
    }

    // Helper function to call Lua functions (from WuChang pattern)
    void CallLuaFunction(const std::string& function_name)
    {
        auto* lua_mod = UE4SSProgram::find_lua_mod_by_name(STR("MortalShellMod"));
        if (lua_mod)
        {
            std::string lua_call = std::format("{}()", function_name);
            lua_mod->lua().execute_string(lua_call);
        }
        else
        {
            Output::send<LogLevel::Warning>(STR("Error: MortalShellMod Lua script not found!\n"));
        }
    }

    // Helper function to call Lua functions with integer arguments
    void CallLuaFunction(const std::string& function_name, int argument)
    {
        auto* lua_mod = UE4SSProgram::find_lua_mod_by_name(STR("MortalShellMod"));
        if (lua_mod)
        {
            std::string lua_call = std::format("{}({})", function_name, argument);
            lua_mod->lua().execute_string(lua_call);
        }
        else
        {
            Output::send<LogLevel::Warning>(STR("Error: MortalShellMod Lua script not found!\n"));
        }
    }

    auto on_ui_init() -> void override
    {
        UE4SS_ENABLE_IMGUI();
        
        // Register ImGui tab after UI is initialized
        register_tab(STR("Weapons Mod"), [](CppUserModBase* instance) {
            auto mod = dynamic_cast<MortalShellMod*>(instance);
            if (mod)
            {
                mod->RenderWeaponModUI();
            }
        });
        register_tab(STR("Player Mods"), [](CppUserModBase* instance) {
            auto mod = dynamic_cast<MortalShellMod*>(instance);
            if (mod)
            {
                mod->RenderPlayerModUI();
            }
        });
    }

    auto on_unreal_init() -> void override
    {
        Output::send<LogLevel::Verbose>(STR("🚀 MortalShellMod: Unreal Engine initialized!\n"));
    
        m_ModInitialized = true;
        Output::send<LogLevel::Verbose>(STR("✅ MortalShellMod initialization complete!\n"));
    }
    // Main UI rendering function like WuChang mod
    auto RenderWeaponModUI() -> void
    {
        ImGui::Text("Weapons Mod");
        ImGui::Separator();
        

        
        // Rewards Section
        if (ImGui::CollapsingHeader("Weapons"))
        {
            ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "Bypass weapon rack & fish chests - instant weapon swap anywhere!");
            ImGui::Spacing();
            
            if (ImGui::Button("Hallowed Sword", ImVec2(0, 30)))
            {
                SummonWeapon(0);
            }           
            ImGui::Spacing();

            if (ImGui::Button("Smoldering Mace", ImVec2(0, 30)))
            {
                SummonWeapon(1);
            }                   
            ImGui::Spacing();

            if (ImGui::Button("Hammer and Chisel", ImVec2(0, 30)))
            {
                SummonWeapon(2);
            }                   
            ImGui::Spacing();

            if (ImGui::Button("Martyr's Blade", ImVec2(0, 30)))
            {
                SummonWeapon(3);
            }                   
            ImGui::Spacing();

            if (ImGui::Button("Axatana twin blades", ImVec2(0, 30)))
            {
                SummonWeapon(4);
            }                   
            ImGui::Spacing();

            if (ImGui::Button("Axatana axe", ImVec2(0, 30)))
            {
                SummonWeapon(5);
            }                   
                  
        }

        if (ImGui::CollapsingHeader("Balistazooka"))
        {
            // Super Balistazooka Status Indicator
            if (m_super_balistazooka_enabled)
            {
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "[ON] Super Balistazooka Enabled");
            }
            else
            {
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "[OFF] Super Balistazooka Disabled");
            }
            ImGui::Spacing();
         
            ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "Balistazooka can be used more freely");
            ImGui::Spacing();
            if (ImGui::Button("Toggle Super Ballistazooka", ImVec2(0, 30)))
            {
                SuperBallistazooka();
            }                   
            ImGui::Spacing();

            ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "Get 999 Bolts");
            ImGui::Spacing();
            if (ImGui::Button("Max out Bolts", ImVec2(0, 30)))
            {
                MaxOutBolts();
            }    
            
            // Bolts Notification (Bootstrap-style alert)
            if (m_show_bolts_notification && m_bolts_notification_timer > 0.0f)
            {
                ImGui::Spacing();
                // Calculate fade effect based on timer
                float alpha = m_bolts_notification_timer / m_bolts_notification_duration;
                ImGui::TextColored(ImVec4(0.0f, 0.8f, 0.0f, alpha), "✓ Bolts Refreshed!");
                
                // Update timer (using ImGui's delta time)
                m_bolts_notification_timer -= ImGui::GetIO().DeltaTime;
                
                if (m_bolts_notification_timer <= 0.0f)
                {
                    m_show_bolts_notification = false;
                }
            }    
        }

        if (ImGui::CollapsingHeader("Keybinds"))
        {
            // Keybinds Status Indicator
            if (m_keybinds_enabled)
            {
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "[ON] Keybinds Enabled");
            }
            else
            {
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "[OFF] Keybinds Disabled");
            }
            ImGui::Spacing();
            
            if (ImGui::Button("Enable Keybinds", ImVec2(0, 30)))
            {
                EnableWeaponModKeybinds();
            }
            ImGui::Spacing();
            
            // Key Mappings Display
            if (m_keybinds_enabled)
            {
                ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.2f, 1.0f), "Key Mappings:");
                ImGui::Separator();
                
                ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "3 - Max Out Bolts");
                ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "4 - Super Ballistazooka");
                
                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.2f, 1.0f), "Weapons:");
                ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "5 - Hallowed Sword");
                ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "6 - Smoldering Mace");
                ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "7 - Hammer and Chisel");
                ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "8 - Martyr's Blade");
                ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "9 - Axatana Twin Blades");
                ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "0 - Axatana Axe");
            }
        }
    } 

    auto RenderPlayerModUI() -> void
    {
        ImGui::Text("Player Mods");
        ImGui::Separator();

        // Status indicators for toggles
        ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.2f, 1.0f), "Toggle Status:");
        
        // Line 1: Walk Fast | Super Stamina | Super Stone Form
        ImGui::TextColored(m_walk_fast_enabled ? ImVec4(0.0f, 1.0f, 0.0f, 1.0f) : ImVec4(0.7f, 0.7f, 0.7f, 1.0f), 
            "Walk Fast: %s", m_walk_fast_enabled ? "ON" : "OFF");
        ImGui::SameLine();
        ImGui::TextColored(m_super_stamina_enabled ? ImVec4(0.0f, 1.0f, 0.0f, 1.0f) : ImVec4(0.7f, 0.7f, 0.7f, 1.0f), 
            " | Super Stamina: %s", m_super_stamina_enabled ? "ON" : "OFF");
        ImGui::SameLine();
        ImGui::TextColored(m_super_stone_form_enabled ? ImVec4(0.0f, 1.0f, 0.0f, 1.0f) : ImVec4(0.7f, 0.7f, 0.7f, 1.0f), 
            " | Super Stone: %s", m_super_stone_form_enabled ? "ON" : "OFF");
        
        // Line 2: Enhanced Parry | God Mode | Matt's Notes
        ImGui::TextColored(m_enhanced_parry_enabled ? ImVec4(0.0f, 1.0f, 0.0f, 1.0f) : ImVec4(0.7f, 0.7f, 0.7f, 1.0f), 
            "Enhanced Parry: %s", m_enhanced_parry_enabled ? "ON" : "OFF");
        ImGui::SameLine();
        ImGui::TextColored(m_god_mode_enabled ? ImVec4(0.0f, 1.0f, 0.0f, 1.0f) : ImVec4(0.7f, 0.7f, 0.7f, 1.0f), 
            " | God Mode: %s", m_god_mode_enabled ? "ON" : "OFF");
        ImGui::SameLine();
        ImGui::TextColored(m_matts_notes_enabled ? ImVec4(0.0f, 1.0f, 0.0f, 1.0f) : ImVec4(0.7f, 0.7f, 0.7f, 1.0f), 
            " | Matt's Notes: %s", m_matts_notes_enabled ? "ON" : "OFF");
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::CollapsingHeader("One-Time Actions"))
        {
            ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "Instant effects - use once per session");
            ImGui::Spacing();
            
            if (ImGui::Button("Stone Form No Cooldown", ImVec2(0, 30)))
            {
                StoneFormNoCooldown();
            }
            RenderButtonNotification(m_stone_form_notif);

            if (ImGui::Button("Max Resolve", ImVec2(0, 30)))
            {
                MaxResolve();
            }
            RenderButtonNotification(m_max_resolve_notif);

            if (ImGui::Button("Max Out Inventory", ImVec2(0, 30)))
            {
                MaxOutInventory();
            }
            RenderButtonNotification(m_max_inventory_notif);

            if (ImGui::Button("Unlock All Shells", ImVec2(0, 30)))
            {
                UnlockAllShells();
            }
            RenderButtonNotification(m_unlock_shells_notif);
        }

        if (ImGui::CollapsingHeader("Toggle Features"))
        {
            ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "On/Off switches - can be toggled multiple times");
            ImGui::Spacing();
            
            if (ImGui::Button("Toggle Walk Fast", ImVec2(0, 30)))
            {
                ToggleWalkFast();
            }
            RenderButtonNotification(m_walk_fast_notif);
            
            if (ImGui::Button("Toggle Super Stamina", ImVec2(0, 30)))
            {
                ToggleSuperStamina();
            }
            RenderButtonNotification(m_super_stamina_notif);
            
            if (ImGui::Button("Toggle Super Stone Form", ImVec2(0, 30)))
            {
                ToggleSuperStoneForm();
            }
            RenderButtonNotification(m_super_stone_notif);
            
            if (ImGui::Button("Toggle Enhanced Parry", ImVec2(0, 30)))
            {
                ToggleEnhancedParry();
            }
            RenderButtonNotification(m_enhanced_parry_notif);
            
            if (ImGui::Button("Toggle God Mode", ImVec2(0, 30)))
            {
                ToggleGodMode();
            }
            RenderButtonNotification(m_god_mode_notif);
        }
        


        if (ImGui::CollapsingHeader("Advanced Features"))
        {
            ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "Powerful features and system settings");
            ImGui::Spacing();

            if (ImGui::Button("Enable Matt's Notes", ImVec2(0, 30)))
            {
                EnableMattsNotes();
            }
            RenderButtonNotification(m_matts_notes_notif);
            
            if (ImGui::Button("Always Allow Fast Travel", ImVec2(0, 30)))
            {
                CallLuaFunction("AlwaysAllowFastTravel");
                ShowButtonNotification(m_fast_travel_notif, "✓ Fast Travel Always Enabled!");
            }
            RenderButtonNotification(m_fast_travel_notif);
            
            if (ImGui::Button("Unlock All Player Funcs", ImVec2(0, 30)))
            {
                CallLuaFunction("UnlockAllPlayerFuncs");
                ShowButtonNotification(m_unlock_funcs_notif, "✓ All Player Functions Unlocked!");
            }
            RenderButtonNotification(m_unlock_funcs_notif);
            
            if (ImGui::Button("Exploit Inventory & Fast Travel", ImVec2(0, 30)))
            {
                CallLuaFunction("ExploitInventoryAndUnlockFastTravelLocations");
                ShowButtonNotification(m_exploit_notif, "✓ Inventory Exploits Enabled!");
            }
            RenderButtonNotification(m_exploit_notif);
            

        }

        if (ImGui::CollapsingHeader("Player Keybinds"))
        {
            ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "Keyboard shortcuts are always enabled");
            ImGui::Spacing();
            
            ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.2f, 1.0f), "One-Time Actions:");
            ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "F1 - Stone Form No Cooldown");
            ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "F5 - Max Resolve");
            ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "F9 - Max Out Inventory");
            ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "NUM_LOCK - Unlock All Shells");
            ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "BACKSPACE - Unlock All Player Funcs");
            
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.2f, 1.0f), "Toggle Features:");
            ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "F2 - Toggle Super Stone Form");
            ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "F3 - Toggle Walk Fast");
            ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "F4 - Toggle Super Stamina");
            ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "F6 - Toggle Enhanced Parry");
            ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "F8 - Toggle God Mode");
            
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.2f, 1.0f), "Advanced Features:");
            ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "F7 - Toggle Matt's Mods (ALL)");
            ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "HOME - Always Allow Fast Travel");
            ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "PAGE_DOWN - Exploit Inventory & Fast Travel");
            ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "CAPS_LOCK - Enable Matt's Notes");
            
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.2f, 1.0f), "System:");
            ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "PAGE_UP - Show Detailed Status");
        }
        
    } 

    void SummonWeapon(int weapon_id)
    {
        Output::send<LogLevel::Verbose>(STR("🎉 Summoning weapon!\n"));
        CallLuaFunction("SummonWeaponFromKeybind", weapon_id);
    }

    void SuperBallistazooka()
    {
        m_super_balistazooka_enabled = !m_super_balistazooka_enabled;
        Output::send<LogLevel::Verbose>(STR("🎉 Super Ballistazooka %s!\n"), 
            m_super_balistazooka_enabled ? STR("Enabled") : STR("Disabled"));
        CallLuaFunction("SuperBallistazooka");
    }

    void MaxOutBolts()
    {
        Output::send<LogLevel::Verbose>(STR("🎉 Maxing out Bolts!\n"));
        CallLuaFunction("MaxOutBolts");
        
        // Trigger the bolts notification
        m_show_bolts_notification = true;
        m_bolts_notification_timer = m_bolts_notification_duration;
    }

    void EnableWeaponModKeybinds()
    {
        Output::send<LogLevel::Verbose>(STR("🎉 Enabling Keybinds!\n"));
        CallLuaFunction("EnableWeaponModKeybinds");
        m_keybinds_enabled = true;
    }

    // Helper function to show individual button notifications
    void ShowButtonNotification(ButtonNotification& notif, const std::string& message)
    {
        notif.text = message;
        notif.show = true;
        notif.timer = notif.duration;
    }

    // Helper function to render notification under a button
    void RenderButtonNotification(ButtonNotification& notif)
    {
        if (notif.show && notif.timer > 0.0f)
        {
            // Calculate fade effect based on timer
            float alpha = notif.timer / notif.duration;
            ImGui::TextColored(ImVec4(0.0f, 0.8f, 0.0f, alpha), notif.text.c_str());
            
            // Update timer (using ImGui's delta time)
            notif.timer -= ImGui::GetIO().DeltaTime;
            
            if (notif.timer <= 0.0f)
            {
                notif.show = false;
            }
        }
    }

    // Toggle functions with state tracking
    void ToggleWalkFast()
    {
        CallLuaFunction("ToggleWalkFast");
        m_walk_fast_enabled = !m_walk_fast_enabled;
        ShowButtonNotification(m_walk_fast_notif, m_walk_fast_enabled ? "Walk Fast: ON" : "Walk Fast: OFF");
    }

    void ToggleSuperStamina()
    {
        CallLuaFunction("ToggleSuperStamina");
        m_super_stamina_enabled = !m_super_stamina_enabled;
        ShowButtonNotification(m_super_stamina_notif, m_super_stamina_enabled ? "Super Stamina: ON" : "Super Stamina: OFF");
    }

    void ToggleEnhancedParry()
    {
        CallLuaFunction("ToggleEnhancedParryChance");
        m_enhanced_parry_enabled = !m_enhanced_parry_enabled;
        ShowButtonNotification(m_enhanced_parry_notif, m_enhanced_parry_enabled ? "Enhanced Parry: ON" : "Enhanced Parry: OFF");
    }

    void ToggleGodMode()
    {
        CallLuaFunction("ToggleGodMode");
        m_god_mode_enabled = !m_god_mode_enabled;
        ShowButtonNotification(m_god_mode_notif, m_god_mode_enabled ? "God Mode: ON" : "God Mode: OFF");
    }

    void EnableMattsNotes()
    {
        CallLuaFunction("EnableMattsNotes");
        m_matts_notes_enabled = true;
        ShowButtonNotification(m_matts_notes_notif, "✓ Matt's Notes Enabled!");
    }



    void ToggleSuperStoneForm()
    {
        CallLuaFunction("ToggleSuperStoneFormMax");
        m_super_stone_form_enabled = !m_super_stone_form_enabled;
        ShowButtonNotification(m_super_stone_notif, m_super_stone_form_enabled ? "Super Stone Form: ON" : "Super Stone Form: OFF");
    }

    // One-time action functions with notifications
    void StoneFormNoCooldown()
    {
        CallLuaFunction("StoneFormNoCooldown");
        ShowButtonNotification(m_stone_form_notif, "✓ Stone Form Cooldown Removed!");
    }

    void MaxResolve()
    {
        CallLuaFunction("MaxResolve");
        ShowButtonNotification(m_max_resolve_notif, "✓ Resolve Maxed Out!");
    }

    void MaxOutInventory()
    {
        CallLuaFunction("MaxOutInventory");
        ShowButtonNotification(m_max_inventory_notif, "✓ Inventory Maxed!");
    }

    void UnlockAllShells()
    {
        CallLuaFunction("UnlockAllShells");
        ShowButtonNotification(m_unlock_shells_notif, "✓ All Shells Unlocked!");
    }

    ~MortalShellMod() override
    {
        Output::send<LogLevel::Verbose>(STR("👋 MortalShellMod shutting down\n"));
    }
};

#define MORTAL_SHELL_MOD_API __declspec(dllexport)
extern "C"
{
    MORTAL_SHELL_MOD_API RC::CppUserModBase* start_mod()
    {
        return new MortalShellMod();
    }

    MORTAL_SHELL_MOD_API void uninstall_mod(RC::CppUserModBase* mod)
    {
        delete mod;
    }
}
