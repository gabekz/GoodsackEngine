local system = {}
--system.run = function() system.start(nil) end

--local system = CreateSystem()
--

function system.start(e)
    print("From start()")
    print("received "..e.."")
end

function system.update(e)
    print("From update()")
    print("received "..e.."")
end

system.run = function() _ECS_RegisterSystem(system) end
return system
