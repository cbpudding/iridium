-- This Source Code Form is subject to the terms of the Mozilla Public
-- License, v. 2.0. If a copy of the MPL was not distributed with this
-- file, You can obtain one at http://mozilla.org/MPL/2.0/.

-- Utility Functions

function ir.register(listeners)
    local subs = {}
    for i, listener in ipairs(listeners) do
        if type(listener) ~= "table" or #listener ~= 2 or type(listener[2]) ~= "function" then
            ir.error("ir.register: Invalid subscription registration at " .. i)
        else
            if not subs[listener[1]] then
                subs[listener[1]] = {}
            end
            table.insert(subs[listener[1]], listener[2])
        end
    end
    return subs
end

-- Event Listeners

ir.listener = {}

function ir.listener.generic(kind)
    return function(pattern, msg)
        return {
            kind,
            function(event)
                for k, v in pairs(pattern) do
                    if type(v) == "function" then
                        if not v(event[k]) then
                            return ir.msg.NOTHING
                        end
                    elseif event[k] ~= v then
                        return ir.msg.NOTHING
                    end
                end
                return msg
            end
        }
    end
end

ir.listener.keychar = ir.listener.generic(ir.internal.EVENT_KEY_CHAR)
ir.listener.keydown = ir.listener.generic(ir.internal.EVENT_KEY_DOWN)
ir.listener.keyup = ir.listener.generic(ir.internal.EVENT_KEY_UP)
ir.listener.mouseaxes = ir.listener.generic(ir.internal.EVENT_MOUSE_AXES)
ir.listener.mousedown = ir.listener.generic(ir.internal.EVENT_MOUSE_DOWN)
ir.listener.mouseenter = ir.listener.generic(ir.internal.EVENT_MOUSE_ENTER_DISPLAY)
ir.listener.mouseleave = ir.listener.generic(ir.internal.EVENT_MOUSE_LEAVE_DISPLAY)
ir.listener.mouseup = ir.listener.generic(ir.internal.EVENT_MOUSE_UP)