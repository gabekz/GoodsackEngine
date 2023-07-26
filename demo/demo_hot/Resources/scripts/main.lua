-- creating an eventlist for future reference
package.path = '../demo/demo_hot/Resources/scripts/?.lua;' .. package.path
--package.path = '../build/bin/?;' .. package.path

-- Running Systems
require("weapon-system").run()
require("wepsway-system").run()
require("camera-input").run()

local GoodsackAPI = require('GoodsackAPI')

function main()
    print("Lua entry")
    print("package.path: "..package.path.."")
    print(GoodsackAPI.version_info())
end