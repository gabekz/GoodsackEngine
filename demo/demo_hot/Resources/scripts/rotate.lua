--require("Vector")

local system = {}

local Keycode = require("keycodes")

--helloC = function() hello() end

function system.start(entity)
    print("From rotate.lua!")
    --entity.ComponentTest.rotation_speed = 12.5
    --entity.ComponentTest.rotation_speed = 12
end

function system.update(entity)
    --Transform:RotateAxis(e.ComponentTransform, 0, 90, 0);
    --print("From rotate! id is: "..entity.id.."")
    print("from update in rotate")


    --Transform.translate(entity.ComponentTransform, vec3:new(0, 0, 0));

    --print("speed: "..entity.ComponentTest.rotation_speed.."")
    --print("movement : "..entity.ComponentTest.movement_increment.."")

    --mystr = delta_time()
    --print("string: "..mystr.."")

end

system.run = function() _ECS_RegisterSystem(system) end
return system