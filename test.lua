a = 5 + 52

PlayerName = "E"

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
    print(e.value)
    e.value = 8
    e.rand = 9
end
