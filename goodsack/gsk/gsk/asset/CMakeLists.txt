add_library(Goodsack::asset ${SRCS})

target_link_libraries(Goodsack::asset 
	Goodsack::core
	Goodsack::ecs
	Goodsack::tools
	# thirdparty
	cglm::cglm
)

add_library(Goodsack::asset::import ${SRCS}/import/**)

target_link_libraries(Goodsack::asset::import
	Goodsack::asset
)