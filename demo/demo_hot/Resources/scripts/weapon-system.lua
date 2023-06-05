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

    -- require Weapon and Transform components
    if not (entity.Weapon) then
        return nil
    end
    if not (entity.Transform) then
        return nil
    end

    last_position = entity.Transform.position
    -- last_position = entity.Weapon.pos_starting

    if(Input:GetKeyDown(KeyCode.W)) then
        print("id: "..entity.id.."")
        print("x: "..entity.Transform.position.x.."")
        print("y: "..last_position.y.."")
        print("z: "..entity.Transform.position.z.."")
    end
end

system.run = function() _ECS_RegisterSystem(system) end
return system
