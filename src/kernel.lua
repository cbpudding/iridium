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
    local framerate = (function(framerate)
        if type(framerate) == "string" then
            local desired = tonumber(framerate)
            if desired then
                ir.info("ir.kernel: Framerate manually set to " .. desired .. "Hz")
                return 1 / desired
            else
                ir.warn("ir.kernel: Failed to parse desired framerate \"" .. tostring(framerate) .. "\"")
            end
        end
        return 1/60
    end)(opts.framerate)
    local frames = 0
    local frametime = framerate / 2
    local running = true

    local function command(cmd)
        local cmds = {
            [ir.cmd.HALT] = function()
                running = false
            end
        }
        if cmds[cmd] and type(cmds[cmd]) == "function" then
            cmds[cmd]()
        elseif cmd ~= ir.cmd.NONE then
            ir.error("Invalid command received: " .. cmd)
        end
    end

    local function time()
        return ir.time() - epoch
    end

    -- Remove unsafe functions
    rawset(_G, "dofile", nil)
    rawset(_G, "load", nil)
    rawset(_G, "loadfile", nil)

    -- Proxy I/O functions
    rawset(_G, "print", ir.info)

    ir.info("ir.kernel: Kernel started")

    command(ir.init(opts))

    while running do
        event = ir.internal.poll()
        if event and ir.subscriptions[event.type] then
            for _, handler in ipairs(ir.subscriptions[event.type]) do
                local msg = handler(event)
                if msg ~= ir.msg.NOTHING then
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
end

setmetatable(ir, {
    __index = irpriv,
    __newindex = function(t, k, v)
        if not irpriv[k] then
            rawset(t, k, v)
        end
    end
})