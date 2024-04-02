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
    ir.init({})
    -- ...
end

function ir.init(opts)
    return ir.cmd.NONE
end

function ir.view()
    return {}
end

function ir.update(msg)
    return ir.cmd.NONE
end

function ir.subscriptions()
    return {}
end

setmetatable(ir, {
    __index = irpriv,
    __newindex = function(t, k, v)
        if not irpriv[k] then
            rawset(t, k, v)
        end
    end
})