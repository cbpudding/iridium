-- This Source Code Form is subject to the terms of the Mozilla Public
-- License, v. 2.0. If a copy of the MPL was not distributed with this
-- file, You can obtain one at http://mozilla.org/MPL/2.0/.

ir.internal.binds = {}

local irpriv = {}

irpriv.bind = {}

irpriv.cmd = {
    none = function()
        return {0}
    end,
    halt = function()
        return {1}
    end,
    texturemap = function(map)
        return {2, map}
    end,
    multiple = function(list)
        return {3, list}
    end
}

local function add_event_listener(type, index, handler)
    if not ir.internal.binds[type] then
        ir.internal.binds[type] = {}
    end
    if index then
        if not ir.internal.binds[type][index] then
            ir.internal.binds[type][index] = {}
        end
        table.insert(ir.internal.binds[type][index], handler)
    else
        -- If index is nil, then the event type doesn't really have a "selector". ~ahill
        table.insert(ir.internal.binds[type], handler)
    end
end

-- Format: key <keycode> <hold|toggle>
local function parse_key_bind(name, tokens)
    if #tokens == 3 then
        local keycode = tonumber(tokens[2])
        if keycode then
            irpriv.bind[name] = 0
            if tokens[3] == "toggle" then
                add_event_listener(ir.internal.EVENT_KEY_DOWN, keycode, function(event)
                    if irpriv.bind[name] == 0 then
                        irpriv.bind[name] = 1
                    else
                        irpriv.bind[name] = 0
                    end
                end)
            else
                if tokens[3] ~= "hold" then
                    ir.warn("parse_key_bind: Invalid behavior \"" .. tokens[3] .. "\". Defaulting to \"hold\".")
                end
                add_event_listener(ir.internal.EVENT_KEY_DOWN, keycode, function(event)
                    irpriv.bind[name] = 1
                end)
                add_event_listener(ir.internal.EVENT_KEY_UP, keycode, function(event)
                    irpriv.bind[name] = 0
                end)
            end
        else
            ir.warn("parse_key_bind: Invalid keycode \"" .. tokens[2] .. "\" on bind \"" .. name .. "\"")
        end
    else
        ir.warn("parse_key_bind: Invalid format for bind \"" .. name .. "\"")
    end
end

-- Format: mouse axis <w|x|y|z> <locked|normal> [inverse]
local function parse_mouse_axis_bind(name, tokens)
    if #tokens >= 4 then
        local axis = tokens[3]
        local locked = tokens[4] == "locked"
        local inverse = tokens[5] == "inverse"
        if tokens[3] == "w" or tokens[3] == "x" or tokens[3] == "y" or tokens[3] == "z" then
            if not locked and tokens[4] ~= "normal" then
                ir.warn("parse_mouse_axis_bind: Invalid behavior \"" .. tokens[3] .. "\". Defaulting to \"normal\".")
            end
            irpriv.bind[name] = 0
            -- TODO: How should cursor locking work? ~ahill
            add_event_listener(ir.internal.EVENT_MOUSE_AXES, nil, function(event)
                -- TODO: Figure out how to properly normalize and scale mouse
                --       movement ~ahill
            end)
        else
            ir.warn("parse_mouse_axis_bind: Unknown axis \"" .. axis .. "\" for bind \"" .. name .. "\"")
        end
    else
        ir.warn("parse_mouse_axis_bind: Invalid format for bind \"" .. name "\"")
    end
end

-- Format: mouse button <code> <hold|toggle>
local function parse_mouse_button_bind(name, tokens)
    if #tokens == 4 then
        local button = tonumber(tokens[3])
        if button then
            irpriv.bind[name] = 0
            if tokens[4] == "toggle" then
                add_event_listener(ir.internal.EVENT_MOUSE_BUTTON_DOWN, button, function(event)
                    if irpriv.bind[name] == 0 then
                        irpriv.bind[name] = 1
                    else
                        irpriv.bind[name] = 0
                    end
                end)
            else
                if tokens[4] ~= "hold" then
                    ir.warn("parse_mouse_button_bind: Invalid behavior \"" .. tokens[3] .. "\". Defaulting to \"hold\".")
                end
                add_event_listener(ir.internal.EVENT_MOUSE_BUTTON_DOWN, button, function(event)
                    irpriv.bind[name] = 1
                end)
                add_event_listener(ir.internal.EVENT_MOUSE_BUTTON_UP, button, function(event)
                    irpriv.bind[name] = 0
                end)
            end
        else
            ir.warn("parse_mouse_button_bind: Invalid code \"" .. tokens[2] .. "\" on bind \"" .. name .. "\"")
        end
    else
        ir.warn("parse_mouse_button_bind: Invalid format for bind \"" .. name .. "\"")
    end
end

-- Format: mouse pressure
-- I legitimately don't know if you can have multiple sources. ~ahill
local function parse_mouse_pressure_bind(name)
    irpriv.bind[name] = 0
    add_event_listener(ir.internal.EVENT_MOUSE_AXES, nil, function(event)
        -- Thanks Allegro for already keeping this input in a range from 0.0 to
        -- 1.0! ~ahill
        irpriv.bind[name] = event.pressure
    end)
end

-- Format: mouse <axis|button|pressure> ...
local function parse_mouse_bind(name, tokens)
    if #tokens >= 2 then
        if tokens[2] == "axis" then
            parse_mouse_axis_bind(name, tokens)
        elseif tokens[2] == "button" then
            parse_mouse_button_bind(name, tokens)
        elseif tokens[2] == "pressure" then
            parse_mouse_pressure_bind(name)
        else
            ir.warn("parse_mouse_bind: Invalid subtype \"" .. tokens[2] .. "\" on bind \"" .. name .. "\"")
        end
    else
        ir.warn("parse_mouse_bind: Invalid format for bind \"" .. name .. "\"")
    end
end

local function parse_bind(name, tokens)
    if #tokens > 1 then
        if tokens[1] == "key" then
            parse_key_bind(name, tokens)
        elseif tokens[1] == "mouse" then
            parse_mouse_bind(name, tokens)
        else
            ir.warn("parse_bind: Invalid type \"" .. tokens[1] .. "\" on bind \"" .. name .. "\"")
        end
    end
end

-- "Take me with you! I'm the one man who knows everything!"
function irpriv.kernel(opts)
    -- 60Hz is a good guess, but isn't accurate for every system (including my
    -- own). Allegro doesn't have the ability to retrieve the refresh rate in a
    -- cross-platform manner so this will have to do for now. ~ahill
    local framerate = 1 / (tonumber(opts.framerate) or 60)
    local frames = 0
    local running = true

    -- This function needs to stay here because it's within the closure that
    -- controls the "running" value. ~ahill
    local function command(cmd)
        local cmds = {
            -- ir.cmd.halt
            [1] = function()
                running = false
            end,
            -- ir.cmd.texturemap
            [2] = function()
                ir.internal.texturemap(cmd[2])
            end,
            -- ir.cmd.multiple
            [3] = function()
                for _, v in ipairs(cmd[2]) do
                    command(v)
                end
            end
        }
        if cmds[cmd[1]] and type(cmds[cmd[1]]) == "function" then
            cmds[cmd[1]]()
        elseif cmd[1] ~= 0 then
            ir.error("ir.kernel: Invalid command received: " .. tostring(cmd))
        end
    end

    ir.info("ir.kernel: Framerate set to " .. tostring(1 / framerate) .. "Hz")

    if opts.fullscreen then
        ir.internal.fullscreen(true)
    else
        ir.internal.fullscreen(false)
    end

    if opts.height then
        local height = tonumber(opts.height)
        if height then
            ir.internal.height(height)
        end
    end

    if opts.width then
        local width = tonumber(opts.width)
        if width then
            ir.internal.width(width)
        end
    end

    -- Remove unsafe functions
    rawset(_G, "dofile", nil)
    rawset(_G, "load", nil)
    rawset(_G, "loadfile", nil)

    -- Proxy I/O functions
    rawset(_G, "print", ir.info)

    -- Interpret user binds
    if ir.user.binds then
        for name, line in pairs(ir.user.binds) do
            local tokens = {}

            for token in line:gmatch("%w+") do
                table.insert(tokens, token)
            end

            parse_bind(name, tokens)
        end
    else
        ir.warn("ir.kernel: User binds not defined!")
    end

    ir.info("ir.kernel: Kernel started")

    if running then
        local main = ir.internal.fetch("main.lua")
        if main then
            local status, err = pcall(assert(loadstring(main), "Failed to parse main.lua"))
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

    -- Start the internal framerate timer ~ahill
    ir.internal.frametimer(framerate)

    while running do
        local event = ir.internal.poll()
        -- This will work for now, but we will need to check the source when we
        -- start to deal with multiple timers later on. ~ahill
        if event.type == ir.internal.EVENT_TIMER then
            ir.internal.clear()

            local stage = ir.view()

            -- If no explicit camera has been defined, use an identity matrix. ~ahill
            stage.camera = stage.camera or ir.mat.identity()
            ir.internal.setcamera(stage.camera)

            local vertices = {}

            -- In this case, "tess" is short for tessellation. I just didn't want to write "tessellation" a lot. ~ahill
            for i, tess in ipairs(stage) do
                if type(tess) == "function" then
                    -- Lua passes by reference so this should be fine... right? ~ahill
                    local verts = tess(stage)
                    -- (3 axes + 1 texture ID + 2 texcoords) * 3 vertices/triangle = 18 values/triangle ~ahill
                    if type(verts) == "table" and #verts % 18 == 0 then
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
        elseif ir.internal.binds[event.type] then
            local index = nil
            if event.type == ir.internal.EVENT_KEY_DOWN or event.type == ir.internal.EVENT_KEY_UP then
                index = event.keycode
            elseif event.type == ir.internal.EVENT_MOUSE_BUTTON_DOWN or event.type == ir.internal.EVENT_MOUSE_BUTTON_UP then
                index = event.button
            else
                ir.warn("ir.kernel: Unhandled event type " .. event.type)
            end
            if index then
                if ir.internal.binds[event.type][index] then
                    for _, update in ipairs(ir.internal.binds[event.type][index]) do
                        update(event)
                    end
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

-- Trigger Functions

ir.trigger = {}

function ir.trigger.under(threshold)
    return function(v)
        return v < threshold
    end
end

function ir.trigger.over(threshold)
    return function(v)
        return v > threshold
    end
end

-- Viewport Functions

ir.viewport = {}

function ir.viewport.height()
    return ir.internal.height()
end

function ir.viewport.ratio()
    return ir.viewport.width() / ir.viewport.height()
end

function ir.viewport.width()
    return ir.internal.width()
end

-- Camera Functions

ir.camera = {}

function ir.camera.perspective(aspect, fov, near, far)
    -- Of all the things I miss, I miss my mind the most. ~ahill
    local inv_dist = math.tan(fov / 2)
    local range = near - far
    return ir.mat.from{
        1.0 / (aspect * inv_dist), 0.0,            0.0,                   0.0,
        0.0,                       1.0 / inv_dist, 0.0,                   0.0,
        0.0,                       0.0,            (-near - far) / range, (2 * far * near) / range,
        0.0,                       0.0,            1.0,                   0.0
    }
end

-- Program Defaults

---@diagnostic disable-next-line: duplicate-set-field
ir.subscriptions = {}

---@diagnostic disable-next-line: duplicate-set-field
function ir.init(opts)
    return ir.cmd.NONE
end

---@diagnostic disable-next-line: duplicate-set-field
function ir.view()
    return {}
end

---@diagnostic disable-next-line: duplicate-set-field
function ir.update(msg)
    return ir.cmd.NONE
end