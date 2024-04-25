-- This Source Code Form is subject to the terms of the Mozilla Public
-- License, v. 2.0. If a copy of the MPL was not distributed with this
-- file, You can obtain one at http://mozilla.org/MPL/2.0/.

local irpriv = {}

-- "Take me with you! I'm the one man who knows everything!"
function irpriv.kernel(opts)
    local epoch = ir.time()
    -- 60Hz is a good guess, but isn't accurate for every system (including my
    -- own). Allegro doesn't have the ability to retrieve the refresh rate in a
    -- cross-platform manner so this will have to do for now. ~ahill
    local framerate = 1 / (tonumber(opts.framerate) or 60)
    local frames = 0
    local game = opts.game or "game.zip"
    local running = true

    ir.info("ir.kernel: Framerate set to " .. tostring(1 / framerate) .. "Hz")

    local function command(cmd)
        local cmds = {
            [ir.cmd.HALT] = function()
                running = false
            end
        }
        if cmds[cmd] and type(cmds[cmd]) == "function" then
            cmds[cmd]()
        elseif cmd ~= ir.cmd.NONE then
            ir.error("ir.kernel: Invalid command received: " .. tostring(cmd))
        end
    end

    local function time()
        -- We have an epoch in the kernel that is subtracted from the engine's
        -- time so we don't get stuck rendering frames at the very beginning.
        -- ~ahill
        return ir.time() - epoch
    end

    -- Remove unsafe functions
    rawset(_G, "dofile", nil)
    rawset(_G, "load", nil)
    rawset(_G, "loadfile", nil)

    -- Proxy I/O functions
    rawset(_G, "print", ir.info)

    ir.info("ir.kernel: Kernel started")

    if not ir.internal.mount(game) then
        ir.error("ir.kernel: Failed to load game archive \"" .. game .. "\"")
        running = false
    end

    if running then
        local main = ir.internal.fetch("main.lua")
        if main then
            local status, err = pcall(loadstring(main))
            if status then
                command(ir.init(opts))
            else
                ir.error("ir.kernel: Failed to execute \"main.lua\": " .. tostring(err))
                running = false
            end
        else
            ir.error("ir.kernel: Failed to load \"main.lua\"")
            running = false
        end
    end

    while running do
        event = ir.internal.poll()
        if event and ir.subscriptions[event.type] then
            for _, listener in ipairs(ir.subscriptions[event.type]) do
                local msg = listener(event)
                if msg then
                    command(ir.update(msg))
                    if not running then
                        break
                    end
                end
            end
        end
        if time() / framerate > frames then
            ir.internal.clear()
            -- ir.view()
            ir.internal.present()
            frames = frames + 1
        end
    end

    ir.info("ir.kernel: Stopping kernel")

    ir.internal.umount(game)
end

setmetatable(ir, {
    __index = irpriv,
    __newindex = function(t, k, v)
        if not irpriv[k] then
            rawset(t, k, v)
        end
    end
})

ir.res = {}

-- ...