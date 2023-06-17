local system = {}
local KeyCode = require("keycodes")

pos_aiming = {
    -0.318,
    -0.187,
    -0.2
}

pos_standby = {
    -0.1,
    -0.22,
    -0.4340
}

testValue = 0

function lerp(start, goal, alpha)
	return start + (goal - start) * alpha
end

function clamp(value, min, max)
    if value < min then value = min end
    if value > max then value = max end
end

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
    if(Input:GetKeyDown(KeyCode.R)) then
        entity.Transform.position = pos_aiming
    else
        entity.Transform.position = pos_standby
    end

    if(Input:GetKeyDown(KeyCode.W)) then
        --entity.Transform.orientation = entity.Weapon.rot_starting
        cntRot = entity.Transform.orientation

        if testValue >= 0 and testValue < 1 then
            testValue = testValue + 1 * 10 * delta_time()
        end

        --clamp(tesValue, 0, 1)
        newRot[1] = lerp(0, -10, testValue)
    end
    if(Input:GetKeyDown(KeyCode.S)) then
        --entity.Transform.orientation = entity.Weapon.rot_starting
        cntRot = entity.Transform.orientation
        newRot[1] = 10
        testValue = 0
    end

    if(Input:GetKeyDown(KeyCode.A)) then
        --entity.Transform.orientation = entity.Weapon.rot_starting
        cntRot = entity.Transform.orientation
        newRot[3] = -175
    end
    if(Input:GetKeyDown(KeyCode.D)) then
        --entity.Transform.orientation = entity.Weapon.rot_starting
        cntRot = entity.Transform.orientation
        newRot[3] = -190
    end
    entity.Transform.orientation = newRot
end

system.run = function() _ECS_RegisterSystem(system) end
return system
