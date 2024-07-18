-- An example designed to be as simple as possible while covering the basics.

-- This function is called with any command line arguments that may have been passed to the engine
function ir.init(opts)
    return ir.cmd.texturemap(ir.fs["test.png"])
end

-- Once the input level for the quit bind makes it past the threshold, send the "stop" message
-- to the update function.
ir.subscriptions = ir.listener.bind("quit", ir.trigger.over(0.5), "stop")

-- Any messages that have been received from a listener are sent here
function ir.update(msg)
    -- When the "stop" message has been received, tell the engine to stop everything and quit.
    if msg == "stop" then
        return ir.cmd.halt()
    end
    return ir.cmd.none()
end

-- Display a simple triangle to verify that everything works
function ir.view()
    return {
        camera = ir.camera.perspective(ir.viewport.ratio(), 90, 0.125, 64),
        (function(stage)
            return {
                0.0, 0.5, 0.5, 0, 0.5, 0.0,
                0.5, -0.5, 0.5, 0, 1.0, 1.0,
                -0.5, -0.5, 0.5, 0, 0.0, 1.0
            }
        end)
    }
end
