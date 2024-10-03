--- @meta _

error("Cannot require a meta file")

ir.internal = {}

--- Log {string} with level DEBUG
--- @param string string
function ir.debug(string) end

--- Log {string} with level ERROR
--- @param string string
function ir.error(string) end

--- Log {string} with level INFO
--- @param string string
function ir.info(string) end

--- TODO:
function ir.shader() end

--- @return number
function ir.time() end

--- Log {string} with level WARN
--- @param string string
function ir.warn(string) end

--- TODO:
function ir.internal.clear() end

--- @param path string
--- @return table
function ir.internal.fetch(path) end

--- @param flag boolean
function ir.internal.fullscreen(flag) end

--- If called with no arguments returns current screen height,
--- otherwise sets it to {height}
--- @param height integer?
--- @return integer?
function ir.internal.height(height) end

--- If called with no arguments returns current screen width,
--- otherwise sets it to {width}
--- @param width integer?
--- @return integer?
function ir.internal.width(width) end

--- @param path string
--- @return string[]
function ir.internal.list(path) end

-- TODO: Wait for the api to become stable before continuing
