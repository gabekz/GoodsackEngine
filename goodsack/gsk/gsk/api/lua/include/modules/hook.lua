local hook = {}

local Hooks = {}

function hook.GetTable()
	return Hooks
end

function hook.Add(event_name, name, func)

    if ( Hooks[ event_name ] == nil ) then
		Hooks[ event_name ] = {}
        print('adding HOOK (' .. event_name .. ')')
	end

	Hooks[ event_name ][ name ] = func
end

function hook.Run(name, ...)

    local HookTable = Hooks[ name ]
	if ( HookTable == nil ) then return nil end

    local a, b, c, d, e, f

	for k, v in pairs( HookTable ) do

        -- check if k = isstring
		if ( ( k ) ) then

			--
			-- If it's a string, it's cool
			--
			a, b, c, d, e, f = v( ... )

		else

			--
			-- If the key isn't a string - we assume it to be an entity
			-- Or panel, or something else that IsValid works on.
			--
			if ( IsValid( k ) ) then
				--
				-- If the object is valid - pass it as the first argument (self)
				--
				a, b, c, d, e, f = v( k, ... )
			else
				--
				-- If the object has become invalid - remove it
				--
				HookTable[ k ] = nil
			end
		end

		--
		-- Hook returned a value - it overrides the gamemode function
		--
		if ( a ~= nil ) then
			return a, b, c, d, e, f
		end

    end
end

return hook