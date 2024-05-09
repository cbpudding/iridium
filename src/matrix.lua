-- This Source Code Form is subject to the terms of the Mozilla Public
-- License, v. 2.0. If a copy of the MPL was not distributed with this
-- file, You can obtain one at http://mozilla.org/MPL/2.0/.

ir.matrix = {}

-- Utility functions

local function ismatrix(mat)
    return type(mat) == "table" and mat.__type == "matrix"
end

-- Metatable definitions

local col_meta = {}
local row_meta = {}

function col_meta.__index(t, k)
    if k == "columns" then
        return #t[1]
    elseif k == "rows" then
        return #t
    elseif k == "__type" then
        return "matrix"
    else
        return rawget(t, k)
    end
end

function col_meta.__newindex(t, k, v)
    if type(k) == "number" and k >= 1 and k <= t.rows then
        rawset(t, k, v)
    end
end

function row_meta.__index(t, k)
    if k == "columns" then
        return #t
    elseif k == "__type" then
        return "matrix.row"
    else
        return rawget(t, k)
    end
end

function row_meta.__newindex(t, k, v)
    if type(k) == "number" and k >= 1 and k <= t.columns then
        rawset(t, k, v)
    end
end

-- Method definitions

function ir.matrix.identity(size)
    local matrix = ir.matrix.new(size, size)

    for i = 1, size do
        matrix[i][i] = 1
    end

    return matrix
end

function ir.matrix.new(cols, rows)
    local matrix = {}

    for c = 1, cols do
        local row = {}

        for r = 1, rows do
            table.insert(row, 0)
        end

        setmetatable(row, row_meta)
        table.insert(matrix, row)
    end

    setmetatable(matrix, col_meta)

    return matrix
end