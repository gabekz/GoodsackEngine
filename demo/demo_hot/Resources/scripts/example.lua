VECTOR_LIB = {
    __gc = vector_gc,
    __newindex = vector_get_x,

    __index = { -- meth
        DoSomething = nil,
    }
}

static_meth = {
    new = vector_new,
}

static_meta = {
    __index = vector_index,
    __call = vector_new,
}
setmetatable(static_meth, static_meta)