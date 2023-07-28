-- creating an eventlist for future reference
package.path = '../demo/demo_hot/Resources/scripts/?.lua;' .. package.path
--package.path = '../build/bin/?;' .. package.path

-- Running Systems
SYSTEM_RUN('systems.weapon_controller')
SYSTEM_RUN('systems.weapon_sway')

print("package.path: "..package.path.."")