require("Vector")

local system = {}

function system.update(e)
    --Transform:RotateAxis(e.ComponentTransform, 0, 90, 0);
    print("From rotate!")
end

system.run = function() _ECS_RegisterSystem(system) end
return system