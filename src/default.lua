-- This Source Code Form is subject to the terms of the Mozilla Public
-- License, v. 2.0. If a copy of the MPL was not distributed with this
-- file, You can obtain one at http://mozilla.org/MPL/2.0/.

ir.subscriptions = {}

function ir.init(opts)
    return ir.cmd.NONE
end

function ir.view()
    return {}
end

function ir.update(msg)
    return ir.cmd.NONE
end