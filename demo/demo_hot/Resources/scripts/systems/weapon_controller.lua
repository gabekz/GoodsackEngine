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
local fov_standby = 55
local fov_lerp_speed = 10

-------------------------------------
-- Handles input for the weapon controller.
-- @param entity Entity Pointer
-- @param[in, out] rotation Entity Rotation
-------------------------------------
local function handle_input(entity, rotation)

    local entity_camera = entity.Weapon.entity_camera
    local isAiming = input.GetKeyDown(keycodes.MOUSE1)

    local aimRotAmount = (isAiming) and (0.05) or (1)

    --
    -- Aiming
    --
    if(isAiming) then
        entity.Transform.position = pos_aiming
        entity_camera.Camera.fov = math.Lerp(entity_camera.Camera.fov, fov_aiming, time.DeltaTime() * fov_lerp_speed);
        entity_camera.Camera.speed = 0.5
    else
        entity.Transform.position = pos_standby
        entity_camera.Camera.fov = math.Lerp(entity_camera.Camera.fov, fov_standby, time.DeltaTime() * fov_lerp_speed);
        entity_camera.Camera.speed = 2.5
    end

    --
    -- Movement
    --
    if(input.GetKeyDown(keycodes.W)) then
        rotation[1] = rotation[1] - 10 * aimRotAmount
    elseif(input.GetKeyDown(keycodes.S)) then
        rotation[1] = rotation[1] + 10 * aimRotAmount
    end

    if(input.GetKeyDown(keycodes.A)) then
        rotation[3] = rotation[3] + 10 * aimRotAmount
    elseif(input.GetKeyDown(keycodes.D)) then
        rotation[3] = rotation[3] - 10 * aimRotAmount
    end

end

function system.update(entity)

    -- require Weapon and Transform components
    if entity.Weapon == nil or entity.Transform == nil then
        return nil
    end

    --print("From entity: "..entity.id.."... camera fov is "..camera_ent.Camera.fov.."");

    local new_rotation = {0, 0, -180}
    local rotate_speed = time.DeltaTime() * 20

    if(input.GetCursorState().is_locked == true ) then
        handle_input(entity, new_rotation)
    else
        entity.Transform.position = pos_standby
    end

    -- update rotation
    new_rotation[1] = math.Lerp(
        entity.Transform.orientation.x, new_rotation[1], rotate_speed);
    new_rotation[3] = math.Lerp(
        entity.Transform.orientation.z, new_rotation[3], rotate_speed);

    entity.Transform.orientation = new_rotation;
end

system.run = function() _ECS_RegisterSystem(system) end
return system
