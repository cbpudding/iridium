-- This Source Code Form is subject to the terms of the Mozilla Public
-- License, v. 2.0. If a copy of the MPL was not distributed with this
-- file, You can obtain one at http://mozilla.org/MPL/2.0/.

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