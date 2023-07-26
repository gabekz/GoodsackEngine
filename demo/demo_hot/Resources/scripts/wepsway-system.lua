--- @system wepsway
local system = {}

local maths = require("maths")
local Time = require('GoodsackAPI.Time')

function system.update(entity)
    if entity.WeaponSway == nil or entity.Transform == nil then
        return nil
    end

    -- target rotation
    
    local axis = Input.get_cursor_axis()

    local sway_amount = 0.8
    local sway_speed = Time.get_delta_time() * 3.7

    local y_offset = axis[1] * sway_amount
    local x_offset = axis[2] * sway_amount

    entity.Transform.orientation = {
        maths.lerp(entity.Transform.orientation.x, x_offset, sway_speed),
        maths.lerp(entity.Transform.orientation.y, -y_offset, sway_speed), 
        maths.lerp(entity.Transform.orientation.y, -y_offset, sway_speed)
    }

end

system.run = function() _ECS_RegisterSystem(system) end
return system
