local system4 = {}

local Keycode = require("keycodes")

function system4.start(e)
    print("START FRO CAMERA")
end

function system4.update(entity) 

    --camera = entity.Camera;
    --transform = entity.Transform;

    if Input:GetKeyDown(Keycode.Q) == true then
        entity.Camera.fov = entity.Camera.fov - 10 * delta_time();
    end
    if Input:GetKeyDown(Keycode.E) == true then
        entity.Camera.fov = entity.Camera.fov + 10 * delta_time();
        print(entity.Test.rotation_speed)
    end
end

system4.run = function() _ECS_RegisterSystem(system4) end
return system4