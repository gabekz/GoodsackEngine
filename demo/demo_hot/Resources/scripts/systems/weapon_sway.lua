--- @system weapon_sway
local system = {}

local time = time
local input = input

function system.update(entity)
    -- check for required components
    if entity.WeaponSway == nil or entity.Transform == nil then
        return nil
    end

    local axis = {0, 0} -- default to empty

    -- we don't want to do any weapon sway unless the cursor state is locked
    if (input.GetCursorState().is_locked == true) then
        axis = input.GetCursorAxis()
    end

    -- target rotation
    
    local sway_amount = 0.8
    local sway_speed = time.DeltaTime() * 3.7

    local y_offset = axis[1] * sway_amount
    local x_offset = axis[2] * sway_amount

    entity.Transform.orientation = {
        math.Lerp(entity.Transform.orientation.x, -x_offset, sway_speed),
        math.Lerp(entity.Transform.orientation.y, y_offset, sway_speed), 
        math.Lerp(entity.Transform.orientation.y, -y_offset, sway_speed)
    }

end

system.run = function() _ECS_RegisterSystem(system) end
return system
