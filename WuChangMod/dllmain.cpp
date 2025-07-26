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
    // -- State Variables for our UI --
    // These hold the current state of our cheats.
    bool m_one_hit_kills = false;
    int m_money_to_add = 10000;
    int m_skill_points_to_add = 100;

    // -- Helper function to call Lua functions without arguments --
    // This function centralizes the logic for calling a Lua function.
    // This avoids repeating the same find_lua_mod_by_name/execute_string code for every button.
    void CallLuaFunction(const std::string& function_name)
    {
        auto* lua_mod = UE4SSProgram::find_lua_mod_by_name(STR("MattsMod"));
        if (lua_mod)
        {
            // Construct the full Lua function call, e.g., "MattsMod.ADD_ALL_GEM()"
            std::string lua_call = std::format("MattsMod.{}()", function_name);
            lua_mod->lua().execute_string(lua_call);
        }
        else
        {
            // It's good practice to provide feedback if the script isn't found.
            ImGui::Text("Error: MattsMod Lua script not found!");
        }
    }

    // -- Overloaded helper function to call Lua functions WITH an integer argument --
    // This allows us to pass values from our UI (like an InputInt) to our Lua scripts.
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

        // The tab registration remains the same.
        // FIXED: Changed lambda capture from [this] to [] to match the expected function type.
        register_tab(STR("MattsMod"), [](CppUserModBase* instance) {
            // We cast the instance once here.
            auto mod = dynamic_cast<WuChangMod*>(instance);
            if (mod)
            {
                // Call our new, clean render function.
                mod->RenderModUI();
            }
        });
    }

    auto on_ui_init() -> void override
    {
        UE4SS_ENABLE_IMGUI();
    }

    // -- The Main UI Rendering Function --
    // This function is now much cleaner and easier to manage.
    auto RenderModUI() -> void
    {
        ImGui::Text("Welcome to your awesome mod!");
        ImGui::Separator();

        // -- Player Cheats Section --
        // Using a collapsing header to keep the UI tidy.
        if (ImGui::CollapsingHeader("Player Cheats"))
        {
            // Use a Checkbox for boolean toggles. It's more intuitive.
            if (ImGui::Checkbox("One Hit Kills", &m_one_hit_kills))
            {
                // We only need to call our helper function now.
                CallLuaFunction("ToggleOneHitKills");
            }
        }

        // -- Item & Unlock Cheats Section --
        if (ImGui::CollapsingHeader("Item & Unlock Cheats"))
        {
            // -- Add Money (Red Mercury) --
            ImGui::InputInt("##MoneyToAdd", &m_money_to_add);
            ImGui::SameLine();
            if (ImGui::Button("Add Red Mercury"))
            {
                // Make sure your Lua 'ADD_MONEY' function is updated to take a number!
                CallLuaFunction("ADD_MONEY", m_money_to_add);
            }
            ImGui::Separator();
                        // -- Skill Points --
            // Use an InputInt to allow the user to specify the amount.
            ImGui::InputInt("##SkillPointsToAdd", &m_skill_points_to_add);
            ImGui::SameLine(); // Place the button on the same line as the input box.
            if (ImGui::Button("Add Skill Points"))
            {
                // Call the overloaded helper that passes the integer value.
                CallLuaFunction("ADD_SKILL_POINTS", m_skill_points_to_add);
            }
            ImGui::Separator();

            // -- Simple Unlock Buttons --
            // These buttons are now simple one-liners.
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
        
        // -- Dangerous Cheats Section --
        if (ImGui::CollapsingHeader("!! DANGER ZONE !!"))
        {
             // FIXED: Changed IM_VEC4 to the correct ImVec4 struct.
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

// --- Boilerplate for UE4SS ---
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
