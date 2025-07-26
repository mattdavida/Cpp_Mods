#include <Mod/CppUserModBase.hpp>
#include <UE4SSProgram.hpp>
#include <Mod/LuaMod.hpp>
#include <LuaType/LuaUObject.hpp>
#include <string>
#include <format>
#include <imgui.h>

class WuChangMod : public RC::CppUserModBase
{
private:
    // UI state variables
    bool m_one_hit_kills = false;
    int m_money_to_add = 10000;
    int m_skill_points_to_add = 100;

    // Helper function to call Lua functions without arguments
    void CallLuaFunction(const std::string& function_name)
    {
        auto* lua_mod = UE4SSProgram::find_lua_mod_by_name(STR("MattsMod"));
        if (lua_mod)
        {
            std::string lua_call = std::format("MattsMod.{}()", function_name);
            lua_mod->lua().execute_string(lua_call);
        }
        else
        {
            ImGui::Text("Error: MattsMod Lua script not found!");
        }
    }

    // Helper function to call Lua functions with integer arguments
    void CallLuaFunction(const std::string& function_name, int argument)
    {
        auto* lua_mod = UE4SSProgram::find_lua_mod_by_name(STR("MattsMod"));
        if (lua_mod)
        {
            std::string lua_call = std::format("MattsMod.{}({})", function_name, argument);
            lua_mod->lua().execute_string(lua_call);
        }
        else
        {
            ImGui::Text("Error: MattsMod Lua script not found!");
        }
    }

public:
    WuChangMod() : CppUserModBase()
    {
        ModName = STR("WuChangMod (Refactored)");
        ModVersion = STR("2.0");
        ModDescription = STR("A cleaner, more organized, and powerful mod GUI.");
        ModAuthors = STR("UE4SS Team & You!");

        // Register ImGui tab for mod interface
        register_tab(STR("MattsMod"), [](CppUserModBase* instance) {
            auto mod = dynamic_cast<WuChangMod*>(instance);
            if (mod)
            {
                mod->RenderModUI();
            }
        });
    }

    auto on_ui_init() -> void override
    {
        UE4SS_ENABLE_IMGUI();
    }

    // Main UI rendering function
    auto RenderModUI() -> void
    {
        ImGui::Text("Welcome to your awesome mod!");
        ImGui::Separator();

        // Player Cheats Section
        if (ImGui::CollapsingHeader("Player Cheats"))
        {
            if (ImGui::Checkbox("One Hit Kills", &m_one_hit_kills))
            {
                CallLuaFunction("ToggleOneHitKills");
            }
        }

        // Item & Unlock Cheats Section
        if (ImGui::CollapsingHeader("Item & Unlock Cheats"))
        {
            // Add Money (Red Mercury)
            ImGui::InputInt("##MoneyToAdd", &m_money_to_add);
            ImGui::SameLine();
            if (ImGui::Button("Add Red Mercury"))
            {
                CallLuaFunction("ADD_MONEY", m_money_to_add);
            }
            ImGui::Separator();

            // Skill Points
            ImGui::InputInt("##SkillPointsToAdd", &m_skill_points_to_add);
            ImGui::SameLine();
            if (ImGui::Button("Add Skill Points"))
            {
                CallLuaFunction("ADD_SKILL_POINTS", m_skill_points_to_add);
            }
            ImGui::Separator();

            // Item unlock buttons
            if (ImGui::Button("Add all gems")) { CallLuaFunction("ADD_ALL_GEM"); }
            if (ImGui::Button("Add all tools")) { CallLuaFunction("ADD_ALL_TOOL"); }
            if (ImGui::Button("Add all skill styles")) { CallLuaFunction("ADD_ALL_STYLE_SKILL"); }
            if (ImGui::Button("Add all equipment")) { CallLuaFunction("ADD_ALL_EQUIPMENT"); }
            if (ImGui::Button("Add all enchants")) { CallLuaFunction("ADD_ALL_ENCHANT"); }
            ImGui::Separator();
            
            if (ImGui::Button("Open Shop for selling items")) { CallLuaFunction("UNLOCK_SHOP_SELL"); }
            if (ImGui::Button("Unlock all fast travel locations")) { CallLuaFunction("UNLOCK_ALL_FIRE_POINT"); }
            if (ImGui::Button("Unlock all help")) { CallLuaFunction("UNLOCK_ALL_HELP"); }
        }
        
        // Dangerous Operations Section
        if (ImGui::CollapsingHeader("!! DANGER ZONE !!"))
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.2f, 0.2f, 1.0f));
            ImGui::TextWrapped("USE WITH CAUTION. RECEIVING ALL ITEMS TAKES AROUND 5 MINUTES AND MAY CAUSE INSTABILITY.");
            ImGui::PopStyleColor();

            if (ImGui::Button("Unlock ALL items"))
            {
                CallLuaFunction("UNLOCK_ALL_ITEMS");
            }
        }
    }
};

// UE4SS mod export functions
#define WUCHANG_MOD_API __declspec(dllexport)
extern "C"
{
    WUCHANG_MOD_API RC::CppUserModBase* start_mod()
    {
        return new WuChangMod();
    }

    WUCHANG_MOD_API void uninstall_mod(RC::CppUserModBase* mod)
    {
        delete mod;
    }
}
