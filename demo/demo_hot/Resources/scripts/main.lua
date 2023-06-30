-- creating an eventlist for future reference
package.path = '../demo/demo_hot/Resources/scripts/?.lua;' .. package.path
--package.path = '../build/bin/?;' .. package.path

-- Running Systems
require("weapon-system").run()
require("camera-input").run()

Goodsack = require('GoodsackAPI')

function main()
    print("Lua entry")
    print("package.path: "..package.path.."")
end
