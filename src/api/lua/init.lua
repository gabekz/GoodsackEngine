require('goodsack')

-- local include_dir = [[/home/gmk/gksrc/projects/GoodsackEngine/src/api/lua/include/]]
-- require('goodsack.util').GetFullPath([[gsk:api//lua/include]], 'dir')
local include_dir = [[E:/Projects/GoodsackEngine/src/api/lua/include/]]

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
