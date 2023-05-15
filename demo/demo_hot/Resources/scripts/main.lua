-- creating an eventlist for future reference
package.path = '../demo/demo_hot/Resources/scripts/?.lua;' .. package.path

-- setup systems to run
--require("ecs-system").run()

require("ecs-system2").run()
require("camera-input").run()
--require("rotate").run()

function main()
    print("Hello world")
    -- so far, nothing speci)al in here...
end
