add_library(Goodsack::core ${SRCS})

target_link_libraries(Goodsack::core
	Goodsack::core
	Goodsack::ecs
	Goodsack::tools
	# thirdparty
	cglm::cglm
)

add_library(Goodsack::context INTERFACE)
