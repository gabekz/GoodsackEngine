local system = {}

Time = require('GoodsackAPI.Time')
Keycode = require("keycodes")

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

    if(entity.Transform == nil) then 
        return nil
    end

    if (Input:GetKeyDown(Keycode.Q)) then
        entity.Camera.fov = entity.Camera.fov - 10 * Time.get_delta_time();
    end
    if (Input:GetKeyDown(Keycode.E)) then
        entity.Camera.fov = entity.Camera.fov + 10 * Time.get_delta_time();
    end
    if (Input:GetKeyDown(Keycode.Z)) then
        print(entity.Camera.speed)
    end
end

system.run = function() _ECS_RegisterSystem(system) end
return system
