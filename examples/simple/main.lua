-- An example designed to be as simple as possible while covering the basics.

-- This function is called with any command line arguments that may have been passed to the engine
function ir.init(opts)
    return ir.cmd.NONE
end

-- Listeners are registered here and will call the update function with messages if triggered
ir.subscriptions = ir.register{
    -- When the Q key is pressed, send the message "stop"
    ir.listener.keydown({keycode = 17}, "stop")
}

-- Any messages that have been received from a listener are sent here
function ir.update(msg)
    -- When the "stop" message has been received, tell the engine to stop everything and quit.
    if msg == "stop" then
        return ir.cmd.HALT
    end
    return ir.cmd.NONE
end