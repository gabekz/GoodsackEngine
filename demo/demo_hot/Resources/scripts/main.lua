-- creating an eventlist for future reference
package.path = '../demo/demo_hot/Resources/scripts/?.lua;' .. package.path
--package.path = '../build/bin/?;' .. package.path

local pScripts = [[e:\Projects\GoodsackEngine\demo\demo_hot\Resources\scripts\include\]]
include = function(path)
    dofile(pScripts .. path)
end

SYSTEM_RUN = function(name)
    require(name).run()
end

include('math.lua')

goodsack = require('goodsack')
input = require('goodsack.input')
time = require('goodsack.time')

-- Running Systems
SYSTEM_RUN('systems.weapon_controller')
SYSTEM_RUN('systems.weapon_sway')
--require('systems.camera_fov').run()

print("Lua entry")
print("package.path: "..package.path.."")
print(goodsack.version_info())