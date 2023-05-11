local system = {}
--system.run = function() system.start(nil) end

--local system = CreateSystem()
--

function system.update(e)
    print("From update() in ecs-system2")
    print("received "..e.id.."ecs-system2")
end

system.run = function() _ECS_RegisterSystem(system) end
return system
