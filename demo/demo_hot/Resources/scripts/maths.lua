--- @module common
local maths = {}

function maths.lerp(start, goal, alpha)
    return start + (goal - start) * alpha;
end

function maths.clamp(value, min, max)
    if value < min then value = min end
    if value > max then value = max end
end


return maths
