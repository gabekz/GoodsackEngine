local system = {}

system.required_components = {
    "Camera",
    "Transform"
}

local KeyCode = require("keycodes")

function system.start(entity)
    print("Hello world from Weapon System!")
end

function system.update(entity)
    if(Input:GetKeyDown(KeyCode.W)) then
        print("Pressing W")
    end
end

system.run = function() _ECS_RegisterSystem(system) end
return system