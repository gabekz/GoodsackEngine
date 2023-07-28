--- weapon_controller ECS system
-- @module system

local system = {}

local keycodes = require("keycodes")

local pos_aiming = {
    -0.31855,
    -0.187,
    -0.2
}

local pos_standby = {
    -0.2,
    -0.22,
    -0.4340
}

local fov_aiming = 30
local fov_standby = 45


local testValue = 0

-------------------------------------
-- Handles input for the weapon controller.
-- @param entity Entity Pointer
-- @param[in, out] rotation Entity Rotation
-------------------------------------
local function handle_input(entity, rotation)

    local entity_camera = entity.Weapon.entity_camera

    -- Aiming --
    if(input.GetKeyDown(keycodes.MOUSE1)) then
        entity.Transform.position = pos_aiming
        entity_camera.Camera.fov = fov_aiming
    else
        entity.Transform.position = pos_standby
        entity_camera.Camera.fov = fov_standby
    end

    -- Movement --
    if(input.GetKeyDown(keycodes.W)) then
        if testValue >= 0 and testValue < 1 then
            testValue = testValue + 1 * 10 * time.DeltaTime()
        end

        rotation[1] = math.Lerp(0, -10, testValue)
    end

    if(input.GetKeyDown(keycodes.S)) then
        rotation[1] = 10
        testValue = 0
    end

    if(input.GetKeyDown(keycodes.A)) then
        rotation[3] = -175
    end
    if(input.GetKeyDown(keycodes.D)) then
        rotation[3] = -190
    end

end

function system.update(entity)

    -- require Weapon and Transform components
    if entity.Weapon == nil or entity.Transform == nil then
        return nil
    end

    --print("From entity: "..entity.id.."... camera fov is "..camera_ent.Camera.fov.."");

    local new_rotation = {0, 0, -180}

    if(input.GetCursorState().is_locked == true ) then
        handle_input(entity, new_rotation)
    else
        entity.Transform.position = pos_standby
    end

    -- update rotation
    entity.Transform.orientation = new_rotation 
end

system.run = function() _ECS_RegisterSystem(system) end
return system
