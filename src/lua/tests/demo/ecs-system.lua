local system = {}
--system.run = function() system.start(nil) end

--local system = CreateSystem()


function system.start(e)
    print("From start()")
    return 1
end

function system.update(e)
    print("From update()")
end

system.run = function() _ECS_RegisterSystem(system) end
return system
