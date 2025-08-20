    #define NOMINMAX
    #include <Windows.h>
    #include <Mod/CppUserModBase.hpp>
    #include <DynamicOutput/DynamicOutput.hpp>
    #include <Unreal/UObjectGlobals.hpp>
    #include <Unreal/UObject.hpp>
    #include <Unreal/UFunction.hpp>
    #include <Unreal/FString.hpp>
    #include <Unreal/FOutputDevice.hpp>
    #include <Unreal/UnrealFlags.hpp>
    #include <Unreal/Hooks.hpp>
    #include <String/StringType.hpp>
    #include <Helpers/String.hpp>
    #include <imgui.h>

    #include <UE4SSProgram.hpp>
    #include <Mod/LuaMod.hpp>
    #include <LuaType/LuaUObject.hpp>
    #include <string>
    #include <format>
    #include <unordered_map>
    #include <thread>
    #include <chrono>

    using namespace RC;
    using namespace RC::Unreal;

    // Static variables for deferred shop execution (like UFunctionCallerWidget)
    static bool s_do_shop_call{};
    static UObject* s_shop_instance{};
    static StringType s_shop_cmd{};
    static FOutputDevice s_shop_ar{};
    static UObject* s_shop_executor{};

    // Static variables for general deferred execution (existing functionality)
    static bool s_do_call{};
    static UObject* s_instance{};
    static FString s_cmd{};
    static FOutputDevice s_ar{};
    static UFunction* s_function{};
    static UObject* s_executor{};

    // Hook callback for deferred shop execution (exactly like UFunctionCallerWidget)
    auto call_shop_console_exec(UObject*, UFunction*, void*) -> void
    {
        if (s_do_shop_call)
        {
            s_do_shop_call = false;
            Output::send(STR("Processing command: {}\n"), s_shop_cmd);
            bool call_succeeded = s_shop_instance->ProcessConsoleExec(FromCharTypePtr<TCHAR>(s_shop_cmd.c_str()), s_shop_ar, s_shop_executor);
            Output::send(STR("call_succeeded: {}\n"), call_succeeded);
        }
    }
    auto call_process_console_exec(UObject*, UFunction*, void*) -> void
    {
        if (s_do_call)
        {
            s_do_call = false;
            auto& function_flags = s_function->GetFunctionFlags();
            function_flags |= FUNC_Exec;
            Output::send(STR("Processing command: {}\n"), s_cmd.GetCharArray());
            bool call_succeeded = s_instance->ProcessConsoleExec(s_cmd.GetCharArray(), s_ar, s_executor);
            Output::send(STR("call_succeeded: {}\n"), call_succeeded);
            function_flags &= ~FUNC_Exec;
        }
    }

    class StellarBladeMod : public RC::CppUserModBase
    {
    private:
        bool m_ModInitialized = false;
        UObject* m_CheatManager = nullptr;
        
        // Shop selection state
        int m_selected_shop_index = 0;
        std::vector<std::string> m_shop_names = {
            "Shop_SimpleCamp",
            "Shop_FullCamp", 
            "Shop_Kaya",
            "Shop_Digger",
            "Shop_Spotter",
            "Shop_Clyde",
            "Shop_Roxane",
            "Shop_Lyle",
            "Shop_NightOperation",
            "Shop_Barry",
            "Shop_HairSalon",
            "Shop_NanosuitMaking_1",
            "Shop_NestCamp",
            "Shop_Emil",
            "Shop_Bolt",
            "Shop_StudioTest"
        };
        
        // Camp selection state - hierarchical structure with FullCamp->SimplyCamp nesting
        struct CampInfo {
            std::string id;
            std::string label;
            std::string type; // "FullCamp" or "SimplyCamp"
        };
        
        struct FullCampGroup {
            CampInfo main_camp; // The FullCamp
            std::vector<CampInfo> simple_camps; // SimplyCamps that belong to this FullCamp
        };
        
        int m_selected_camp_index = 0;
        std::string m_selected_camp_display = "Silent Street"; // For display in dropdown button
        std::vector<std::pair<std::string, std::vector<FullCampGroup>>> m_hierarchical_camps = {
            {"Eidos 7", {
                {{"DED10_Camp_03", "Parking Tower 2nd Floor", "FullCamp"}, {
                    {"DED10_Camp_01", "Silent Street - Checkpoint 1", "SimplyCamp"},
                    {"DED10_Camp_02", "Silent Street - Checkpoint 2", "SimplyCamp"}
                }},
                {{"DED10_Camp_06", "Abandoned Station", "FullCamp"}, {
                    {"DED10_Camp_04", "Parking Tower - Checkpoint 1", "SimplyCamp"},
                    {"DED10_Camp_05", "Parking Tower - Checkpoint 2", "SimplyCamp"}
                }},
                {{"DED10_Camp_07", "Silent Street", "FullCamp"}, {}},
                {{"DED20_Camp_02", "Plaza Entryway", "FullCamp"}, {
                    {"DED20_Camp_01", "Plaza Entryway - Checkpoint 1", "SimplyCamp"}
                }},
                {{"DED20_Camp_05", "Construction Zone", "FullCamp"}, {
                    {"DED20_Camp_03", "Construction Zone - Checkpoint 1", "SimplyCamp"},
                    {"DED20_Camp_04", "Construction Zone - Checkpoint 2", "SimplyCamp"},
                    {"DED20_Camp_06", "Construction Zone - Checkpoint 3", "SimplyCamp"}
                }},
                {{"DED30_Camp_02", "Crater", "FullCamp"}, {
                    {"DED30_Camp_01", "Crater - Checkpoint 1", "SimplyCamp"}
                }}
            }},
            {"Eidos 9", {
                {{"DEDA_Camp_02", "Submerged City", "FullCamp"}, {
                    {"DEDA_Camp_01", "Submerged City - Checkpoint 1", "SimplyCamp"},
                    {"DEDA_Camp_03", "Submerged City - Checkpoint 2", "SimplyCamp"},
                    {"DEDA_Camp_04", "Submerged City - Checkpoint 3", "SimplyCamp"},
                    {"DEDA_Camp_05", "Submerged City - Checkpoint 4", "SimplyCamp"},
                    {"DEDA_Camp_06", "Submerged City - Checkpoint 5", "SimplyCamp"}
                }}
            }},
            {"Xion City", {
                {{"Xion_Camp_10", "Xion City", "FullCamp"}, {
                    {"Xion_Camp_20", "Xion City - Checkpoint 1", "SimplyCamp"}
                }}
            }},
            {"Wasteland", {
                {{"WLA10_Camp_01", "Hidden Path", "FullCamp"}, {
                    {"WLA10_Camp_04", "Hidden Path - Checkpoint 1", "SimplyCamp"},
                    {"WLA10_Camp_05", "Hidden Path - Checkpoint 2", "SimplyCamp"},
                    {"WLA10_Camp_07", "Hidden Path - Checkpoint 3", "SimplyCamp"}
                }},
                {{"WLA10_Camp_02", "Western Great Canyon", "FullCamp"}, {}},
                {{"WLA10_Camp_06", "Solar Tower Entrance", "FullCamp"}, {}},
                {{"WLA30_Camp_01", "Junkyard Supply Camp", "FullCamp"}, {
                    {"WLA30_Camp_02", "Junkyard - Checkpoint 1", "SimplyCamp"}
                }},
                {{"WLA30_Camp_03", "Altes Levoire Entrance", "FullCamp"}, {}},
                {{"WLA40_Camp_01", "Central Scrap Plains", "FullCamp"}, {
                    {"WLA40_Camp_02", "Central Scrap - Checkpoint 1", "SimplyCamp"}
                }},
                {{"WLA50_Camp_01", "Scrap Yard Entrance", "FullCamp"}, {}}
            }},
            {"Altess Levoire", {
                {{"ATL03_Camp_02", "Bottom of the Heart of the Infection", "FullCamp"}, {
                    {"ATL01_Camp_01", "Altess Lab - Checkpoint 1", "SimplyCamp"},
                    {"ATL03_Camp_01", "Altess Lab - Checkpoint 2", "SimplyCamp"},
                    {"ATL01_Camp_02", "Altess Lab - Checkpoint 3", "SimplyCamp"},
                    {"ATL02_Camp_01", "Altess Lab - Checkpoint 4", "SimplyCamp"}
                }}
            }},
            {"Matrix 11", {
                {{"ME01_Camp_03", "Underground Platform", "FullCamp"}, {
                    {"ME01_Camp_01", "Underground - Checkpoint 1", "SimplyCamp"},
                    {"ME01_Camp_02", "Underground - Checkpoint 2", "SimplyCamp"}
                }},
                {{"ME03_Camp_01", "Twisted Iron Bridge", "FullCamp"}, {
                    {"ME03_Camp_02", "Iron Bridge - Checkpoint 1", "SimplyCamp"}
                }},
                {{"ME03_Camp_03", "Rail Yard", "FullCamp"}, {
                    {"ME04_Camp_01", "Rail Yard - Checkpoint 1", "SimplyCamp"},
                    {"ME04_Camp_02", "Rail Yard - Checkpoint 2", "SimplyCamp"}
                }},
                {{"ME05_Camp_01", "Temporary Armory Entrance", "FullCamp"}, {
                    {"ME05_Camp_02", "Armory - Checkpoint 1", "SimplyCamp"}
                }},
                {{"ME06_Camp_02", "Contaminated Water Purification Plant Entrance", "FullCamp"}, {
                    {"ME06_Camp_01", "Water Plant - Checkpoint 1", "SimplyCamp"}
                }},
                {{"WLB20_Camp_01", "Matrix 11 Entrance", "FullCamp"}, {}}
            }},
            {"Great Desert", {
                {{"WLB10_Camp_01", "Exiles Passage", "FullCamp"}, {
                    {"WLB10_Camp_03", "Exiles - Checkpoint 1", "SimplyCamp"}
                }},
                {{"WLB10_Camp_02", "Oasis", "FullCamp"}, {}},
                {{"WLB10_Camp_04", "Central Great Desert", "FullCamp"}, {}},
                {{"WLB10_Camp_05", "Buried Ruins Outskirt", "FullCamp"}, {
                    {"WLB10_Camp_06", "Buried Ruins - Checkpoint 1", "SimplyCamp"}
                }},
                {{"WLB10_Camp_07", "Abyss Levoire Entrance", "FullCamp"}, {}},
                {{"WLB10_Camp_08", "Great Desert", "FullCamp"}, {}},
                {{"WLB20_Camp_03", "Abandoned Overpass", "FullCamp"}, {
                    {"WLB20_Camp_02", "Overpass - Checkpoint 1", "SimplyCamp"}
                }},
                {{"WLB20_Camp_04", "Way to the Solar Tower", "FullCamp"}, {}},
                {{"WLB20_Camp_05", "Debris-filled Entryway", "FullCamp"}, {}},
                {{"WLB30_Camp_01", "Great Desert Outskirts", "FullCamp"}, {
                    {"WLB30_Camp_02", "Outskirts - Checkpoint 1", "SimplyCamp"}
                }},
                {{"WLB30_Camp_03", "Twin Rocks", "FullCamp"}, {}},
                {{"WLB50_Camp_06", "Crumbling Rooftop", "FullCamp"}, {
                    {"WLB50_Camp_01", "Rooftop - Checkpoint 1", "SimplyCamp"},
                    {"WLB50_Camp_02", "Rooftop - Checkpoint 2", "SimplyCamp"},
                    {"WLB50_Camp_03", "Rooftop - Checkpoint 3", "SimplyCamp"}
                }}
            }},
            {"Abyss Levoire", {
                {{"AYL01_Camp_01", "Ventilation Sector", "FullCamp"}, {
                    {"AYL01_Camp_02", "Ventilation - Checkpoint 1", "SimplyCamp"}
                }},
                {{"AYL06_Camp_01", "Heart of the Contamination", "FullCamp"}, {
                    {"AYL03_Camp_01", "Heart - Checkpoint 1", "SimplyCamp"},
                    {"AYL04_Camp_01", "Heart - Checkpoint 2", "SimplyCamp"},
                    {"AYL04_Camp_02", "Heart - Checkpoint 3", "SimplyCamp"},
                    {"AYL05_Camp_01", "Heart - Checkpoint 4", "SimplyCamp"}
                }}
            }},
            {"Spire 4", {
                {{"SE01_Camp_01", "Space Complex Entryway (Original)", "FullCamp"}, {
                    {"SE01_Camp_02", "Space Complex - Checkpoint 1", "SimplyCamp"}
                }},
                {{"SE01_Camp_01_Re", "Space Complex Entryway", "FullCamp"}, {}},
                {{"SE02_Camp_02", "Hypertube Entrance", "FullCamp"}, {
                    {"SE04_Camp_01", "Hypertube - Checkpoint 1", "SimplyCamp"}
                }},
                {{"SE05_Camp_01", "Space Station Level 1", "FullCamp"}, {
                    {"SE05_Camp_02", "Space Station - Checkpoint 1", "SimplyCamp"}
                }},
                {{"SE05_Camp_03", "Space Logistics Center", "FullCamp"}, {
                    {"SE05_Camp_04", "Logistics - Checkpoint 1", "SimplyCamp"}
                }},
                {{"SE06_Camp_02", "Raphael Space Center Lobby", "FullCamp"}, {
                    {"SE06_Camp_01", "Space Center - Checkpoint 1", "SimplyCamp"}
                }},
                {{"SE08_Camp_02", "Prestige Lounge (Original)", "FullCamp"}, {
                    {"SE07_Camp_01", "Prestige - Checkpoint 1", "SimplyCamp"},
                    {"SE07_Camp_02", "Prestige - Checkpoint 2", "SimplyCamp"},
                    {"SE08_Camp_01", "Prestige - Checkpoint 3", "SimplyCamp"}
                }},
                {{"SE08_Camp_02_Re", "Prestige Lounge", "FullCamp"}, {}},
                {{"SE09_Camp_01", "Central Core Entrance", "FullCamp"}, {}}
            }},
            {"Nest", {
                {{"Nest30_Camp_01", "Nest", "FullCamp"}, {}}
            }},
            {"Danger Areas", {
                {{"CombatTestMapMk2_Camp_01", "Combat Test Map (Danger - All Black)", "FullCamp"}, {}},
                {{"SysTest_Camp_01", "System Test 1 (Danger - All Black)", "FullCamp"}, {
                    {"SysTest_Camp_00", "System Test - Checkpoint 1", "SimplyCamp"}
                }},
                {{"SysTest_Camp_02", "System Test 2 (Danger - All Black)", "FullCamp"}, {}},
                {{"SysTest_Nikke_Camp_00", "System Test Nikke (Danger - All Black)", "FullCamp"}, {}},
                {{"ARD10_Camp_01", "ARD10 (Danger - All Black)", "FullCamp"}, {}},
                {{"MET10_Camp_01", "MET10 (Danger - All Black)", "FullCamp"}, {}}
            }}
        };
        
        // Shop to currency mapping
        std::unordered_map<std::string, std::string> m_shop_currencies = {
            {"Shop_SimpleCamp", "BetaCrystal"},
            {"Shop_FullCamp", "BetaCrystal"},
            {"Shop_Kaya", "BetaCrystal"},
            {"Shop_Digger", "BetaCrystal"},
            {"Shop_Spotter", "ETC_Item_VendingMachineCoin"},
            {"Shop_Clyde", "FishingPoint"},
            {"Shop_Roxane", "BetaCrystal"},
            {"Shop_Lyle", "BetaCrystal"},
            {"Shop_NightOperation", "BetaCrystal"},
            {"Shop_Barry", "BetaCrystal"},
            {"Shop_HairSalon", "BetaCrystal"},
            {"Shop_NanosuitMaking_1", "BetaCrystal"},
            {"Shop_NestCamp", "BetaCrystal"},
            {"Shop_Emil", "Money_Nier"},
            {"Shop_Bolt", "Money_Nikke"},
            {"Shop_StudioTest", "BetaCrystal"}
        };
        
        // Custom reward input
        char m_custom_reward_input[256] = "";
        
        // Can selection state (1-based for Can_001 to Can_049)
        int m_selected_can_index = 1;
        
        // Unlock all cans state
        bool m_unlocking_all_cans = false;
        int m_current_unlock_can = 1;
        
        // Trophy data structure
        struct TrophyInfo {
            std::string key;
            bool hidden;
        };
        
        std::vector<TrophyInfo> m_trophies = {
            {"Platinum", false},
            {"Activate_FirstCamp", false},
            {"KillCharacter_Opener", true},
            {"KillCharacter_GrubShooter", true},
            {"KillCharacter_Gigas_Kill", true},
            {"KillCharacter_Brute", true},
            {"CompleteLevel_AltesLabor", true},
            {"KillCharacter_Stalker_Kill", true},
            {"KillCharacter_Juggernaut_Kill", true},
            {"KillCharacter_Tachy_Kill", true},
            {"KillCharacter_Behemoth", true},
            {"CompleteLevel_AbyssLabor", true},
            {"KillCharacter_Belial_A_Kill", true},
            {"KillCharacter_Marionette_Kill", true},
            {"KillCharacter_Gorgon_Kill", true},
            {"KillCharacter_Raven_Kill", true},
            {"Ending_Colony", true},
            {"Ending_LostMemory", true},
            {"Ending_NewMemory", true},
            {"Acquire_AllCan", false},
            {"Acquire_AllNanoSuit", false},
            {"Acquire_AllRecords", false},
            {"Album_AllFish", false},
            {"Open_AllBox", false},
            {"KillCharacter_AllNative", false},
            {"Activate_AllCamp", false},
            {"LevelUpMax_AllExoSpine", false},
            {"WeaponMaxUpgrade", false},
            {"TumblerMaxUpgrade", false},
            {"BodyMaxUpgrade", false},
            {"BetaMaxUpgrade", false},
            {"Acquire_AllSkill", false},
            {"CompleteQuest_Enya", true},
            {"CompleteQuest_Kaya", true},
            {"CompleteQuest_Digger", true},
            {"JustEvade", false},
            {"JustParry", false},
            {"CharKill_AssassinationSkills", false},
            {"CharKill_BetaSkill", false},
            {"CharKill_BurstSkill", false},
            {"CharKill_TachySkill", true},
            {"CharKill_RangeSkill", false},
            {"KillCharacter", false},
            {"Complete_NewGamePlus", false},
            {"Acquire_AllSkill_NewGamePlus", false}
        };
        
        // Trophy selection state
        int m_selected_trophy_index = 0;
        std::string m_selected_trophy_display = "Platinum";
        
        // Filter visible trophies (non-hidden ones)
        std::vector<TrophyInfo> GetVisibleTrophies() const
        {
            std::vector<TrophyInfo> visible;
            for (const auto& trophy : m_trophies)
            {
                visible.push_back(trophy);
            }
            return visible;
        }
        
        // Cheat state tracking
        bool m_infinite_jump_enabled = false;
        bool m_god_mode_enabled = false;

    public:
        StellarBladeMod() : CppUserModBase()
        {
            ModName = STR("StellarBlade Rewards");
            ModVersion = STR("1.0");
            ModDescription = STR("Stellar Blade reward dropping mod with GUI");
            ModAuthors = STR("Your Name");
        }

        auto on_unreal_init() -> void override
        {
            Output::send<LogLevel::Verbose>(STR("[INIT] Stellar Blade Rewards: Unreal Engine initialized!\n"));
            
            m_ModInitialized = true;
            Output::send<LogLevel::Verbose>(STR("[OK] Stellar Blade Rewards initialization complete! Check UE4SS GUI tab.\n"));
        }

        auto on_ui_init() -> void override
        {
            UE4SS_ENABLE_IMGUI();
            
            // Register ImGui tab after UI is initialized
            register_tab(STR("Stellar Blade Rewards"), [](CppUserModBase* instance) {
                auto mod = dynamic_cast<StellarBladeMod*>(instance);
                if (mod)
                {
                    mod->RenderModUI();
                }
            });
        }

        // Helper function to call Lua functions (from WuChang pattern)
        void CallLuaFunction(const std::string& function_name)
        {
            auto* lua_mod = UE4SSProgram::find_lua_mod_by_name(STR("SBRewardsMod"));
            if (lua_mod)
            {
                std::string lua_call = std::format("{}()", function_name);
                lua_mod->lua().execute_string(lua_call);
            }
            else
            {
                Output::send<LogLevel::Warning>(STR("Error: StellarBladeRewards Lua script not found!\n"));
            }
        }

        // Main UI rendering function like WuChang mod
        auto RenderModUI() -> void
        {
            ImGui::Text("Stellar Blade Cheats");
            ImGui::Separator();
            
            // Simple status display
            if (m_CheatManager)
            {
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "[OK] CheatManager Found");
            }
            
            // Rewards Section
            if (ImGui::CollapsingHeader("Rewards"))
            {
                if (ImGui::Button("Drop Sealed Gold", ImVec2(0, 30)))
                {
                    DropRewardSimple();
                }
                
                ImGui::Spacing();
                if (ImGui::Button("Drop VitCoins", ImVec2(0, 30)))
                {
                    DropVitCoins();
                }
                
                ImGui::Spacing();
                if (ImGui::Button("Drop Omnibolt", ImVec2(0, 30)))
                {
                    DropOmnibolt();
                }
                
                ImGui::Spacing();
                if (ImGui::Button("Drop Drone Upgrade Module", ImVec2(0, 30)))
                {
                    DropDroneModule();
                }
                
                ImGui::Spacing();
                if (ImGui::Button("Drop Tumbler Expansion Module", ImVec2(0, 30)))
                {
                    DropTumblerExpansionModule();
                }
                
                ImGui::Spacing();
                if (ImGui::Button("Give Skill Points", ImVec2(0, 30)))
                {
                    CallLuaFunction("GiveSkillPoints");
                }
                

                
                ImGui::Separator();
                ImGui::Text("Give specific reward:");
                ImGui::InputText("Group", m_custom_reward_input, sizeof(m_custom_reward_input));
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "(?)");
                if (ImGui::IsItemHovered())
                {
                    ImGui::SetTooltip("Enter reward group (e.g., 'b_elevator_Box12', 'subGroup_b_destruct_ditCoin')");
                }
                
                if (ImGui::Button("Drop Custom Reward", ImVec2(0, 30)))
                {
                    DropCustomReward();
                }
            }

            if (ImGui::CollapsingHeader("Cans"))
            {
                // Can Selection Section
                ImGui::Separator();
                ImGui::Text("Unlock Specific Can:");
                
                // Can dropdown
                std::string can_display = std::format("Can_{:03d}_RewardGroup", m_selected_can_index);
                if (ImGui::BeginCombo("##CanSelect", can_display.c_str()))
                {
                    for (int i = 1; i <= 49; i++)
                    {
                        bool is_selected = (m_selected_can_index == i);
                        std::string can_name = std::format("Can_{:03d}_RewardGroup", i);
                        if (ImGui::Selectable(can_name.c_str(), is_selected))
                        {
                            m_selected_can_index = i;
                        }
                        if (is_selected)
                        {
                            ImGui::SetItemDefaultFocus();
                        }
                    }
                    ImGui::EndCombo();
                }
                
                ImGui::Spacing();
                if (ImGui::Button("Unlock Selected Can", ImVec2(0, 30)))
                {
                    DropSelectedCan();
                }
                
                ImGui::Spacing();
                if (m_unlocking_all_cans)
                {
                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Unlocking All Cans... (%d/49)", m_current_unlock_can);
                    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Please wait 3 seconds between each can");
                }
                else
                {
                    if (ImGui::Button("Unlock All Cans (3s delay)", ImVec2(0, 30)))
                    {
                        StartUnlockAllCans();
                    }
                }
            }
            
            // Shops Section
            if (ImGui::CollapsingHeader("Shops"))
            {
                ImGui::Text("Select Shop:");
                
                // Shop dropdown
                if (ImGui::BeginCombo("##ShopSelect", m_shop_names[m_selected_shop_index].c_str()))
                {
                    for (int i = 0; i < m_shop_names.size(); i++)
                    {
                        bool is_selected = (m_selected_shop_index == i);
                        if (ImGui::Selectable(m_shop_names[i].c_str(), is_selected))
                        {
                            m_selected_shop_index = i;
                        }
                        if (is_selected)
                        {
                            ImGui::SetItemDefaultFocus();
                        }
                    }
                    ImGui::EndCombo();
                }
                
                // Show currency hint for selected shop
                auto& selected_shop = m_shop_names[m_selected_shop_index];
                auto currency_it = m_shop_currencies.find(selected_shop);
                if (currency_it != m_shop_currencies.end())
                {
                    ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "Currency: %s", currency_it->second.c_str());
                    ImGui::SameLine();
                    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "(find rewards with this alias)");
                }
                
                ImGui::Spacing();
                if (ImGui::Button("Open Selected Shop", ImVec2(0, 30)))
                {
                    OpenShop();
                }
            }

            if (ImGui::CollapsingHeader("Hair Salon"))
            {
                ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "Must be called from pause menu");
                ImGui::Spacing();

                if (ImGui::Button("Open Hair Salon", ImVec2(0, 30)))
                {
                    OpenHairSalon();
                }
            }
            
            // Warp to Camp Section
            if (ImGui::CollapsingHeader("Warp"))  
            {
                ImGui::Text("Select Camp:");
                
                // Hierarchical camp dropdown
                if (ImGui::BeginCombo("##CampSelect", m_selected_camp_display.c_str()))
                {
                    int global_index = 0;
                    for (const auto& region : m_hierarchical_camps)
                    {
                        // Region header (non-selectable)
                        ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.2f, 1.0f), "%s", region.first.c_str());
                        ImGui::Separator();
                        
                        // Full camps and their simple camps
                        for (const auto& full_camp_group : region.second)
                        {
                            bool is_selected = (m_selected_camp_index == global_index);
                            
                            // Full camp (main camp) - with emoji indicator
                            ImGui::Indent(16.0f);
                            std::string full_camp_display = "[CAMP] " + full_camp_group.main_camp.label;
                            if (ImGui::Selectable(full_camp_display.c_str(), is_selected))
                            {
                                m_selected_camp_index = global_index;
                                m_selected_camp_display = full_camp_group.main_camp.label;
                        }
                        if (is_selected)
                        {
                            ImGui::SetItemDefaultFocus();
                            }
                            ImGui::Unindent(16.0f);
                            global_index++;
                            
                            // Simple camps (checkpoints) under this full camp
                            for (const auto& simple_camp : full_camp_group.simple_camps)
                            {
                                bool simple_is_selected = (m_selected_camp_index == global_index);
                                
                                // Double indent for simple camps
                                ImGui::Indent(32.0f);
                                std::string simple_camp_display = "    > " + simple_camp.label;
                                if (ImGui::Selectable(simple_camp_display.c_str(), simple_is_selected))
                                {
                                    m_selected_camp_index = global_index;
                                    m_selected_camp_display = simple_camp.label;
                                }
                                if (simple_is_selected)
                                {
                                    ImGui::SetItemDefaultFocus();
                                }
                                ImGui::Unindent(32.0f);
                                global_index++;
                            }
                        }
                        
                        // Add spacing between regions (except for last region)
                        if (&region != &m_hierarchical_camps.back())
                        {
                            ImGui::Spacing();
                        }
                    }
                    ImGui::EndCombo();
                }
                
                ImGui::Spacing();
                if (ImGui::Button("Warp to Camp", ImVec2(0, 30)))
                {
                    WarpToCamp();
                }
            }
            
            // Trophies Section
            if (ImGui::CollapsingHeader("Trophies"))
            {
                ImGui::Text("Select Trophy:");
                
                // Trophy dropdown (only visible/non-hidden trophies)
                auto visible_trophies = GetVisibleTrophies();
                if (ImGui::BeginCombo("##TrophySelect", m_selected_trophy_display.c_str()))
                {
                    for (int i = 0; i < visible_trophies.size(); i++)
                    {
                        bool is_selected = (m_selected_trophy_index == i);
                        if (ImGui::Selectable(visible_trophies[i].key.c_str(), is_selected))
                        {
                            m_selected_trophy_index = i;
                            m_selected_trophy_display = visible_trophies[i].key;
                        }
                        if (is_selected)
                        {
                            ImGui::SetItemDefaultFocus();
                        }
                    }
                    ImGui::EndCombo();
                }
                
                ImGui::Spacing();
                if (ImGui::Button("Unlock Selected Trophy", ImVec2(0, 30)))
                {
                    UnlockSelectedTrophy();
                }
                
                ImGui::Spacing();
                if (ImGui::Button("Unlock All Trophies", ImVec2(0, 30)))
                {
                    UnlockAllTrophies();
                }
            }
            
            // Player Cheats Section
            if (ImGui::CollapsingHeader("Player Cheats"))
            {
                // Cheat status indicators
                if (m_infinite_jump_enabled)
                {
                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "[ON] Infinite Jump Enabled");
                }
                else
                {
                    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "[OFF] Infinite Jump Disabled");
                }
                
                if (m_god_mode_enabled)
                {
                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "[ON] God Mode Enabled");
                }
                else
                {
                    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "[OFF] God Mode Disabled");
                }
                
                ImGui::Separator();
                ImGui::Text("Movement & Player Modifications:");
                ImGui::Spacing();
                
                ImGui::TextColored(ImVec4(0.8f, 0.7f, 0.4f, 1.0f), "Note: You may need to re-enable Infinite Jump when new areas load or on player death");
                ImGui::Spacing();

                if (ImGui::Button("Infinite Jump", ImVec2(0, 30)))
                {
                    CallLuaFunction("InfiniteJump");
                    m_infinite_jump_enabled = true;
                }
                
                
                ImGui::Spacing();
                if (ImGui::Button("God Mode", ImVec2(0, 30)))
                {
                    CallLuaFunction("GodMod");
                    m_god_mode_enabled = true;
                }
            }
        }
        
        void OpenShop()
        {
            Output::send<LogLevel::Verbose>(STR("Opening shop: {}\n"), ensure_str(m_shop_names[m_selected_shop_index]));
            
            try
            {
                // Find CheatManager if we don't have it
                if (!m_CheatManager)
                {
                    m_CheatManager = UObjectGlobals::StaticFindObject(nullptr, nullptr, STR("/Script/SB.Default__SBCheatManager"));
                    if (!m_CheatManager)
                    {
                        Output::send<LogLevel::Warning>(STR("Could not find CheatManager\n"));
                        return;
                    }
                    Output::send<LogLevel::Verbose>(STR("Found CheatManager: {:p}\n"), static_cast<void*>(m_CheatManager));
                }
                
                // Build the command - SBUIOpenShop takes 3 parameters (all the same shop name like in Lua)
                auto& shop_name = m_shop_names[m_selected_shop_index];
                // Use ensure_str like LiveView does
                auto command_str = fmt::format(STR("SBUIOpenShop {} {} {}"), ensure_str(shop_name), ensure_str(shop_name), ensure_str(shop_name));
                
                // Log the command like LiveView does
                Output::send(STR("Queueing command: {}\n"), command_str);
                
                // Set up deferred execution (like UFunctionCallerWidget)
                s_shop_cmd = command_str;
                s_shop_instance = m_CheatManager;
                s_shop_executor = m_CheatManager;
                
                // Register hook if not already done
                static bool s_shop_is_hooked{};
                if (!s_shop_is_hooked)
                {
                    s_shop_is_hooked = true;
                    Hook::RegisterProcessEventPostCallback(call_shop_console_exec);
                }
                
                // Trigger deferred execution
                s_do_shop_call = true;
            }
            catch (...)
            {
                Output::send<LogLevel::Error>(STR("Exception opening shop\n"));
            }
        }
        
        void DropRewardSimple()
        {
            Output::send<LogLevel::Verbose>(STR("Button clicked - attempting simple reward drop...\n"));
            
            try
            {
                // Find CheatManager if we don't have it
                if (!m_CheatManager)
                {
                    m_CheatManager = UObjectGlobals::StaticFindObject(nullptr, nullptr, STR("/Script/SB.Default__SBCheatManager"));
                    if (!m_CheatManager)
                    {
                        Output::send<LogLevel::Warning>(STR("Could not find CheatManager\n"));
                        return;
                    }
                    Output::send<LogLevel::Verbose>(STR("Found CheatManager: {:p}\n"), static_cast<void*>(m_CheatManager));
                }
                
                // Simple ProcessConsoleExec call
                static FOutputDevice output_device{};
                FString command(STR("SBRewardGroupDropInven b_sealed_gold true"));
                
                bool success = m_CheatManager->ProcessConsoleExec(
                    command.GetCharArray(), 
                    output_device, 
                    m_CheatManager
                );
                
                Output::send<LogLevel::Verbose>(STR("Simple call result: {}\n"), success ? STR("SUCCESS") : STR("FAILED"));
            }
            catch (...)
            {
                Output::send<LogLevel::Error>(STR("Exception in simple reward drop\n"));
            }
        }
        
        void OpenHairSalon()
        {
            Output::send<LogLevel::Verbose>(STR("Button clicked - attempting to open hair salon...\n"));
            
            try
            {
                // Find CheatManager if we don't have it
                if (!m_CheatManager)
                {
                    m_CheatManager = UObjectGlobals::StaticFindObject(nullptr, nullptr, STR("/Script/SB.Default__SBCheatManager"));
                    if (!m_CheatManager)
                    {
                        Output::send<LogLevel::Warning>(STR("Could not find CheatManager\n"));
                        return;
                    }
                    Output::send<LogLevel::Verbose>(STR("Found CheatManager: {:p}\n"), static_cast<void*>(m_CheatManager));
                }
                
                // Use deferred execution like OpenShop (UI commands need this pattern)
                auto command_str = STR("SBUIOpenGameMenuToHairSalon");
                
                // Log the command
                Output::send(STR("Queueing command: {}\n"), command_str);
                
                // Set up deferred execution (like UFunctionCallerWidget and OpenShop)
                s_shop_cmd = command_str;
                s_shop_instance = m_CheatManager;
                s_shop_executor = m_CheatManager;
                
                // Register hook if not already done
                static bool s_shop_is_hooked{};
                if (!s_shop_is_hooked)
                {
                    s_shop_is_hooked = true;
                    Hook::RegisterProcessEventPostCallback(call_shop_console_exec);
                }
                
                // Trigger deferred execution
                s_do_shop_call = true;
            }
            catch (...)
            {
                Output::send<LogLevel::Error>(STR("Exception in SBUIOpenGameMenuToHairSalon\n"));
            }
        }
        
        // Helper function to get camp info by global index
        std::pair<std::string, std::string> GetCampByIndex(int global_index)
        {
            int current_index = 0;
            for (const auto& region : m_hierarchical_camps)
            {
                for (const auto& full_camp_group : region.second)
                {
                    // Check main camp
                    if (current_index == global_index)
                    {
                        return {full_camp_group.main_camp.id, full_camp_group.main_camp.label};
                    }
                    current_index++;
                    
                    // Check simple camps
                    for (const auto& simple_camp : full_camp_group.simple_camps)
                    {
                        if (current_index == global_index)
                        {
                            return {simple_camp.id, simple_camp.label};
                        }
                        current_index++;
                    }
                }
            }
            // Return first camp as fallback
            return {m_hierarchical_camps[0].second[0].main_camp.id, m_hierarchical_camps[0].second[0].main_camp.label};
        }
        
        
        void WarpToCamp()
        {
            auto [camp_id, camp_label] = GetCampByIndex(m_selected_camp_index);
            Output::send<LogLevel::Verbose>(STR("Warping to camp: {} ({})\n"), ensure_str(camp_label), ensure_str(camp_id));
            
            try
            {
                // Find CheatManager if we don't have it
                if (!m_CheatManager)
                {
                    m_CheatManager = UObjectGlobals::StaticFindObject(nullptr, nullptr, STR("/Script/SB.Default__SBCheatManager"));
                    if (!m_CheatManager)
                    {
                        Output::send<LogLevel::Warning>(STR("Could not find CheatManager\n"));
                        return;
                    }
                    Output::send<LogLevel::Verbose>(STR("Found CheatManager: {:p}\n"), static_cast<void*>(m_CheatManager));
                }
                
                // Build the SBWarpCamp command using the camp ID
                auto command_str = fmt::format(STR("SBWarpCamp {}"), ensure_str(camp_id));
                
                // Log the command
                Output::send(STR("Queueing command: {}\n"), command_str);
                
                // Set up deferred execution (like UFunctionCallerWidget and OpenShop)
                s_shop_cmd = command_str;
                s_shop_instance = m_CheatManager;
                s_shop_executor = m_CheatManager;
                
                // Register hook if not already done
                static bool s_shop_is_hooked{};
                if (!s_shop_is_hooked)
                {
                    s_shop_is_hooked = true;
                    Hook::RegisterProcessEventPostCallback(call_shop_console_exec);
                }
                
                // Trigger deferred execution
                s_do_shop_call = true;
            }
            catch (...)
            {
                Output::send<LogLevel::Error>(STR("Exception in WarpToCamp\n"));
            }
        }
        
        void DropVitCoins()
        {
            Output::send<LogLevel::Verbose>(STR("Button clicked - attempting VitCoins drop...\n"));
            
            try
            {
                // Find CheatManager if we don't have it
                if (!m_CheatManager)
                {
                    m_CheatManager = UObjectGlobals::StaticFindObject(nullptr, nullptr, STR("/Script/SB.Default__SBCheatManager"));
                    if (!m_CheatManager)
                    {
                        Output::send<LogLevel::Warning>(STR("Could not find CheatManager\n"));
                        return;
                    }
                    Output::send<LogLevel::Verbose>(STR("Found CheatManager: {:p}\n"), static_cast<void*>(m_CheatManager));
                }
                
                // Simple ProcessConsoleExec call for VitCoins
                static FOutputDevice output_device{};
                FString command(STR("SBRewardGroupDropInven subGroup_b_destruct_ditCoin true"));
                
                bool success = m_CheatManager->ProcessConsoleExec(
                    command.GetCharArray(), 
                    output_device, 
                    m_CheatManager
                );
                
                Output::send<LogLevel::Verbose>(STR("VitCoins drop result: {}\n"), success ? STR("SUCCESS") : STR("FAILED"));
            }
            catch (...)
            {
                Output::send<LogLevel::Error>(STR("Exception in VitCoins drop\n"));
            }
        }
        
        void DropOmnibolt()
        {
            Output::send<LogLevel::Verbose>(STR("Button clicked - attempting Omnibolt drop...\n"));
            
            try
            {
                // Find CheatManager if we don't have it
                if (!m_CheatManager)
                {
                    m_CheatManager = UObjectGlobals::StaticFindObject(nullptr, nullptr, STR("/Script/SB.Default__SBCheatManager"));
                    if (!m_CheatManager)
                    {
                        Output::send<LogLevel::Warning>(STR("Could not find CheatManager\n"));
                        return;
                    }
                    Output::send<LogLevel::Verbose>(STR("Found CheatManager: {:p}\n"), static_cast<void*>(m_CheatManager));
                }
                
                // Simple ProcessConsoleExec call for Omnibolt
                static FOutputDevice output_device{};
                FString command(STR("SBRewardGroupDropInven b_Xion_Box1 true"));
                
                bool success = m_CheatManager->ProcessConsoleExec(
                    command.GetCharArray(), 
                    output_device, 
                    m_CheatManager
                );
                
                Output::send<LogLevel::Verbose>(STR("Omnibolt drop result: {}\n"), success ? STR("SUCCESS") : STR("FAILED"));
            }
            catch (...)
            {
                Output::send<LogLevel::Error>(STR("Exception in Omnibolt drop\n"));
            }
        }
        
        void DropDroneModule()
        {
            Output::send<LogLevel::Verbose>(STR("Button clicked - attempting Drone Module drop...\n"));
            
            try
            {
                // Find CheatManager if we don't have it
                if (!m_CheatManager)
                {
                    m_CheatManager = UObjectGlobals::StaticFindObject(nullptr, nullptr, STR("/Script/SB.Default__SBCheatManager"));
                    if (!m_CheatManager)
                    {
                        Output::send<LogLevel::Warning>(STR("Could not find CheatManager\n"));
                        return;
                    }
                    Output::send<LogLevel::Verbose>(STR("Found CheatManager: {:p}\n"), static_cast<void*>(m_CheatManager));
                }
                
                // Simple ProcessConsoleExec call for Drone Module
                static FOutputDevice output_device{};
                FString command(STR("SBRewardGroupDropInven Records_LockedBox_DroneCore true"));
                
                bool success = m_CheatManager->ProcessConsoleExec(
                    command.GetCharArray(), 
                    output_device, 
                    m_CheatManager
                );
                
                Output::send<LogLevel::Verbose>(STR("Drone Module drop result: {}\n"), success ? STR("SUCCESS") : STR("FAILED"));
            }
            catch (...)
            {
                Output::send<LogLevel::Error>(STR("Exception in Drone Module drop\n"));
            }
        }

                
        void DropTumblerExpansionModule()
        {
            Output::send<LogLevel::Verbose>(STR("Button clicked - attempting Tumbler Expansion Module drop...\n"));
            
            try
            {
                // Find CheatManager if we don't have it
                if (!m_CheatManager)
                {
                    m_CheatManager = UObjectGlobals::StaticFindObject(nullptr, nullptr, STR("/Script/SB.Default__SBCheatManager"));
                    if (!m_CheatManager)
                    {
                        Output::send<LogLevel::Warning>(STR("Could not find CheatManager\n"));
                        return;
                    }
                    Output::send<LogLevel::Verbose>(STR("Found CheatManager: {:p}\n"), static_cast<void*>(m_CheatManager));
                }
                
                // Simple ProcessConsoleExec call for Drone Module
                static FOutputDevice output_device{};
                FString command(STR("SBRewardGroupDropInven e_tumbler_core_test true"));
                
                bool success = m_CheatManager->ProcessConsoleExec(
                    command.GetCharArray(), 
                    output_device, 
                    m_CheatManager
                );
                
                Output::send<LogLevel::Verbose>(STR("Tumbler Expansion Module drop result: {}\n"), success ? STR("SUCCESS") : STR("FAILED"));
            }
            catch (...)
            {
                Output::send<LogLevel::Error>(STR("Exception in Tumbler Expansion Module drop\n"));
            }
        }
        
        void DropSelectedCan()
        {
            std::string can_name = std::format("Can_{:03d}_RewardGroup", m_selected_can_index);
            Output::send<LogLevel::Verbose>(STR("Button clicked - attempting to unlock can: {}\n"), ensure_str(can_name));
            
            try
            {
                // Find CheatManager if we don't have it
                if (!m_CheatManager)
                {
                    m_CheatManager = UObjectGlobals::StaticFindObject(nullptr, nullptr, STR("/Script/SB.Default__SBCheatManager"));
                    if (!m_CheatManager)
                    {
                        Output::send<LogLevel::Warning>(STR("Could not find CheatManager\n"));
                        return;
                    }
                    Output::send<LogLevel::Verbose>(STR("Found CheatManager: {:p}\n"), static_cast<void*>(m_CheatManager));
                }
                
                // Build command with selected can
                std::string command_str = std::format("SBRewardGroupDropInven {} true", can_name);
                static FOutputDevice output_device{};
                FString command(ensure_str(command_str).c_str());
                
                bool success = m_CheatManager->ProcessConsoleExec(
                    command.GetCharArray(), 
                    output_device, 
                    m_CheatManager
                );
                
                Output::send<LogLevel::Verbose>(STR("Can unlock result: {}\n"), success ? STR("SUCCESS") : STR("FAILED"));
            }
            catch (...)
            {
                Output::send<LogLevel::Error>(STR("Exception in can unlock\n"));
            }
        }
        
        void StartUnlockAllCans()
        {
            if (m_unlocking_all_cans)
            {
                Output::send<LogLevel::Warning>(STR("Already unlocking all cans, please wait...\n"));
                return;
            }
            
            m_unlocking_all_cans = true;
            m_current_unlock_can = 1;
            
            Output::send<LogLevel::Verbose>(STR("Starting to unlock all cans with 3-second delays...\n"));
            
            // Start the unlocking process in a separate thread
            std::thread unlock_thread([this]() {
                UnlockAllCansThread();
            });
            unlock_thread.detach(); // Let it run independently
        }
        
        void UnlockAllCansThread()
        {
            try
            {
                for (int can_num = 1; can_num <= 49; can_num++)
                {
                    m_current_unlock_can = can_num;
                    
                    // Unlock this can
                    std::string can_name = std::format("Can_{:03d}_RewardGroup", can_num);
                    Output::send<LogLevel::Verbose>(STR("Unlocking can: {}\n"), ensure_str(can_name));
                    
                    // Find CheatManager if we don't have it
                    if (!m_CheatManager)
                    {
                        m_CheatManager = UObjectGlobals::StaticFindObject(nullptr, nullptr, STR("/Script/SB.Default__SBCheatManager"));
                        if (!m_CheatManager)
                        {
                            Output::send<LogLevel::Warning>(STR("Could not find CheatManager during unlock all\n"));
                            m_unlocking_all_cans = false;
                            return;
                        }
                    }
                    
                    // Execute the unlock command
                    std::string command_str = std::format("SBRewardGroupDropInven {} true", can_name);
                    static FOutputDevice output_device{};
                    FString command(ensure_str(command_str).c_str());
                    
                    bool success = m_CheatManager->ProcessConsoleExec(
                        command.GetCharArray(), 
                        output_device, 
                        m_CheatManager
                    );
                    
                    Output::send<LogLevel::Verbose>(STR("Can {} unlock result: {}\n"), can_num, success ? STR("SUCCESS") : STR("FAILED"));
                    
                    // Wait 3 seconds before next can (except for the last one)
                    if (can_num < 49)
                    {
                        std::this_thread::sleep_for(std::chrono::seconds(3));
                    }
                }
                
                Output::send<LogLevel::Verbose>(STR("Finished unlocking all 49 cans!\n"));
                m_unlocking_all_cans = false;
            }
            catch (...)
            {
                Output::send<LogLevel::Error>(STR("Exception during unlock all cans\n"));
                m_unlocking_all_cans = false;
            }
        }
        
        void DropCustomReward()
        {
            // Check if input is empty
            if (strlen(m_custom_reward_input) == 0)
            {
                Output::send<LogLevel::Warning>(STR("Custom reward input is empty\n"));
                return;
            }
            
            Output::send<LogLevel::Verbose>(STR("Button clicked - attempting custom reward drop: {}\n"), ensure_str(m_custom_reward_input));
            
            try
            {
                // Find CheatManager if we don't have it
                if (!m_CheatManager)
                {
                    m_CheatManager = UObjectGlobals::StaticFindObject(nullptr, nullptr, STR("/Script/SB.Default__SBCheatManager"));
                    if (!m_CheatManager)
                    {
                        Output::send<LogLevel::Warning>(STR("Could not find CheatManager\n"));
                        return;
                    }
                    Output::send<LogLevel::Verbose>(STR("Found CheatManager: {:p}\n"), static_cast<void*>(m_CheatManager));
                }
                
                // Build command with user input
                std::string command_str = std::format("SBRewardGroupDropInven {} true", m_custom_reward_input);
                static FOutputDevice output_device{};
                FString command(ensure_str(command_str).c_str());
                
                bool success = m_CheatManager->ProcessConsoleExec(
                    command.GetCharArray(), 
                    output_device, 
                    m_CheatManager
                );
                
                Output::send<LogLevel::Verbose>(STR("Custom reward drop result: {}\n"), success ? STR("SUCCESS") : STR("FAILED"));
            }
            catch (...)
            {
                Output::send<LogLevel::Error>(STR("Exception in custom reward drop\n"));
            }
        }
        
        void UnlockSelectedTrophy()
        {
            auto visible_trophies = GetVisibleTrophies();
            if (m_selected_trophy_index >= visible_trophies.size())
            {
                Output::send<LogLevel::Warning>(STR("Invalid trophy selection\n"));
                return;
            }
            
            const auto& selected_trophy = visible_trophies[m_selected_trophy_index];
            Output::send<LogLevel::Verbose>(STR("Button clicked - attempting to unlock trophy: {}\n"), ensure_str(selected_trophy.key));
            
            try
            {
                // Find CheatManager if we don't have it
                if (!m_CheatManager)
                {
                    m_CheatManager = UObjectGlobals::StaticFindObject(nullptr, nullptr, STR("/Script/SB.Default__SBCheatManager"));
                    if (!m_CheatManager)
                    {
                        Output::send<LogLevel::Warning>(STR("Could not find CheatManager\n"));
                        return;
                    }
                    Output::send<LogLevel::Verbose>(STR("Found CheatManager: {:p}\n"), static_cast<void*>(m_CheatManager));
                }
                
                // Build SBProgressTrophy command with trophy name and progress value 100
                auto command_str = fmt::format(STR("SBProgressTrophy {} 100"), ensure_str(selected_trophy.key));
                
                // Log the command
                Output::send(STR("Queueing command: {}\n"), command_str);
                
                // Set up deferred execution (like other shop/UI commands)
                s_shop_cmd = command_str;
                s_shop_instance = m_CheatManager;
                s_shop_executor = m_CheatManager;
                
                // Register hook if not already done
                static bool s_shop_is_hooked{};
                if (!s_shop_is_hooked)
                {
                    s_shop_is_hooked = true;
                    Hook::RegisterProcessEventPostCallback(call_shop_console_exec);
                }
                
                // Trigger deferred execution
                s_do_shop_call = true;
            }
            catch (...)
            {
                Output::send<LogLevel::Error>(STR("Exception in UnlockSelectedTrophy\n"));
            }
        }
        
        void UnlockAllTrophies()
        {
            Output::send<LogLevel::Verbose>(STR("Button clicked - attempting to unlock all trophies...\n"));
            
            try
            {
                // Find CheatManager if we don't have it
                if (!m_CheatManager)
                {
                    m_CheatManager = UObjectGlobals::StaticFindObject(nullptr, nullptr, STR("/Script/SB.Default__SBCheatManager"));
                    if (!m_CheatManager)
                    {
                        Output::send<LogLevel::Warning>(STR("Could not find CheatManager\n"));
                        return;
                    }
                    Output::send<LogLevel::Verbose>(STR("Found CheatManager: {:p}\n"), static_cast<void*>(m_CheatManager));
                }
                
                // Iterate through all trophies (including hidden ones) and unlock them
                for (const auto& trophy : m_trophies)
                {
                    Output::send<LogLevel::Verbose>(STR("Unlocking trophy: {}\n"), ensure_str(trophy.key));
                    
                    // Build SBProgressTrophy command with trophy name and progress value 100
                    auto command_str = fmt::format(STR("SBProgressTrophy {} 100"), ensure_str(trophy.key));
                    static FOutputDevice output_device{};
                    FString command(command_str.c_str());
                    
                    bool success = m_CheatManager->ProcessConsoleExec(
                        command.GetCharArray(), 
                        output_device, 
                        m_CheatManager
                    );
                    
                    Output::send<LogLevel::Verbose>(STR("Trophy {} unlock result: {}\n"), ensure_str(trophy.key), success ? STR("SUCCESS") : STR("FAILED"));
                }
                
                Output::send<LogLevel::Verbose>(STR("Finished unlocking all trophies!\n"));
            }
            catch (...)
            {
                Output::send<LogLevel::Error>(STR("Exception in UnlockAllTrophies\n"));
            }
        }

        void FindCheatManager()
        {
            if (m_CheatManager)
            {
                Output::send<LogLevel::Verbose>(STR("CheatManager already cached\n"));
                return;
            }
            
            // Search silently for SBCheatManager - minimal logging to avoid clutter
            UObjectGlobals::ForEachUObject([&](UObject* object, ...) {
                if (!object) return LoopAction::Continue;
                
                try 
                {
                    auto object_name = object->GetFullName();
                    if (object_name.find(STR("Default__SBCheatManager")) != std::string::npos)
                    {
                        m_CheatManager = object;
                        Output::send<LogLevel::Verbose>(STR("Found CheatManager at: {:p}\n"), static_cast<void*>(m_CheatManager));
                        return LoopAction::Break; // Stop searching once found
                    }
                }
                catch (...)
                {
                    // Safely ignore any problematic objects
                }
                
                return LoopAction::Continue;
            });
            
            if (!m_CheatManager)
            {
                Output::send<LogLevel::Warning>(STR("CheatManager not found\n"));
            }
        }

        void CallRewardFunction()
        {
            if (!m_CheatManager) return;
            
            try
            {
                // Get the function from the instance like Live View does - NOT StaticFindObject!
                auto function = m_CheatManager->GetFunctionByNameInChain(STR("SBRewardGroupDropInven"));
                if (!function)
                {
                    Output::send<LogLevel::Warning>(STR("Function not found on instance\n"));
                    return;
                }
                
                // Use EXACTLY the same pattern as UFunctionCallerWidget::call_selected_function
                FString cmd = FString(STR("SBRewardGroupDropInven b_sealed_gold true"));
                
                Output::send(STR("Queueing command: {}\n"), cmd.GetCharArray());
                
                s_cmd = cmd;
                s_instance = m_CheatManager;
                s_function = function;
                s_executor = m_CheatManager; // context_is_implicit = true for cheat manager
                
                static bool s_is_hooked{};
                if (!s_is_hooked)
                {
                    s_is_hooked = true;
                    Hook::RegisterProcessEventPostCallback(call_process_console_exec);
                }
                s_do_call = true;
            }
            catch (...)
            {
                Output::send<LogLevel::Error>(STR("Exception in CallRewardFunction\n"));
            }
        }

        ~StellarBladeMod() override
        {
            Output::send<LogLevel::Verbose>(STR("[SHUTDOWN] NewModExample shutting down\n"));
        }
    };

    #define STELLAR_BLADE_MOD_API __declspec(dllexport)
    extern "C"
    {
        STELLAR_BLADE_MOD_API RC::CppUserModBase* start_mod()
        {
            return new StellarBladeMod();
        }

        STELLAR_BLADE_MOD_API void uninstall_mod(RC::CppUserModBase* mod)
        {
            delete mod;
        }
    }
