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
    local methods = {
        flatten = ir.matrix.flatten
    }

    if k == "columns" then
        return #t[1]
    elseif k == "rows" then
        return #t
    elseif k == "__type" then
        return "matrix"
    elseif methods[k] then
        return methods[k]
    else
        return rawget(t, k)
    end
end

function col_meta.__mul(left, right)
    -- Keep the matrix on the left side for simplicity ~ahill
    if left.__type ~= "matrix" then
        left, right = right, left
    end

    -- Scalar multiplication
    if type(right) == "number" then
        local result = ir.matrix.new(left.columns, left.rows)

        for c = 1, left.columns do
            for r = 1, left.rows do
                result[c][r] = left[c][r] * right
            end
        end

        return result

    -- Matrix multiplication
    elseif right.__type == "matrix" then
        -- Multiplying only works given the following condition! ~ahill
        if left.columns ~= right.rows then
            return nil
        end

        local result = ir.matrix.new(right.columns, left.rows)

        for c = 1, result.columns do
            for r = 1, result.rows do
                -- We don't need to set the initial value because ir.matrix.new
                -- gives a matrix that has already been zeroed out! ~ahill
                for n = 1, left.columns do
                    result[c][r] = result[c][r] + (left[c][n] * right[n][r])
                end
            end
        end

        return result

    -- All other operations undefined! ~ahill
    else
        return nil
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

function ir.matrix.flatten(matrix)
    local result = {}

    for c = 1, matrix.columns do
        for r = 1, matrix.rows do
            table.insert(result, matrix[c][r])
        end
    end

    return result
end

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