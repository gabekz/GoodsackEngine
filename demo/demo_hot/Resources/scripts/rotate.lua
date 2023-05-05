--require("Vector")

local system = {}

function system.start(entity)
    print("From rotate.lua!")
    --entity.ComponentTest.rotation_speed = 12.5
    --entity.ComponentTest.rotation_speed = 12
end

function system.update(entity)
    --Transform:RotateAxis(e.ComponentTransform, 0, 90, 0);
    print("From rotate! id is: "..entity.id.."")

    print("speed: "..entity.ComponentTest.rotation_speed.."")
    print("movement : "..entity.ComponentTest.movement_increment.."")

end

system.run = function() _ECS_RegisterSystem(system) end
return system