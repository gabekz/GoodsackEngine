--[[
 -- Copyright (c) 2023, Gabriel Kutuzov
 -- SPDX-License-Identifier: MIT
 --]]

function math.Lerp(start, goal, alpha)
    return start + (goal - start) * alpha;
end

function math.Clamp(value, min, max)
    if value < min then value = min end
    if value > max then value = max end
end