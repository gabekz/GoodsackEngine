local system = {}

local maths = require("maths")
local keycodes = require("keycodes")
local Time = require('GoodsackAPI.Time')

local pos_aiming = {
    -0.318,
    -0.187,
    -0.2
}

local pos_standby = {
    -0.1,
    -0.22,
    -0.4340
}

local testValue = 0

function system.start(entity)

    if not (entity.Weapon) then
        return nil
    end
end

function system.update(entity)

    -- require Weapon and Transform components
    if entity.Weapon == nil or entity.Transform == nil then
        return nil
    end

    last_position = entity.Transform.scale
    -- last_position = entity.Weapon.pos_starting

    newRot = {0, 0, -180}

    -- Aiming
    if(Input.GetKeyDown(keycodes.R)) then
        entity.Transform.position = pos_aiming
    else
        entity.Transform.position = pos_standby
    end

    if(Input.GetKeyDown(keycodes.W)) then
        --entity.Transform.orientation = entity.Weapon.rot_starting
        cntRot = entity.Transform.orientation

        if testValue >= 0 and testValue < 1 then
            testValue = testValue + 1 * 10 * Time.get_delta_time()
        end

        newRot[1] = maths.lerp(0, -10, testValue)
    end

    if(Input.GetKeyDown(keycodes.S)) then
        --entity.Transform.orientation = entity.Weapon.rot_starting
        cntRot = entity.Transform.orientation
        newRot[1] = 10
        testValue = 0
    end

    if(Input.GetKeyDown(keycodes.A)) then
        --entity.Transform.orientation = entity.Weapon.rot_starting
        cntRot = entity.Transform.orientation
        newRot[3] = -175
    end
    if(Input.GetKeyDown(keycodes.D)) then
        --entity.Transform.orientation = entity.Weapon.rot_starting
        cntRot = entity.Transform.orientation
        newRot[3] = -190
    end
    entity.Transform.orientation = newRot
end

system.run = function() _ECS_RegisterSystem(system) end
return system
