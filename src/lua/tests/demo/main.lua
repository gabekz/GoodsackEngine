-- creating an eventlist for future reference
package.path = '../src/lua/tests/demo/?.lua;' .. package.path
require("ecs-system").run()

function main()
    print("Hello world!")
end

function test() 
    -- loading some textures
    texture1 = Texture.Create({
        texture_path = "../res/textures/box/diffuse.png",
        texture_bpp = 4
    })
    texture2 = Texture.Create({
        texture_path = "../res/textures/box/normal.png",
        texture_bpp = 4
    })

    -- create an entity
    box = {
        ComponentTransform = {},
        ComponentMesh      = {}
    }

    CreateEntity(box)
end
