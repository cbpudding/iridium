-- This Source Code Form is subject to the terms of the Mozilla Public
-- License, v. 2.0. If a copy of the MPL was not distributed with this
-- file, You can obtain one at http://mozilla.org/MPL/2.0/.

ir.internal.binds = {}

local irpriv = {}

irpriv.bind = {}

irpriv.cmd = {
    NONE = 0,
    HALT = 1
}

-- "Take me with you! I'm the one man who knows everything!"
function irpriv.kernel(opts)
    local epoch = ir.time()
    -- 60Hz is a good guess, but isn't accurate for every system (including my
    -- own). Allegro doesn't have the ability to retrieve the refresh rate in a
    -- cross-platform manner so this will have to do for now. ~ahill
    local framerate = 1 / (tonumber(opts.framerate) or 60)
    local frames = 0
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

    -- Interpret user binds
    for name, line in pairs(ir.pref.binds) do
        local tokens = {}

        for token in line:gmatch("%w+") do
            table.insert(tokens, token)
        end

        if #tokens > 1 then
            if tokens[1] == "key" then
                if #tokens == 3 then
                    local keycode = tonumber(tokens[2])
                    if keycode then
                        irpriv.bind[name] = 0
                        if not ir.internal.binds[ir.internal.EVENT_KEY_DOWN] then
                            ir.internal.binds[ir.internal.EVENT_KEY_DOWN] = {}
                        end
                        if not ir.internal.binds[ir.internal.EVENT_KEY_DOWN][keycode] then
                            ir.internal.binds[ir.internal.EVENT_KEY_DOWN][keycode] = {}
                        end
                        if tokens[3] == "toggle" then
                            table.insert(ir.internal.binds[ir.internal.EVENT_KEY_DOWN][keycode], function(event)
                                if irpriv.bind[name] == 0 then
                                    irpriv.bind[name] = 1
                                else
                                    irpriv.bind[name] = 0
                                end
                            end)
                        else
                            if tokens[3] ~= "hold" then
                                ir.warn("ir.kernel: Invalid behavior \"" .. tokens[3] .. "\". Defaulting to \"hold\".")
                            end
                            if not ir.internal.binds[ir.internal.EVENT_KEY_UP] then
                                ir.internal.binds[ir.internal.EVENT_KEY_UP] = {}
                            end
                            if not ir.internal.binds[ir.internal.EVENT_KEY_UP][keycode] then
                                ir.internal.binds[ir.internal.EVENT_KEY_UP][keycode] = {}
                            end
                            table.insert(ir.internal.binds[ir.internal.EVENT_KEY_DOWN][keycode], function(event)
                                irpriv.bind[name] = 1
                            end)
                            table.insert(ir.internal.binds[ir.internal.EVENT_KEY_UP][keycode], function(event)
                                irpriv.bind[name] = 0
                            end)
                        end
                    else
                        ir.warn("ir.kernel: Invalid keycode \"" .. tokens[2] .. "\" on bind \"" .. name .. "\"")
                    end
                else
                    ir.warn("ir.kernel: Invalid format for bind \"" .. name .. "\"")
                end
            else
                ir.warn("ir.kernel: Invalid type \"" .. tokens[1] .. "\" on bind \"" .. name .. "\"")
            end
        end
    end

    ir.info("ir.kernel: Kernel started")

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
        local event = ir.internal.poll()
        if event and ir.internal.binds[event.type] then
            if event.type == ir.internal.EVENT_KEY_DOWN or event.type == ir.internal.EVENT_KEY_DOWN then
                if ir.internal.binds[event.type][event.keycode] then
                    for _, update in ipairs(ir.internal.binds[event.type][event.keycode]) do
                        update(event)
                    end
                end
            else
                ir.warn("ir.kernel: Unhandled event type " .. event.type)
            end
            for _, listener in ipairs(ir.subscriptions) do
                local msg = listener()
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

            local stage = ir.view()

            -- If no explicit camera has been defined, use an identity matrix. ~ahill
            stage.camera = stage.camera or {
                1.0, 0.0, 0.0, 0.0,
                0.0, 1.0, 0.0, 0.0,
                0.0, 0.0, 1.0, 0.0,
                0.0, 0.0, 0.0, 1.0
            }
            ir.internal.setcamera(stage.camera)

            -- ...

            local vertices = {}

            -- In this case, "tess" is short for tessellation. I just didn't want to write "tessellation" a lot. ~ahill
            for i, tess in ipairs(stage) do
                if type(tess) == "function" then
                    -- Lua passes by reference so this should be fine... right? ~ahill
                    local verts = tess(stage)
                    -- 3 axes * 3 vertices/triangle = 9 values/triangle ~ahill
                    if type(verts) == "table" and #verts % 9 == 0 then
                        for _, n in ipairs(verts) do
                            table.insert(vertices, n)
                        end
                    else
                        ir.warn("ir.kernel: Value returned from index " .. i .. " is not renderable")
                    end
                else
                    ir.warn("ir.kernel: Index " .. i .. " is not renderable")
                end
            end

            ir.internal.render(vertices)

            ir.internal.present()
            frames = frames + 1
        end
    end

    ir.info("ir.kernel: Stopping kernel")
end

setmetatable(ir, {
    __index = irpriv,
    -- "I hate this hacker crap!" ~ahill
    __metatable = "You didn't say the magic word!",
    __newindex = function(t, k, v)
        if not irpriv[k] then
            rawset(t, k, v)
        end
    end
})

-- PhysicsFS integration

local function fs_meta(path)
    return {
        __index = function(t, k)
            local itempath
            if path == "" then
                itempath = k
            else
                itempath = path .. "/" .. k
            end
            local item = ir.internal.fetch(itempath)
            if type(item) == "table" then
                local newpath
                if path == "" then
                    newpath = k
                else
                    newpath = newpath .. "/" .. k
                end
                setmetatable(item, fs_meta(newpath))
            end
            return item
        end,
        -- Don't forget to compile LuaJIT with LUAJIT_ENABLE_LUA52COMPAT or
        -- this won't work! ~ahill
        __pairs = function(t)
            local i = 1
            local list = ir.internal.list(path)
            return function(t, k)
                if k == nil then
                    return list[1], nil
                else
                    i = i + 1
                    return list[i], nil
                end
            end
        end
    }
end

ir.fs = {}

setmetatable(ir.fs, fs_meta(""))

-- Matrix Math

local function matrix_row_meta(parent, column, rows)
    return {
        __index = function(t, k)
            if k >= 1 and k <= rows then
                return ir.internal.matrix_index(parent, column, k)
            end
        end,
        __newindex = function(t, k, v)
            if k >= 1 and k <= rows then
                ir.internal.matrix_newindex(parent, column, k, v)
            end
        end
    }
end

local matrix_meta = {
    __index = function(t, k)
        if k >= 1 and k <= rawget(t, "columns") then
            local column = {}
            setmetatable(column, matrix_row_meta(t, k, rawget(t, "rows")))
            return column
        elseif rawget(t, k) then
            return rawget(t, k)
        end
    end,
    -- "Gandalf!" ~ahill
    __metatable = "You shall not pass!",
    __mul = ir.internal.matrix_mul,
    __newindex = function(t, k, v)
        -- No! ~ahill
    end
}

function ir.matrix(columns, rows)
    local victim = ir.internal.matrix_new(columns, rows)
    setmetatable(victim, matrix_meta)
    return victim
end

-- Listeners

ir.listener = {}

function ir.listener.bind(bind, cond, msg)
    local active = false
    return {
        function()
            if cond(ir.bind[bind]) and not active then
                active = true
                return msg
            elseif active then
                active = false
            end
            return nil
        end
    }
end

function ir.listener.merge(listeners)
    local merged = {}

    for _, list in ipairs(listeners) do
        for _, listener in ipairs(list) do
            table.insert(merged, listener)
        end
    end

    return merged
end

-- Program Defaults

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