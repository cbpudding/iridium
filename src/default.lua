-- This Source Code Form is subject to the terms of the Mozilla Public
-- License, v. 2.0. If a copy of the MPL was not distributed with this
-- file, You can obtain one at http://mozilla.org/MPL/2.0/.

-- { init : opts -> Cmd
-- , view : () -> Stage
-- , update : msg -> Cmd
-- , subscriptions : () -> List (Sub msg)
-- }

local irpriv = {}

-- "Take me with you! I'm the one man who knows everything!"
function irpriv.kernel()
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

    ir.info("ir.kernel: Kernel started")

    command(ir.init({}))

    while running do
        event = ir.poll()
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

function irpriv.register(type, handler)
    if not ir.subscriptions[type] then
        ir.subscriptions[type] = {}
    end
    table.insert(ir.subscriptions[type], handler)
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
    ir.register(ir.internal.EVENT_KEY_DOWN, function(event)
        return 1
    end)

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