-- This Source Code Form is subject to the terms of the Mozilla Public
-- License, v. 2.0. If a copy of the MPL was not distributed with this
-- file, You can obtain one at http://mozilla.org/MPL/2.0/.

local ir.internal.bindstate = {}

local irpriv = {}

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

    local function parse_bind(line)
        local bind_mapping = {}

        local tokens = {}

        for token in line:gmatch("%w+") do
            table.insert(tokens, token)
        end

        if not bind_mapping[tokens[1]] then
            ir.warn("ir.kernel.parse_bind: Invalid bind type \"" .. tokens[1] .. "\"")
            return nil
        end

        return bind_mapping[tokens[1]](tokens)
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

    -- Interpret binds
    for bind, line in pairs(ir.pref.binds) do
        irpriv.bind[bind] = parse_bind(line)
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

ir.listener = {}

function ir.listener.generic(kind)
    return function(pattern, msg)
        return {
            kind,
            function(event)
                for k, v in pairs(pattern) do
                    if type(v) == "function" then
                        if not v(event[k]) then
                            return nil
                        end
                    elseif event[k] ~= v then
                        return nil
                    end
                end
                return msg
            end
        }
    end
end

ir.listener.focus = ir.listener.generic(ir.internal.EVENT_DISPLAY_SWITCH_IN)
ir.listener.keychar = ir.listener.generic(ir.internal.EVENT_KEY_CHAR)
ir.listener.keydown = ir.listener.generic(ir.internal.EVENT_KEY_DOWN)
ir.listener.keyup = ir.listener.generic(ir.internal.EVENT_KEY_UP)
ir.listener.mouseaxes = ir.listener.generic(ir.internal.EVENT_MOUSE_AXES)
ir.listener.mousedown = ir.listener.generic(ir.internal.EVENT_MOUSE_DOWN)
ir.listener.mouseenter = ir.listener.generic(ir.internal.EVENT_MOUSE_ENTER_DISPLAY)
ir.listener.mouseleave = ir.listener.generic(ir.internal.EVENT_MOUSE_LEAVE_DISPLAY)
ir.listener.mouseup = ir.listener.generic(ir.internal.EVENT_MOUSE_UP)
ir.listener.quit = ir.listener.generic(ir.internal.EVENT_DISPLAY_CLOSE)
ir.listener.resize = ir.listener.generic(ir.internal.EVENT_DISPLAY_RESIZE)
ir.listener.unfocus = ir.listener.generic(ir.internal.EVENT_DISPLAY_SWITCH_OUT)

function ir.listener.bind(bind)
    local active = false
    return function(cond, msg)
        if type(cond) == "function" then
            if ir.internal.bindstate[bind] then
                if cond(ir.internal.bindstate[bind]) then
                    if not active then
                        active = true
                        return msg
                    end
                else
                    active = false
                end
            end
        end
        return nil
    end
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