local system = {}
--system.run = function() system.start(nil) end

--local system = CreateSystem()
--

function system.start(e)
    print("From start()")

    e.ComponentCamera.speed = 200
    print("received "..e.ComponentCamera.speed.."")

end

function system.update(e)
    print("From update()")
    --print("received "..e.."")
end

system.run = function() _ECS_RegisterSystem(system) end
return system
