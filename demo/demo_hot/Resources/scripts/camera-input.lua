local system = {}

local Keycode = require("keycodes")

function system.start(e)

    if(e.Camera == nil) then 
        return nil
    end

    print("Start - From Camera")
end

function system.update(entity) 

    if(entity.Camera == nil) then 
        return nil
    end

    if (Input:GetKeyDown(Keycode.Q)) then
        entity.Camera.fov = entity.Camera.fov - 10 * delta_time();
    end
    if (Input:GetKeyDown(Keycode.E)) then
        entity.Camera.fov = entity.Camera.fov + 10 * delta_time();
    end
end

system.run = function() _ECS_RegisterSystem(system) end
return system