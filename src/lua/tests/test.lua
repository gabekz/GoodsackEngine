a = 5 + 52

PlayerName = "E"

system = {}

Player = {
    Age = 32,
    Name = "Edward"
}

function Add(a, b)
    print("\n[Lua] Add "..a.."+"..b.."")
    return a + b
end

------------------------------------------
--- Testing metatables for data-overwrite

function Update(e)
    -- metatable __index
    --print(e.value)
    --print(e.rand)
    print(e.id)
    print(e.data.value)

    --print(e.subset.health)


    -- metatable __newindex
    e.data.value = 8
    e.data.rand = 9
end

function system.test()

end