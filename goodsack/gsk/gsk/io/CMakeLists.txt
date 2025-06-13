add_library(Goodsack_io parse_image.c serialize_model.c)
target_include_directories(Goodsack_io PUBLIC ${SRC_DIR} ThirdParty_cglm)

#target_link_libraries(Goodsack::io
#	Goodsack::core
#	Goodsack::ecs
#	Goodsack::tools
#	# thirdparty
#	cglm::cglm
#)

#add_library(Goodsack::asset::import ${SRCS}/import/**)