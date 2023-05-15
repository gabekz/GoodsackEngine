local system = {}

local Keycode = require("keycodes")

function system.start(e)
    print("Start - From Camera")
end

function system.update(entity) 

    --camera = entity.Camera;
    --transform = entity.Transform;

    if (Input:GetKeyDown(Keycode.Q)) then
        entity.Camera.fov = entity.Camera.fov - 10 * delta_time();
    end
    if (Input:GetKeyDown(Keycode.E)) then
        entity.Camera.fov = entity.Camera.fov + 10 * delta_time();
    end
end

system.run = function() _ECS_RegisterSystem(system) end
return system