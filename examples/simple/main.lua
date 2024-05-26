-- An example designed to be as simple as possible while covering the basics.

-- This function is called with any command line arguments that may have been passed to the engine
function ir.init(opts)
    return ir.cmd.NONE
end

-- Once the input level for the quit bind makes it past the threshold, send the "stop" message
-- to the update function.
ir.subscriptions = ir.listener.bind("quit", (function(v) return v > 0.5 end), "stop")

-- Any messages that have been received from a listener are sent here
function ir.update(msg)
    -- When the "stop" message has been received, tell the engine to stop everything and quit.
    if msg == "stop" then
        return ir.cmd.HALT
    end
    return ir.cmd.NONE
end