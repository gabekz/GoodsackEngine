-- creating an eventlist for future reference
--package.path = 'E:/Projects/GoodsackEngine/demo/demo_hot/Resources/scripts/?.lua;' .. package.path
package.path = '/home/gmk/gksrc/projects/GoodsackEngine/demo/demo_hot/Resources/scripts/?.lua;' .. package.path
--package.path = '../build/bin/?;' .. package.path

-- Running Systems
SYSTEM_RUN('systems.weapon_controller')
SYSTEM_RUN('systems.weapon_sway')

print("package.path: "..package.path.."")

local myvec = Vector(7, 8, 9)
local myvec2 = Vector(3, 1, 8)

myvec = myvec - myvec2
print(myvec)

print(myvec:Cross(Vector(8, 9, 20)))
