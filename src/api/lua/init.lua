require('goodsack')

local include_dir = [[e:\Projects\GoodsackEngine\src\api\lua\include\]]

-- dofile() alias for specified directory
--
---@param path string
include = function(path)
    dofile(include_dir .. path)
end

--
-- Alias for immediately running ECS Systems
--
SYSTEM_RUN = function(name)
    require(name).run()
end

--[[---------------------------------------------------------
	Shared Modules
-----------------------------------------------------------]]
input       = require('goodsack.input')
time        = require('goodsack.time')

Vector = require('goodsack.vector')


--[[---------------------------------------------------------
	Extensions
-----------------------------------------------------------]]
include('extension/math.lua')