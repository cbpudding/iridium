-- This Source Code Form is subject to the terms of the Mozilla Public
-- License, v. 2.0. If a copy of the MPL was not distributed with this
-- file, You can obtain one at http://mozilla.org/MPL/2.0/.

local irpriv = {}

-- "Take me with you! I'm the one man who knows everything!"
function irpriv.kernel(opts)
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
    end

    ir.info("ir.kernel: Stopping kernel")
end

function irpriv.register(handler)
    if type(handler) ~= "table" or #handler ~= 2 or type(handler[1]) ~= "number" or type(handler[2]) ~= "function" then
        ir.error("ir.register: Invalid subscription registration")
    else
        if not ir.subscriptions[handler[1]] then
            ir.subscriptions[handler[1]] = {}
        end
        table.insert(ir.subscriptions[handler[1]], handler[2])
        return true
    end
    return false
end

irpriv.listener = {}

function irpriv.listener.keydown(pattern, msg)
    return {
        ir.internal.EVENT_KEY_DOWN,
        function(event)
            for k, v in pairs(pattern) do
                if event[k] ~= v then
                    return ir.msg.NOTHING
                end
            end
            return msg
        end
    }
end

setmetatable(ir, {
    __index = irpriv,
    __newindex = function(t, k, v)
        if not irpriv[k] then
            rawset(t, k, v)
        end
    end
})

-- Application defaults

ir.subscriptions = {}

function ir.init(opts)
    ir.register(ir.listener.keydown({keycode = 59}, 1))

    return ir.cmd.NONE
end

function ir.view()
    return {}
end

function ir.update(msg)
    if msg == 1 then
        return ir.cmd.HALT
    end
    return ir.cmd.NONE
end