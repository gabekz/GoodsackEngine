--require("Vector")

local system = {}

function system.start(entity)
    entity.ComponentTest.rotation_speed = 12.5
end

function system.update(entity)
    --Transform:RotateAxis(e.ComponentTransform, 0, 90, 0);
    print("From rotate!")

    print("speed: "..entity.ComponentTest.rotation_speed.."")

end

system.run = function() _ECS_RegisterSystem(system) end
return system