-- creating an eventlist for future reference
package.path = (fs.URI([[data://scripts/]]) .. '?.lua;' .. package.path)

-- Running Systems
SYSTEM_RUN('systems.weapon_controller')
SYSTEM_RUN('systems.weapon_sway')

print("package.path: "..package.path.."")

local myvec = Vector(7, 8, 9)
local myvec2 = Vector(3, 1, 8)

myvec = myvec - myvec2
print(myvec)

print(myvec:Cross(Vector(8, 9, 20)))
