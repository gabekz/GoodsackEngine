--[[
 -- Copyright (c) 2023, Gabriel Kutuzov
 -- SPDX-License-Identifier: MIT
 --]]

require('goodsack')

--[[---------------------------------------------------------
	Shared Modules
-----------------------------------------------------------]]
input       = require('goodsack.input')
time        = require('goodsack.time')
fs          = require('goodsack.filesystem')

Vector = require('goodsack.vector')

--[[---------------------------------------------------------
	Aliases
-----------------------------------------------------------]]

local include_dir = fs.URI([[gsk://../gsk/gsk/api/lua/include/]])

-- dofile() alias for specified directory
--
---@param path string
include = function(path)
    str = include_dir .. path
    dofile(str)
end

--
-- Alias for immediately running ECS Systems
--
SYSTEM_RUN = function(name)
    require(name).run()
end



--[[---------------------------------------------------------
	Extensions
-----------------------------------------------------------]]
include('extension/math.lua')
