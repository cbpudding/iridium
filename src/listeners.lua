-- This Source Code Form is subject to the terms of the Mozilla Public
-- License, v. 2.0. If a copy of the MPL was not distributed with this
-- file, You can obtain one at http://mozilla.org/MPL/2.0/.

function ir.register(handler)
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

ir.listener = {}

function ir.listener.keydown(pattern, msg)
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