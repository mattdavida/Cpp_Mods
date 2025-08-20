print('--------------- SB Rewards Mod ------------------')


local UEHelpers = require("UEHelpers.UEHelpers")


-- @param func function to execute
-- @param intervalMs interval in milliseconds
local function setGameThreadInterval(func, intervalMs)
    local function repeatFunc()
        ExecuteInGameThread(function()
            func()
        end)
        ExecuteWithDelay(intervalMs, repeatFunc)
    end
    ExecuteWithDelay(intervalMs, repeatFunc)
end

-- infinite jumping. really fun, but there is a max fly time
-- programmed somewhere so you'll die if you fly for too long
function InfiniteJump()
    ExecuteInGameThread(function()
        local player = UEHelpers.GetPlayer()
        print("Player: " .. player:GetFullName())
        if player then
            print('JUMP MAX 1000')
            player.JumpMaxCount = 1000
        end
    end)
end

function GiveSkillPoints()
    ExecuteInGameThread(function()
        local cm = StaticFindObject("/Script/SB.Default__SBCheatManager")
        if cm then
            cm:SBPlayerSP(9999)
        end
    end)
end

-- essentially god mod as best as i could get it
-- every 2 seconds reset health and beta skill guage and burst skill guage and max tachy guage
-- works exceptionally well
function GodMod()
    local function updateStats()
        local cm = StaticFindObject("/Script/SB.Default__SBCheatManager")
        if cm then
            cm:SBPlayerHP(9999)
            cm:SBPlayerBetaGauge(9999)
            -- need to have burst skills unlocked for any effect
            cm:SBPlayerBurstGauge(9999)
            -- need to have tachy skills unlocked for any effect 
            cm:SBPlayerMaxTachyGauge()
        end
    end

    -- initial call to update stats on enable
    updateStats()

    -- refresh stats every 2 seconds
    setGameThreadInterval(function()
        updateStats()
    end, 1000)
end