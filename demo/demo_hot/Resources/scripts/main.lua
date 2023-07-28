-- creating an eventlist for future reference
package.path = '../demo/demo_hot/Resources/scripts/?.lua;' .. package.path
--package.path = '../build/bin/?;' .. package.path

-- Running Systems
require('systems.weapon_controller').run()
require('systems.weapon_sway').run()
--require('systems.camera_fov').run()

local GoodsackAPI = require('GoodsackAPI')

function main()
    print("Lua entry")
    print("package.path: "..package.path.."")
    print(GoodsackAPI.version_info())
end