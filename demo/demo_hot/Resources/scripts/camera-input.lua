local system = {}

local Keycode = require("keycodes")

function system.start(e)
    print("START FRO CAMERA")
end

function system.update(entity) 

    --camera = entity.Camera;
    --transform = entity.Transform;

    --speed = camera.speed * delta_time();

    if Input:GetKeyDown(Keycode.Q) == true then
        entity.Camera.fov = entity.Camera.fov - 10 * delta_time();
    end
    if Input:GetKeyDown(Keycode.E) == true then
        entity.Camera.fov = entity.Camera.fov + 10 * delta_time();
    end
end

system.run = function() _ECS_RegisterSystem(system) end
return system