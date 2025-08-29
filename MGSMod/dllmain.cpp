#define NOMINMAX
#include <Windows.h>
#include <Mod/CppUserModBase.hpp>
#include <DynamicOutput/DynamicOutput.hpp>
#include <Unreal/Core/Containers/Map.hpp>
#include "MGS3Types.hpp"
#include <Unreal/UObjectGlobals.hpp>
#include <Unreal/UObject.hpp>
#include <Unreal/FProperty.hpp>
#include <Unreal/Property/FStructProperty.hpp>
#include <Unreal/UScriptStruct.hpp>
#include <UE4SSProgram.hpp>
#include <imgui.h>
#include <string>
#include <format>
#include <chrono>
#include <unordered_map>
#include "UE4SSUtils.hpp" // Include our utility library

using namespace RC;
using namespace RC::Unreal;

class MGSModMod : public RC::CppUserModBase
{
private:
    bool m_ModInitialized = false;
    UObject* m_Player = nullptr;
    
    // Notification system
    struct Notification {
        std::string message;
        ImVec4 color;
        std::chrono::steady_clock::time_point start_time;
        bool active = false;
    };
    
    std::unordered_map<std::string, Notification> m_notifications;
    
    // Notification helper methods
    void ShowNotification(const std::string& id, const std::string& message, ImVec4 color = ImVec4(0.0f, 1.0f, 0.0f, 1.0f))
    {
        m_notifications[id] = {message, color, std::chrono::steady_clock::now(), true};
    }
    
    void RenderNotification(const std::string& id)
    {
        auto it = m_notifications.find(id);
        if (it != m_notifications.end() && it->second.active)
        {
            auto& notif = it->second;
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - notif.start_time).count();
            
            // Show for 3 seconds with fade effect
            if (elapsed < 3000)
            {
                float alpha = 1.0f;
                if (elapsed > 2000) // Start fading after 2 seconds
                {
                    alpha = 1.0f - ((elapsed - 2000) / 1000.0f);
                }
                
                ImVec4 faded_color = notif.color;
                faded_color.w = alpha;
                
                ImGui::TextColored(faded_color, "%s", notif.message.c_str());
            }
            else
            {
                notif.active = false; // Hide after 3 seconds
            }
        }
    }

public:
    MGSModMod() : CppUserModBase()
    {
        ModName = STR("MGSMod");
        ModVersion = STR("1.0");
        ModDescription = STR("MGS Delta C++ Mod");
        ModAuthors = STR("MGS Modder");
    }

    auto on_ui_init() -> void override
    {
        UE4SS_ENABLE_IMGUI();
        
        // Register ImGui tab after UI is initialized
        register_tab(STR("MGSMod"), [](CppUserModBase* instance) {
            auto mod = dynamic_cast<MGSModMod*>(instance);
            if (mod)
            {
                mod->RenderMGSModUI();
            }
        });
    }

    auto on_unreal_init() -> void override
    {
        Output::send<LogLevel::Verbose>(STR("MGSModMod: Unreal Engine initialized!\n"));
        
        // Initialize UE4SSUtils
        if (!UE4SSUtils::Initialize())
        {
            Output::send<LogLevel::Error>(STR("Failed to initialize UE4SSUtils!\n"));
            return;
        }
        Output::send<LogLevel::Verbose>(STR("UE4SSUtils initialized successfully!\n"));
        
        // Try to find Player on initialization
        FindPlayer();
        
        m_ModInitialized = true;
        Output::send<LogLevel::Verbose>(STR("MGSModMod initialization complete!\n"));
    }

    auto RenderMGSModUI() -> void
    {
        ImGui::Text("MGS Delta Mod");
        ImGui::Separator();
        
        if (ImGui::CollapsingHeader("Save Game Manipulation"))
            {
                ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "Direct save data modification");
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.6f, 1.0f), "ℹ️  What gets unlocked:");
                ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "   • Extras Menu: Theaters, viewers (immediate after menu refresh)");
                ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "   • New Game: Camouflage, items from completion bonuses");
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.6f, 1.0f), "⚠️  To save changes:");
                ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "   • In-game: Enter new area to trigger save");
                ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "   • Main menu: Enter/exit any menu to refresh");
                ImGui::Spacing();
            
            // Camouflage Collection Section
            if (ImGui::CollapsingHeader("Camouflage Collection"))
            {
                ImGui::TextColored(ImVec4(0.8f, 0.9f, 0.6f, 1.0f), "Unlock camouflage, face paint, and collection viewer");
                ImGui::Spacing();
                
                if (ImGui::Button("🔓 Uniform (Camouflage)", ImVec2(0, 30)))
                {
                    UnlockAllCamouflage();
                }
                RenderNotification("unlock_camo");
                if (ImGui::IsItemHovered())
                {
                    ImGui::BeginTooltip();
                    ImGui::Text("Unlocks all camouflage uniforms for gameplay use");
                    ImGui::Text("Updates: CamouflageList + UnlockCamouflageMap");
                    ImGui::EndTooltip();
                }
                
                if (ImGui::Button("🎨 Face (Face Paint)", ImVec2(0, 30)))
                {
                    UnlockFacePaintMap();
                }
                RenderNotification("unlock_facepaint");
                if (ImGui::IsItemHovered())
                {
                    ImGui::BeginTooltip();
                    ImGui::Text("Unlocks all face paint patterns for gameplay use");
                    ImGui::Text("Updates: FacePaintList + UnlockFacePaintMap");
                    ImGui::EndTooltip();
                }
                
                if (ImGui::Button("🎯 Item (Collection Viewer)", ImVec2(0, 30)))
                {
                    UnlockCamouflageCollectionViewer();
                }
                RenderNotification("unlock_viewer");
                if (ImGui::IsItemHovered())
                {
                    ImGui::BeginTooltip();
                    ImGui::Text("Unlocks all items for viewing in collection/museum mode");
                    ImGui::Text("Updates: UnlockCamouflageCollectionViewerMap");
                    ImGui::EndTooltip();
                }
            }
            
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            
            if (ImGui::Button("🏆 Unlock All Rewards", ImVec2(0, 30)))
            {
                UnlockAllRewards();
            }
            RenderNotification("unlock_rewards");
            if (ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();
                ImGui::Text("Unlocks all achievement rewards");
                ImGui::Text("Updates: RewardsGained");
                ImGui::EndTooltip();
            }
            
            if (ImGui::Button("🎬 Unlock Secret Theater", ImVec2(0, 30)))
            {
                UnlockSecretTheater();
            }
            RenderNotification("unlock_theater");
            if (ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();
                ImGui::Text("Unlocks all Secret Theater videos and dialogs");
                ImGui::Text("Sets UnlockSecretTheaterMap to Acquired and SecretTheaterDialogMap to true");
                ImGui::EndTooltip();
            }
            
            if (ImGui::Button("🐸 Unlock All Kerotan", ImVec2(0, 30)))
            {
                UnlockAllKerotan();
            }
            RenderNotification("unlock_kerotan");
            if (ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();
                ImGui::Text("Marks all Kerotan frogs as collected");
                ImGui::Text("Sets all KerotanList entries to true");
                ImGui::EndTooltip();
            }
            
            if (ImGui::Button("🦎 Unlock All Gako", ImVec2(0, 30)))
            {
                UnlockAllGako();
            }
            RenderNotification("unlock_gako");
            if (ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();
                ImGui::Text("Marks all Gako lizards as collected");
                ImGui::Text("Sets all GakoList entries to true");
                ImGui::EndTooltip();
            }
            
            if (ImGui::Button("🏆 Unlock All Record Titles", ImVec2(0, 30)))
            {
                UnlockAllRecordTitles();
            }
            RenderNotification("unlock_titles");
            if (ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();
                ImGui::Text("Unlocks all record titles and rankings");
                ImGui::Text("Sets all RecordTitleData entries to ACQUIRED");
                ImGui::EndTooltip();
            }
            
            if (ImGui::Button("👥 Unlock Character Viewer", ImVec2(0, 30)))
            {
                UnlockCharacterViewer();
            }
            RenderNotification("unlock_character");
            if (ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();
                ImGui::Text("Unlocks the Character Viewer in extras menu");
                ImGui::Text("Sets UnlockCharacterViewerStatus to Acquired");
                ImGui::EndTooltip();
            }
            
            if (ImGui::Button("🎮 Unlock Guy Savage", ImVec2(0, 30)))
            {
                UnlockGuysavage();
            }
            RenderNotification("unlock_guy");
            if (ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();
                ImGui::Text("Unlocks Guy Savage mini-game from main menu");
                ImGui::Text("Sets bHasCleared to true");
                ImGui::EndTooltip();
            }
            
            if (ImGui::Button("🎬 Unlock Demo Theater", ImVec2(0, 30)))
            {
                UnlockDemoTheater();
            }
            RenderNotification("unlock_demo");
            if (ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();
                ImGui::Text("Unlocks all demo theater content and cutscenes");
                ImGui::Text("Sets DemoTheaterOpenFlags + DemoTheaterInitialFlagList to true");
                ImGui::EndTooltip();
            }
            
            if (ImGui::Button("💀 Unlock European Extreme", ImVec2(0, 30)))
            {
                UnlockEuropeanExtreme();
            }
            RenderNotification("unlock_extreme");
            if (ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();
                ImGui::Text("Unlocks European Extreme difficulty level");
                ImGui::Text("Sets bHasClearedExtremeDifficulty to true");
                ImGui::EndTooltip();
            }
        }
        
        if (ImGui::CollapsingHeader("Steam Achievements"))
        {
            ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "DANGER: This will unlock ALL Steam achievements!");
            ImGui::TextColored(ImVec4(0.9f, 0.6f, 0.6f, 1.0f), "This action cannot be undone and affects your Steam profile.");
            ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "Only use if you understand the consequences.");
            ImGui::Spacing();
            
            if (ImGui::Button("Unlock All Achievements", ImVec2(0, 30)))
            {
                UnlockAllAchievements();
            }
            RenderNotification("unlock_achievements");
            if (ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();
                ImGui::Text("WARNING: This unlocks ALL Steam achievements!");
                ImGui::Text("Sets all achievements to 100% completion");
                ImGui::Text("Updates Progress and Percent to 1.0 for all AchievementData entries");
                ImGui::Text("This action is permanent and cannot be undone!");
                ImGui::EndTooltip();
            }
        }
        
        if (ImGui::CollapsingHeader("Equip Items"))
        {
            ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "Direct item equipping");
            ImGui::Spacing();
            
            if (ImGui::Button("IT_Stealth (23)", ImVec2(0, 30)))
            {
                SetItemById(23);
            }
            RenderNotification("equip_stealth");

            if (ImGui::Button("IT_ATCamo (26)", ImVec2(0, 30)))
            {
                SetItemById(26);
            }
            RenderNotification("equip_atcamo");
        }
    }

    void FindPlayer()
    {
        Output::send<LogLevel::Verbose>(STR("🎯 Finding Player using UE4SSUtils...\n"));
        
        // Clear cached player to force fresh search
        m_Player = nullptr;
        
        // Use UE4SSUtils to find player with MGS-specific class
        m_Player = UE4SSUtils::FindPlayer("AGsrPlayer");
        
        if (m_Player)
        {
            Output::send<LogLevel::Verbose>(STR("✅ Player found using utils!\n"));
            // Optional: Log detailed player info for debugging
            UE4SSUtils::LogObjectInfo(m_Player, false);
        }
        else
        {
            Output::send<LogLevel::Warning>(STR("❌ Player not found with utils\n"));
        }
    }
    
    void SetItemById(int itemId)
    {
        Output::send<LogLevel::Verbose>(STR("🎒 Setting Item ID {} using UE4SSUtils...\n"), itemId);
        
        try
        {
            // Ensure player is found
            if (!m_Player)
            {
                FindPlayer();
                if (!m_Player) return;
            }
            
            // Find equipment controller using utils
            UE4SSUtils::ObjectSearchConfig config;
            config.ClassName = "BP_GsrEquipController_C";
            config.NameContains = "BP_Player_C";
            config.MaxResults = 1;
            
            auto controllers = UE4SSUtils::FindObjects(config);
            if (controllers.empty())
            {
                Output::send<LogLevel::Warning>(STR("Equipment controller not found with utils\n"));
                return;
            }
            
            auto equipController = controllers[0];
            
            // Build command
            std::string command = std::format("SetCurrentItemId {}", itemId);
            
            Output::send<LogLevel::Verbose>(STR("Setting item to: {} using utils\n"), itemId);
            
            // Execute command using utils
            bool success = UE4SSUtils::ExecuteConsoleCommand(command, equipController);
            if (success)
            {
                Output::send<LogLevel::Verbose>(STR("✅ Item {} command queued successfully!\n"), itemId);
                std::string notificationId = (itemId == 23) ? "equip_stealth" : 
                                            (itemId == 26) ? "equip_atcamo" : 
                                            "equip_item";
                std::string itemName = (itemId == 23) ? "IT_Stealth" : 
                                      (itemId == 26) ? "IT_ATCamo" : 
                                      std::format("Item {}", itemId);
                ShowNotification(notificationId, std::format("{} equipped!", itemName), ImVec4(0.6f, 0.8f, 1.0f, 1.0f));
            }
            else
            {
                Output::send<LogLevel::Warning>(STR("❌ Failed to queue item {} command\n"), itemId);
                std::string notificationId = (itemId == 23) ? "equip_stealth" : 
                                            (itemId == 26) ? "equip_atcamo" : 
                                            "equip_item";
                ShowNotification(notificationId, "Failed to equip item", ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
            }
        }
        catch (...)
        {
            Output::send<LogLevel::Error>(STR("Exception in SetItemById for item {}\n"), itemId);
        }
    }
    
    void UnlockAllCamouflage()
    {
        Output::send<LogLevel::Verbose>(STR("🎨 Unlocking All Camouflage using UE4SSUtils patterns...\n"));
        
        try 
        {
            // Use UE4SSUtils to find UserProfileSaveGame objects
            UE4SSUtils::ObjectSearchConfig config;
            config.ClassName = "UserProfileSaveGame";
            config.RequireValid = true;
            config.MaxResults = 5;
            
            auto foundSaveGames = UE4SSUtils::FindObjects(config);
            if (foundSaveGames.empty())
            {
                Output::send<LogLevel::Warning>(STR("❌ No UserProfileSaveGame found\n"));
                return;
            }
            
            bool unlockSuccess = false;
            for (auto* saveGame : foundSaveGames)
            {
                auto saveGameName = saveGame->GetFullName();
                Output::send<LogLevel::Verbose>(STR("Processing save game: {}\n"), saveGameName);
                
                int totalUnlocked = 0;
                
                // Step 1: Update CamouflageList TArray<boolean>
                auto camoListResult = UE4SSUtils::GetProperty(saveGame, "CamouflageList");
                if (camoListResult.Found)
                {
                    Output::send<LogLevel::Verbose>(STR("Found CamouflageList TArray, unlocking...\n"));
                    
                    auto camoArray = UE4SSUtils::GetTArray<bool>(saveGame, "CamouflageList");
                    if (camoArray && camoArray->ArrayNum > 0)
                    {
                        for (int i = 0; i < camoArray->ArrayNum; i++)
                        {
                            camoArray->Data[i] = true; // Set to unlocked
                            totalUnlocked++;
                        }
                        Output::send<LogLevel::Verbose>(STR("✅ Updated {} entries in CamouflageList (Array size: {})\n"), totalUnlocked, camoArray->ArrayNum);
                    }
                }
                
                // Step 2: Update UnlockCamouflageMap TMap<ECamouflageType, EGsrExtraAcquiredStatus>
                auto unlockMapResult = UE4SSUtils::GetProperty(saveGame, "UnlockCamouflageMap");
                if (unlockMapResult.Found && unlockMapResult.DataPtr)
                {
                    Output::send<LogLevel::Verbose>(STR("Found UnlockCamouflageMap TMap, unlocking...\n"));
                    auto unlockMap = static_cast<TMap<ECamouflageType, EGsrExtraAcquiredStatus>*>(unlockMapResult.DataPtr);
                    if (unlockMap)
                    {
                        int mapUnlocked = 0;
                        
                        Output::send<LogLevel::Verbose>(STR("Map contains {} camouflage entries, unlocking all...\n"), unlockMap->Num());
                        
                        // Simply set all entries to Acquired - map already contains all needed entries
                        for (auto& pair : *unlockMap) 
                        { 
                            Output::send<LogLevel::Verbose>(STR("Key: {}, Old Value: {}, Setting to: Acquired\n"), 
                                static_cast<int>(pair.Key), static_cast<int>(pair.Value));
                            pair.Value = EGsrExtraAcquiredStatus::Acquired;
                            mapUnlocked++; 
                        }
                        
                        Output::send<LogLevel::Verbose>(STR("✅ Set {} entries to Acquired in UnlockCamouflageMap\n"), mapUnlocked);
                        totalUnlocked += mapUnlocked;
                    }
                }
                
                if (totalUnlocked > 0)
                {
                    Output::send<LogLevel::Verbose>(STR("✅ Successfully updated both CamouflageList and UnlockCamouflageMap\n"));
                    ShowNotification("unlock_camo", "All camouflage unlocked!", ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
                    unlockSuccess = true;
                    break; // Exit the save game loop since we succeeded
                }
                
                // Simple fallback if CamouflageList wasn't found
                if (!unlockSuccess)
                {
                    Output::send<LogLevel::Verbose>(STR("CamouflageList not found, trying simple console command...\n"));
                    if (UE4SSUtils::ExecuteConsoleCommand("UnlockAllCamouflage", saveGame))
                    {
                        unlockSuccess = true;
                        break;
                    }
                }
            }
            
            if (!unlockSuccess)
            {
                Output::send<LogLevel::Warning>(STR("❌ All unlock methods failed\n"));
            }
        }
        catch (const std::exception& e)
        {
            Output::send<LogLevel::Error>(STR("❌ Error unlocking camouflage: {}\n"), to_wstring(e.what()));
        }
    }
    
    void UnlockFacePaintMap()
    {
        Output::send<LogLevel::Verbose>(STR("🎨 Unlocking All Face Paint using UE4SSUtils patterns...\n"));
        try 
        {
            UE4SSUtils::ObjectSearchConfig config;
            config.ClassName = "UserProfileSaveGame";
            config.RequireValid = true;
            config.MaxResults = 5;
            auto foundSaveGames = UE4SSUtils::FindObjects(config);
            if (foundSaveGames.empty()) 
            { 
                Output::send<LogLevel::Warning>(STR("❌ No UserProfileSaveGame objects found\n"));
                return; 
            }
            
            bool unlockSuccess = false;
            for (auto* saveGame : foundSaveGames)
            {
                auto saveGameName = saveGame->GetFullName();
                Output::send<LogLevel::Verbose>(STR("Processing save game: {}\n"), saveGameName);
                
                int totalUnlocked = 0;
                
                // Update FacePaintList TArray<boolean> - set all to true
                auto facePaintListResult = UE4SSUtils::GetProperty(saveGame, "FacePaintList");
                if (facePaintListResult.Found)
                {
                    Output::send<LogLevel::Verbose>(STR("Found FacePaintList TArray, unlocking...\n"));
                    
                    auto facePaintArray = UE4SSUtils::GetTArray<bool>(saveGame, "FacePaintList");
                    if (facePaintArray && facePaintArray->ArrayNum > 0)
                    {
                        for (int i = 0; i < facePaintArray->ArrayNum; i++)
                        {
                            facePaintArray->Data[i] = true; // Set to unlocked
                            totalUnlocked++;
                        }
                        Output::send<LogLevel::Verbose>(STR("✅ Updated {} entries in FacePaintList (Array size: {})\n"), totalUnlocked, facePaintArray->ArrayNum);
                    }
                }
                
                // Update UnlockFacePaintMap TMap<EFacePaintType, EGsrExtraAcquiredStatus>
                auto unlockMapResult = UE4SSUtils::GetProperty(saveGame, "UnlockFacePaintMap");
                if (unlockMapResult.Found && unlockMapResult.DataPtr)
                {
                    Output::send<LogLevel::Verbose>(STR("Found UnlockFacePaintMap TMap, unlocking...\n"));
                    auto unlockMap = static_cast<TMap<EFacePaintType, EGsrExtraAcquiredStatus>*>(unlockMapResult.DataPtr);
                    if (unlockMap)
                    {
                        int mapUnlocked = 0;
                        Output::send<LogLevel::Verbose>(STR("UnlockFacePaintMap size: {}\n"), unlockMap->Num());
                        
                        // Set all face paint entries to Acquired (1)
                        for (auto& pair : *unlockMap) 
                        { 
                            Output::send<LogLevel::Verbose>(STR("Setting face paint {} (was: {}) to: Acquired\n"), 
                                static_cast<int>(pair.Key), static_cast<int>(pair.Value));
                            pair.Value = EGsrExtraAcquiredStatus::Acquired;
                            mapUnlocked++;
                        }
                        Output::send<LogLevel::Verbose>(STR("✅ Updated {} entries in UnlockFacePaintMap\n"), mapUnlocked);
                        totalUnlocked += mapUnlocked;
                    }
                }
                else
                {
                    Output::send<LogLevel::Warning>(STR("UnlockFacePaintMap not found\n"));
                }
                
                if (totalUnlocked > 0)
                {
                    Output::send<LogLevel::Verbose>(STR("✅ Successfully updated FacePaintList and UnlockFacePaintMap\n"));
                    ShowNotification("unlock_facepaint", "All face paint unlocked!", ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
                    unlockSuccess = true;
                    break; // Exit the save game loop since we succeeded
                }
            }
            
            if (!unlockSuccess)
            {
                Output::send<LogLevel::Warning>(STR("❌ FacePaintList or UnlockFacePaintMap not found or empty\n"));
            }
        }
        catch (const std::exception& e)
        {
            Output::send<LogLevel::Error>(STR("❌ Error unlocking face paint: {}\n"), to_wstring(e.what()));
        }
    }
    
    void UnlockCamouflageCollectionViewer()
    {
        Output::send<LogLevel::Verbose>(STR("🎯 Unlocking Camouflage Collection Viewer using UE4SSUtils patterns...\n"));
        try 
        {
            UE4SSUtils::ObjectSearchConfig config;
            config.ClassName = "UserProfileSaveGame";
            config.RequireValid = true;
            config.MaxResults = 5;
            auto foundSaveGames = UE4SSUtils::FindObjects(config);
            if (foundSaveGames.empty()) 
            { 
                Output::send<LogLevel::Warning>(STR("❌ No UserProfileSaveGame objects found\n"));
                return;
            }
            
            bool unlockSuccess = false;
            for (auto* saveGame : foundSaveGames)
            {
                auto saveGameName = saveGame->GetFullName();
                Output::send<LogLevel::Verbose>(STR("Processing save game: {}\n"), saveGameName);
                
                int totalUnlocked = 0;
                
                // Update UnlockCamouflageCollectionViewerMap TMap<EItemName, EGsrExtraAcquiredStatus>
                auto unlockMapResult = UE4SSUtils::GetProperty(saveGame, "UnlockCamouflageCollectionViewerMap");
                if (unlockMapResult.Found && unlockMapResult.DataPtr)
                {
                    Output::send<LogLevel::Verbose>(STR("Found UnlockCamouflageCollectionViewerMap TMap, unlocking...\n"));
                    auto unlockMap = static_cast<TMap<EItemName, EGsrExtraAcquiredStatus>*>(unlockMapResult.DataPtr);
                    if (unlockMap)
                    {
                        int mapUnlocked = 0;
                        Output::send<LogLevel::Verbose>(STR("UnlockCamouflageCollectionViewerMap size: {}\n"), unlockMap->Num());
                        
                        // Set all collection viewer entries to Acquired (1)
                        for (auto& pair : *unlockMap) 
                        { 
                            Output::send<LogLevel::Verbose>(STR("Setting collection item {} (was: {}) to: Acquired\n"), 
                                static_cast<int>(pair.Key), static_cast<int>(pair.Value));
                            pair.Value = EGsrExtraAcquiredStatus::Acquired;
                            mapUnlocked++;
                        }
                        Output::send<LogLevel::Verbose>(STR("✅ Updated {} entries in UnlockCamouflageCollectionViewerMap\n"), mapUnlocked);
                        totalUnlocked = mapUnlocked;
                    }
                }
                else
                {
                    Output::send<LogLevel::Warning>(STR("UnlockCamouflageCollectionViewerMap not found\n"));
                }
                
                if (totalUnlocked > 0)
                {
                    Output::send<LogLevel::Verbose>(STR("✅ Successfully updated UnlockCamouflageCollectionViewerMap\n"));
                    ShowNotification("unlock_viewer", "Collection viewer unlocked!", ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
                    unlockSuccess = true;
                    break; // Exit the save game loop since we succeeded
                }
            }
            
            if (!unlockSuccess)
            {
                Output::send<LogLevel::Warning>(STR("❌ UnlockCamouflageCollectionViewerMap not found or empty\n"));
            }
        }
        catch (const std::exception& e)
        {
            Output::send<LogLevel::Error>(STR("❌ Error unlocking collection viewer: {}\n"), to_wstring(e.what()));
        }
    }
    

    
    void UnlockAllRewards()
    {
        Output::send<LogLevel::Verbose>(STR("🏆 Unlocking All Rewards (RewardsGained + RewardsDisplayed + ConditionsAttained)...\n"));
        try 
        {
            UE4SSUtils::ObjectSearchConfig config;
            config.ClassName = "UserProfileSaveGame";
            config.RequireValid = true;
            config.MaxResults = 5;
            auto foundSaveGames = UE4SSUtils::FindObjects(config);
            if (foundSaveGames.empty()) 
            { 
                Output::send<LogLevel::Warning>(STR("❌ No UserProfileSaveGame objects found\n"));
                return;
            }
            
            bool unlockSuccess = false;
            for (auto* saveGame : foundSaveGames)
            {
                auto saveGameName = saveGame->GetFullName();
                Output::send<LogLevel::Verbose>(STR("Processing save game: {}\n"), saveGameName);
                
                int totalUnlocked = 0;
                
                // Update RewardsGained TMap<ECobraRewardItem, bool>
                void* rewardsGainedDataPtr = UE4SSUtils::GetStructTMapProperty<ECobraRewardItem, bool>(saveGame, "RewardsData", "RewardsGained");
                if (rewardsGainedDataPtr)
                {
                    Output::send<LogLevel::Verbose>(STR("Found RewardsGained TMap, setting all to true...\n"));
                    auto rewardsGainedMap = static_cast<TMap<ECobraRewardItem, bool>*>(rewardsGainedDataPtr);
                    if (rewardsGainedMap)
                    {
                        int rewardsGainedUnlocked = 0;
                        Output::send<LogLevel::Verbose>(STR("RewardsGained map size: {}\n"), rewardsGainedMap->Num());
                        
                        for (auto& pair : *rewardsGainedMap) 
                        { 
                            Output::send<LogLevel::Verbose>(STR("Setting RewardsGained {} (was: {}) to: true\n"), 
                                static_cast<int>(pair.Key), pair.Value);
                            pair.Value = true;
                            rewardsGainedUnlocked++;
                        }
                        Output::send<LogLevel::Verbose>(STR("✅ Set {} rewards to true in RewardsGained\n"), rewardsGainedUnlocked);
                        totalUnlocked += rewardsGainedUnlocked;
                    }
                }
                
                // Update RewardsDisplayed TMap<ECobraRewardItem, bool>
                void* rewardsDisplayedDataPtr = UE4SSUtils::GetStructTMapProperty<ECobraRewardItem, bool>(saveGame, "RewardsData", "RewardsDisplayed");
                if (rewardsDisplayedDataPtr)
                {
                    Output::send<LogLevel::Verbose>(STR("Found RewardsDisplayed TMap, setting all to true...\n"));
                    auto rewardsDisplayedMap = static_cast<TMap<ECobraRewardItem, bool>*>(rewardsDisplayedDataPtr);
                    if (rewardsDisplayedMap)
                    {
                        int rewardsDisplayedUnlocked = 0;
                        Output::send<LogLevel::Verbose>(STR("RewardsDisplayed map size: {}\n"), rewardsDisplayedMap->Num());
                        
                        for (auto& pair : *rewardsDisplayedMap) 
                        { 
                            Output::send<LogLevel::Verbose>(STR("Setting RewardsDisplayed {} (was: {}) to: true\n"), 
                                static_cast<int>(pair.Key), pair.Value);
                            pair.Value = true;
                            rewardsDisplayedUnlocked++;
                        }
                        Output::send<LogLevel::Verbose>(STR("✅ Set {} rewards to true in RewardsDisplayed\n"), rewardsDisplayedUnlocked);
                        totalUnlocked += rewardsDisplayedUnlocked;
                    }
                }
                
                // Update ConditionsAttained TMap<ERewardCondition, bool>
                void* conditionsAttainedDataPtr = UE4SSUtils::GetStructTMapProperty<ERewardCondition, bool>(saveGame, "RewardsData", "ConditionsAttained");
                if (conditionsAttainedDataPtr)
                {
                    Output::send<LogLevel::Verbose>(STR("Found ConditionsAttained TMap, setting all to true...\n"));
                    auto conditionsAttainedMap = static_cast<TMap<ERewardCondition, bool>*>(conditionsAttainedDataPtr);
                    if (conditionsAttainedMap)
                    {
                        int conditionsUnlocked = 0;
                        Output::send<LogLevel::Verbose>(STR("ConditionsAttained map size: {}\n"), conditionsAttainedMap->Num());
                        
                        for (auto& pair : *conditionsAttainedMap) 
                        { 
                            Output::send<LogLevel::Verbose>(STR("Setting ConditionsAttained {} (was: {}) to: true\n"), 
                                static_cast<int>(pair.Key), pair.Value);
                            pair.Value = true;
                            conditionsUnlocked++;
                        }
                        Output::send<LogLevel::Verbose>(STR("✅ Set {} conditions to true in ConditionsAttained\n"), conditionsUnlocked);
                        totalUnlocked += conditionsUnlocked;
                    }
                }
                
                if (totalUnlocked > 0)
                {
                    Output::send<LogLevel::Verbose>(STR("✅ Successfully updated RewardsGained, RewardsDisplayed, and ConditionsAttained maps\n"));
                    ShowNotification("unlock_rewards", "All rewards unlocked!", ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
                    unlockSuccess = true;
                    break; // Exit the save game loop since we succeeded
                }
            }
            
            if (!unlockSuccess)
            {
                Output::send<LogLevel::Warning>(STR("❌ RewardsGained, RewardsDisplayed, or ConditionsAttained not found or empty\n"));
            }
        }
        catch (const std::exception& e)
        {
            Output::send<LogLevel::Error>(STR("❌ Error unlocking rewards: {}\n"), to_wstring(e.what()));
        }
    }
    
    void UnlockSecretTheater()
    {
        Output::send<LogLevel::Verbose>(STR("🎬 Unlocking Secret Theater (Videos + Dialogs)...\n"));
        try
        {
            UE4SSUtils::ObjectSearchConfig config;
            config.ClassName = "UserProfileSaveGame";
            config.RequireValid = true;
            config.MaxResults = 5;
            auto foundSaveGames = UE4SSUtils::FindObjects(config);
            if (foundSaveGames.empty()) 
            { 
                Output::send<LogLevel::Warning>(STR("❌ No UserProfileSaveGame objects found\n"));
                return;
            }
            
            bool unlockSuccess = false;
            for (auto* saveGame : foundSaveGames)
            {
                auto saveGameName = saveGame->GetFullName();
                Output::send<LogLevel::Verbose>(STR("Processing save game: {}\n"), saveGameName);
                
                int totalUnlocked = 0;
                
                // Update UnlockSecretTheaterMap TMap<EGsrSecretTheaterType, EGsrExtraAcquiredStatus>
                auto unlockMapResult = UE4SSUtils::GetProperty(saveGame, "UnlockSecretTheaterMap");
                if (unlockMapResult.Found && unlockMapResult.DataPtr)
                {
                    auto unlockMap = static_cast<TMap<EGsrSecretTheaterType, EGsrExtraAcquiredStatus>*>(unlockMapResult.DataPtr);
                    if (unlockMap)
                    {
                        Output::send<LogLevel::Verbose>(STR("Found UnlockSecretTheaterMap TMap, unlocking...\n"));
                        Output::send<LogLevel::Verbose>(STR("UnlockSecretTheaterMap size: {}\n"), unlockMap->Num());
                        
                        for (auto& pair : *unlockMap) 
                        { 
                            Output::send<LogLevel::Verbose>(STR("Setting SecretTheater {} (was: {}) to: Acquired\n"), 
                                static_cast<int>(pair.Key), static_cast<int>(pair.Value));
                            pair.Value = EGsrExtraAcquiredStatus::Acquired;
                            totalUnlocked++;
                        }
                        Output::send<LogLevel::Verbose>(STR("✅ Updated {} entries in UnlockSecretTheaterMap\n"), totalUnlocked);
                    }
                }
                
                // Update SecretTheaterDialogMap TMap<EGsrSecretTheaterType, bool>
                auto dialogMapResult = UE4SSUtils::GetProperty(saveGame, "SecretTheaterDialogMap");
                if (dialogMapResult.Found && dialogMapResult.DataPtr)
                {
                    auto dialogMap = static_cast<TMap<EGsrSecretTheaterType, bool>*>(dialogMapResult.DataPtr);
                    if (dialogMap)
                    {
                        int dialogUnlocked = 0;
                        Output::send<LogLevel::Verbose>(STR("Found SecretTheaterDialogMap TMap, enabling all dialogs...\n"));
                        Output::send<LogLevel::Verbose>(STR("SecretTheaterDialogMap size: {}\n"), dialogMap->Num());
                        
                        for (auto& pair : *dialogMap) 
                        { 
                            Output::send<LogLevel::Verbose>(STR("Setting SecretTheaterDialog {} (was: {}) to: true\n"), 
                                static_cast<int>(pair.Key), pair.Value);
                            pair.Value = true;
                            dialogUnlocked++;
                        }
                        Output::send<LogLevel::Verbose>(STR("✅ Updated {} entries in SecretTheaterDialogMap\n"), dialogUnlocked);
                        totalUnlocked += dialogUnlocked;
                    }
                }
                
                if (totalUnlocked > 0)
                {
                    Output::send<LogLevel::Verbose>(STR("✅ Successfully updated UnlockSecretTheaterMap and SecretTheaterDialogMap\n"));
                    ShowNotification("unlock_theater", "Secret Theater unlocked!", ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
                    unlockSuccess = true;
                    break; // Exit the save game loop since we succeeded
                }
            }
            
            if (!unlockSuccess)
            {
                Output::send<LogLevel::Warning>(STR("❌ UnlockSecretTheaterMap or SecretTheaterDialogMap not found or empty\n"));
            }
        }
        catch (const std::exception& e)
        {
            Output::send<LogLevel::Error>(STR("❌ Error unlocking Secret Theater: {}\n"), to_wstring(e.what()));
        }
    }
    
    void UnlockAllAchievements()
    {
        Output::send<LogLevel::Verbose>(STR("🏆 Unlocking All Achievements...\n"));
        try
        {
            UE4SSUtils::ObjectSearchConfig config;
            config.ClassName = "UserProfileSaveGame";
            config.RequireValid = true;
            config.MaxResults = 5;
            auto foundSaveGames = UE4SSUtils::FindObjects(config);
            if (foundSaveGames.empty()) 
            { 
                Output::send<LogLevel::Warning>(STR("❌ No UserProfileSaveGame objects found\n"));
                return;
            }
            
            bool unlockSuccess = false;
            for (auto* saveGame : foundSaveGames)
            {
                auto saveGameName = saveGame->GetFullName();
                Output::send<LogLevel::Verbose>(STR("Processing save game: {}\n"), saveGameName);
                
                int totalUnlocked = 0;
                
                // Update AchievementData TArray<FAchievementData>
                auto achievementDataResult = UE4SSUtils::GetProperty(saveGame, "AchievementData");
                if (achievementDataResult.Found)
                {
                    auto achievementArray = UE4SSUtils::GetTArray<FAchievementData>(saveGame, "AchievementData");
                    if (achievementArray && achievementArray->ArrayNum > 0)
                    {
                        Output::send<LogLevel::Verbose>(STR("Found AchievementData TArray, setting all to 1.0 (100%)...\n"));
                        Output::send<LogLevel::Verbose>(STR("AchievementData array size: {}\n"), achievementArray->ArrayNum);
                        
                        for (int i = 0; i < achievementArray->ArrayNum; i++)
                        {
                            auto& achievement = achievementArray->Data[i];
                            Output::send<LogLevel::Verbose>(STR("Setting Achievement {} (Progress: {:.6f}, Percent: {:.6f}) to: 1.0\n"), 
                                i, achievement.Progress, achievement.Percent);
                            
                            // Set both Progress and Percent to 1.0 (normalized 100% completion)
                            achievement.Progress = 1.0f;
                            achievement.Percent = 1.0f;
                            totalUnlocked++;
                        }
                        Output::send<LogLevel::Verbose>(STR("✅ Updated {} achievements to 1.0 (100% completion)\n"), totalUnlocked);
                    }
                }
                
                if (totalUnlocked > 0)
                {
                    Output::send<LogLevel::Verbose>(STR("✅ Successfully updated AchievementData array\n"));
                    ShowNotification("unlock_achievements", "All achievements unlocked!", ImVec4(1.0f, 0.6f, 0.0f, 1.0f)); // Orange for warning
                    unlockSuccess = true;
                    break; // Exit the save game loop since we succeeded
                }
            }
            
            if (!unlockSuccess)
            {
                Output::send<LogLevel::Warning>(STR("❌ AchievementData not found or empty\n"));
            }
        }
        catch (const std::exception& e)
        {
            Output::send<LogLevel::Error>(STR("❌ Error unlocking achievements: {}\n"), to_wstring(e.what()));
        }
    }
    
    void UnlockAllKerotan()
    {
        Output::send<LogLevel::Verbose>(STR("🐸 Unlocking All Kerotan...\n"));
        try
        {
            UE4SSUtils::ObjectSearchConfig config;
            config.ClassName = "UserProfileSaveGame";
            config.RequireValid = true;
            config.MaxResults = 5;
            auto foundSaveGames = UE4SSUtils::FindObjects(config);
            if (foundSaveGames.empty()) 
            { 
                Output::send<LogLevel::Warning>(STR("❌ No UserProfileSaveGame objects found\n"));
                return; 
            }
            
            bool unlockSuccess = false;
            for (auto* saveGame : foundSaveGames)
            {
                auto saveGameName = saveGame->GetFullName();
                Output::send<LogLevel::Verbose>(STR("Processing save game: {}\n"), saveGameName);
                
                int totalUnlocked = 0;
                
                // Update KerotanList TArray<bool>
                auto kerotanListResult = UE4SSUtils::GetProperty(saveGame, "KerotanList");
                if (kerotanListResult.Found)
                {
                    auto kerotanArray = UE4SSUtils::GetTArray<bool>(saveGame, "KerotanList");
                    if (kerotanArray && kerotanArray->ArrayNum > 0)
                    {
                        Output::send<LogLevel::Verbose>(STR("Found KerotanList TArray, setting all to true...\n"));
                        Output::send<LogLevel::Verbose>(STR("KerotanList array size: {}\n"), kerotanArray->ArrayNum);
                        
                        for (int i = 0; i < kerotanArray->ArrayNum; i++)
                        {
                            Output::send<LogLevel::Verbose>(STR("Setting Kerotan {} (was: {}) to: true\n"), 
                                i, kerotanArray->Data[i]);
                            kerotanArray->Data[i] = true;
                            totalUnlocked++;
                        }
                        Output::send<LogLevel::Verbose>(STR("✅ Updated {} entries in KerotanList\n"), totalUnlocked);
                    }
                }
                
                if (totalUnlocked > 0)
                {
                    Output::send<LogLevel::Verbose>(STR("✅ Successfully updated KerotanList array\n"));
                    ShowNotification("unlock_kerotan", "All Kerotan unlocked!", ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
                    unlockSuccess = true;
                    break; // Exit the save game loop since we succeeded
                }
            }
            
            if (!unlockSuccess)
            {
                Output::send<LogLevel::Warning>(STR("❌ KerotanList not found or empty\n"));
            }
        }
        catch (const std::exception& e)
        {
            Output::send<LogLevel::Error>(STR("❌ Error unlocking Kerotan: {}\n"), to_wstring(e.what()));
        }
    }
    
    void UnlockAllGako()
    {
        Output::send<LogLevel::Verbose>(STR("🦎 Unlocking All Gako...\n"));
        try
        {
            UE4SSUtils::ObjectSearchConfig config;
            config.ClassName = "UserProfileSaveGame";
            config.RequireValid = true;
            config.MaxResults = 5;
            auto foundSaveGames = UE4SSUtils::FindObjects(config);
            if (foundSaveGames.empty()) 
            { 
                Output::send<LogLevel::Warning>(STR("❌ No UserProfileSaveGame objects found\n"));
                return; 
            }
            
            bool unlockSuccess = false;
            for (auto* saveGame : foundSaveGames)
            {
                auto saveGameName = saveGame->GetFullName();
                Output::send<LogLevel::Verbose>(STR("Processing save game: {}\n"), saveGameName);
                
                int totalUnlocked = 0;
                
                // Update GakoList TArray<bool>
                auto gakoListResult = UE4SSUtils::GetProperty(saveGame, "GakoList");
                if (gakoListResult.Found)
                {
                    auto gakoArray = UE4SSUtils::GetTArray<bool>(saveGame, "GakoList");
                    if (gakoArray && gakoArray->ArrayNum > 0)
                    {
                        Output::send<LogLevel::Verbose>(STR("Found GakoList TArray, setting all to true...\n"));
                        Output::send<LogLevel::Verbose>(STR("GakoList array size: {}\n"), gakoArray->ArrayNum);
                        
                        for (int i = 0; i < gakoArray->ArrayNum; i++)
                        {
                            Output::send<LogLevel::Verbose>(STR("Setting Gako {} (was: {}) to: true\n"), 
                                i, gakoArray->Data[i]);
                            gakoArray->Data[i] = true;
                            totalUnlocked++;
                        }
                        Output::send<LogLevel::Verbose>(STR("✅ Updated {} entries in GakoList\n"), totalUnlocked);
                    }
                }
                
                if (totalUnlocked > 0)
                {
                    Output::send<LogLevel::Verbose>(STR("✅ Successfully updated GakoList array\n"));
                    ShowNotification("unlock_gako", "All Gako unlocked!", ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
                    unlockSuccess = true;
                    break; // Exit the save game loop since we succeeded
                }
            }
            
            if (!unlockSuccess)
            {
                Output::send<LogLevel::Warning>(STR("❌ GakoList not found or empty\n"));
            }
        }
        catch (const std::exception& e)
        {
            Output::send<LogLevel::Error>(STR("❌ Error unlocking Gako: {}\n"), to_wstring(e.what()));
        }
    }
    
    void UnlockAllRecordTitles()
    {
        Output::send<LogLevel::Verbose>(STR("🏆 Unlocking All Record Titles...\n"));
        try
        {
            UE4SSUtils::ObjectSearchConfig config;
            config.ClassName = "UserProfileSaveGame";
            config.RequireValid = true;
            config.MaxResults = 5;
            auto foundSaveGames = UE4SSUtils::FindObjects(config);
            if (foundSaveGames.empty()) 
            { 
                Output::send<LogLevel::Warning>(STR("❌ No UserProfileSaveGame objects found\n"));
                return; 
            }
            
            bool unlockSuccess = false;
            for (auto* saveGame : foundSaveGames)
            {
                auto saveGameName = saveGame->GetFullName();
                Output::send<LogLevel::Verbose>(STR("Processing save game: {}\n"), saveGameName);
                
                int totalUnlocked = 0;
                
                // Update RecordTitleData TArray<ECobraRecordTitleStatus>
                auto recordTitleResult = UE4SSUtils::GetProperty(saveGame, "RecordTitleData");
                if (recordTitleResult.Found)
                {
                    auto recordTitleArray = UE4SSUtils::GetTArray<ECobraRecordTitleStatus>(saveGame, "RecordTitleData");
                    if (recordTitleArray && recordTitleArray->ArrayNum > 0)
                    {
                        Output::send<LogLevel::Verbose>(STR("Found RecordTitleData TArray, setting all to ACQUIRED...\n"));
                        Output::send<LogLevel::Verbose>(STR("RecordTitleData array size: {}\n"), recordTitleArray->ArrayNum);
                        
                        for (int i = 0; i < recordTitleArray->ArrayNum; i++)
                        {
                            auto currentStatus = static_cast<int>(recordTitleArray->Data[i]);
                            Output::send<LogLevel::Verbose>(STR("Setting Record Title {} (was: {}) to: ACQUIRED\n"), 
                                i, currentStatus);
                            recordTitleArray->Data[i] = ECobraRecordTitleStatus::ACQUIRED;
                            totalUnlocked++;
                        }
                        Output::send<LogLevel::Verbose>(STR("✅ Updated {} entries in RecordTitleData\n"), totalUnlocked);
                    }
                }
                
                if (totalUnlocked > 0)
                {
                    Output::send<LogLevel::Verbose>(STR("✅ Successfully updated RecordTitleData array\n"));
                    ShowNotification("unlock_titles", "All record titles unlocked!", ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
                    unlockSuccess = true;
                    break; // Exit the save game loop since we succeeded
                }
            }
            
            if (!unlockSuccess)
            {
                Output::send<LogLevel::Warning>(STR("❌ RecordTitleData not found or empty\n"));
            }
        }
        catch (const std::exception& e)
        {
            Output::send<LogLevel::Error>(STR("❌ Error unlocking Record Titles: {}\n"), to_wstring(e.what()));
        }
    }
    
    void UnlockCharacterViewer()
    {
        Output::send<LogLevel::Verbose>(STR("👥 Unlocking Character Viewer...\n"));
        try
        {
            UE4SSUtils::ObjectSearchConfig config;
            config.ClassName = "UserProfileSaveGame";
            config.RequireValid = true;
            config.MaxResults = 5;
            auto foundSaveGames = UE4SSUtils::FindObjects(config);
            if (foundSaveGames.empty()) 
            { 
                Output::send<LogLevel::Warning>(STR("❌ No UserProfileSaveGame objects found\n"));
                return; 
            }
            
            bool unlockSuccess = false;
            for (auto* saveGame : foundSaveGames)
            {
                auto saveGameName = saveGame->GetFullName();
                Output::send<LogLevel::Verbose>(STR("Processing save game: {}\n"), saveGameName);
                
                // Update UnlockCharacterViewerStatus EGsrExtraAcquiredStatus
                auto charViewerResult = UE4SSUtils::GetProperty(saveGame, "UnlockCharacterViewerStatus");
                if (charViewerResult.Found)
                {
                    auto currentStatus = *static_cast<EGsrExtraAcquiredStatus*>(charViewerResult.DataPtr);
                    auto currentStatusInt = static_cast<int>(currentStatus);
                    
                    Output::send<LogLevel::Verbose>(STR("Found UnlockCharacterViewerStatus (was: {}), setting to ACQUIRED...\n"), currentStatusInt);
                    
                    // Set to Acquired
                    *static_cast<EGsrExtraAcquiredStatus*>(charViewerResult.DataPtr) = EGsrExtraAcquiredStatus::Acquired;
                    
                    Output::send<LogLevel::Verbose>(STR("✅ Successfully updated UnlockCharacterViewerStatus to Acquired\n"));
                    ShowNotification("unlock_character", "Character viewer unlocked!", ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
                    unlockSuccess = true;
                    break; // Exit the save game loop since we succeeded
                }
            }
            
            if (!unlockSuccess)
            {
                Output::send<LogLevel::Warning>(STR("❌ UnlockCharacterViewerStatus not found\n"));
            }
        }
        catch (const std::exception& e)
        {
            Output::send<LogLevel::Error>(STR("❌ Error unlocking Character Viewer: {}\n"), to_wstring(e.what()));
        }
    }
    
    void UnlockGuysavage()
    {
        Output::send<LogLevel::Verbose>(STR("🎮 Unlocking Guy Savage...\n"));
        try
        {
            UE4SSUtils::ObjectSearchConfig config;
            config.ClassName = "UserProfileSaveGame";
            config.RequireValid = true;
            config.MaxResults = 5;
            auto foundSaveGames = UE4SSUtils::FindObjects(config);
            if (foundSaveGames.empty()) 
            { 
                Output::send<LogLevel::Warning>(STR("❌ No UserProfileSaveGame objects found\n"));
                return; 
            }
            
            bool unlockSuccess = false;
            for (auto* saveGame : foundSaveGames)
            {
                auto saveGameName = saveGame->GetFullName();
                Output::send<LogLevel::Verbose>(STR("Processing save game: {}\n"), saveGameName);
                
                // Update bHasCleared bool
                auto hasClearedResult = UE4SSUtils::GetProperty(saveGame, "bHasCleared");
                if (hasClearedResult.Found)
                {
                    auto currentValue = *static_cast<bool*>(hasClearedResult.DataPtr);
                    
                    Output::send<LogLevel::Verbose>(STR("Found bHasCleared (was: {}), setting to true...\n"), currentValue);
                    
                    // Set to true
                    *static_cast<bool*>(hasClearedResult.DataPtr) = true;
                    
                    Output::send<LogLevel::Verbose>(STR("✅ Successfully updated bHasCleared to true\n"));
                    ShowNotification("unlock_guy", "Guy Savage unlocked!", ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
                    unlockSuccess = true;
                    break; // Exit the save game loop since we succeeded
                }
            }
            
            if (!unlockSuccess)
            {
                Output::send<LogLevel::Warning>(STR("❌ bHasCleared not found\n"));
            }
        }
        catch (const std::exception& e)
        {
            Output::send<LogLevel::Error>(STR("❌ Error unlocking Guy Savage: {}\n"), to_wstring(e.what()));
        }
    }
    
    void UnlockDemoTheater()
    {
        Output::send<LogLevel::Verbose>(STR("🎬 Unlocking Demo Theater (Open Flags + Initial Flags)...\n"));
        try
        {
            UE4SSUtils::ObjectSearchConfig config;
            config.ClassName = "UserProfileSaveGame";
            config.RequireValid = true;
            config.MaxResults = 5;
            auto foundSaveGames = UE4SSUtils::FindObjects(config);
            if (foundSaveGames.empty()) 
            { 
                Output::send<LogLevel::Warning>(STR("❌ No UserProfileSaveGame objects found\n"));
                return; 
            }
            
            bool unlockSuccess = false;
            for (auto* saveGame : foundSaveGames)
            {
                auto saveGameName = saveGame->GetFullName();
                Output::send<LogLevel::Verbose>(STR("Processing save game: {}\n"), saveGameName);
                
                int totalUnlocked = 0;
                
                // Update DemoTheaterOpenFlags TArray<bool>
                auto demoTheaterResult = UE4SSUtils::GetProperty(saveGame, "DemoTheaterOpenFlags");
                if (demoTheaterResult.Found)
                {
                    auto demoTheaterArray = UE4SSUtils::GetTArray<bool>(saveGame, "DemoTheaterOpenFlags");
                    if (demoTheaterArray && demoTheaterArray->ArrayNum > 0)
                    {
                        Output::send<LogLevel::Verbose>(STR("Found DemoTheaterOpenFlags TArray, setting all to true...\n"));
                        Output::send<LogLevel::Verbose>(STR("DemoTheaterOpenFlags array size: {}\n"), demoTheaterArray->ArrayNum);
                        
                        for (int i = 0; i < demoTheaterArray->ArrayNum; i++)
                        {
                            Output::send<LogLevel::Verbose>(STR("Setting Demo Theater Open Flag {} (was: {}) to: true\n"), 
                                i, demoTheaterArray->Data[i]);
                            demoTheaterArray->Data[i] = true;
                            totalUnlocked++;
                        }
                        Output::send<LogLevel::Verbose>(STR("✅ Updated {} entries in DemoTheaterOpenFlags\n"), demoTheaterArray->ArrayNum);
                    }
                }
                
                // Update DemoTheaterInitialFlagList TArray<bool>
                auto demoInitialResult = UE4SSUtils::GetProperty(saveGame, "DemoTheaterInitialFlagList");
                if (demoInitialResult.Found)
                {
                    auto demoInitialArray = UE4SSUtils::GetTArray<bool>(saveGame, "DemoTheaterInitialFlagList");
                    if (demoInitialArray && demoInitialArray->ArrayNum > 0)
                    {
                        Output::send<LogLevel::Verbose>(STR("Found DemoTheaterInitialFlagList TArray, setting all to true...\n"));
                        Output::send<LogLevel::Verbose>(STR("DemoTheaterInitialFlagList array size: {}\n"), demoInitialArray->ArrayNum);
                        
                        for (int i = 0; i < demoInitialArray->ArrayNum; i++)
                        {
                            Output::send<LogLevel::Verbose>(STR("Setting Demo Theater Initial Flag {} (was: {}) to: true\n"), 
                                i, demoInitialArray->Data[i]);
                            demoInitialArray->Data[i] = true;
                            totalUnlocked++;
                        }
                        Output::send<LogLevel::Verbose>(STR("✅ Updated {} entries in DemoTheaterInitialFlagList\n"), demoInitialArray->ArrayNum);
                    }
                }
                
                if (totalUnlocked > 0)
                {
                    Output::send<LogLevel::Verbose>(STR("✅ Successfully updated Demo Theater arrays\n"));
                    ShowNotification("unlock_demo", "Demo Theater unlocked!", ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
                    unlockSuccess = true;
                    break; // Exit the save game loop since we succeeded
                }
            }
            
            if (!unlockSuccess)
            {
                Output::send<LogLevel::Warning>(STR("❌ Demo Theater arrays not found or empty\n"));
            }
        }
        catch (const std::exception& e)
        {
            Output::send<LogLevel::Error>(STR("❌ Error unlocking Demo Theater: {}\n"), to_wstring(e.what()));
        }
    }
    
    void UnlockEuropeanExtreme()
    {
        Output::send<LogLevel::Verbose>(STR("💀 Unlocking European Extreme...\n"));
        try
        {
            UE4SSUtils::ObjectSearchConfig config;
            config.ClassName = "UserProfileSaveGame";
            config.RequireValid = true;
            config.MaxResults = 5;
            auto foundSaveGames = UE4SSUtils::FindObjects(config);
            if (foundSaveGames.empty()) 
            { 
                Output::send<LogLevel::Warning>(STR("❌ No UserProfileSaveGame objects found\n"));
                return; 
            }
            
            bool unlockSuccess = false;
            for (auto* saveGame : foundSaveGames)
            {
                auto saveGameName = saveGame->GetFullName();
                Output::send<LogLevel::Verbose>(STR("Processing save game: {}\n"), saveGameName);
                
                // Update bHasClearedExtremeDifficulty bool
                auto extremeResult = UE4SSUtils::GetProperty(saveGame, "bHasClearedExtremeDifficulty");
                if (extremeResult.Found)
                {
                    auto currentValue = *static_cast<bool*>(extremeResult.DataPtr);
                    
                    Output::send<LogLevel::Verbose>(STR("Found bHasClearedExtremeDifficulty (was: {}), setting to true...\n"), currentValue);
                    
                    // Set to true
                    *static_cast<bool*>(extremeResult.DataPtr) = true;
                    
                    Output::send<LogLevel::Verbose>(STR("✅ Successfully updated bHasClearedExtremeDifficulty to true\n"));
                    ShowNotification("unlock_extreme", "European Extreme unlocked!", ImVec4(1.0f, 0.4f, 0.4f, 1.0f)); // Red for extreme difficulty
                    unlockSuccess = true;
                    break; // Exit the save game loop since we succeeded
                }
            }
            
            if (!unlockSuccess)
            {
                Output::send<LogLevel::Warning>(STR("❌ bHasClearedExtremeDifficulty not found\n"));
            }
        }
        catch (const std::exception& e)
        {
            Output::send<LogLevel::Error>(STR("❌ Error unlocking European Extreme: {}\n"), to_wstring(e.what()));
        }
    }

    // Destructor - cleanup utils
    ~MGSModMod() override
    {
        UE4SSUtils::Cleanup();
        Output::send<LogLevel::Verbose>(STR("MGSMod: Cleanup complete\n"));
    }

};

#define TEST_MOD_API __declspec(dllexport)
extern "C"
{
    TEST_MOD_API RC::CppUserModBase* start_mod()
    {
        return new MGSModMod();
    }

    TEST_MOD_API void uninstall_mod(RC::CppUserModBase* mod)
    {
        delete mod;
    }
}