target_sources(C-OpenGL-Test PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/vulkanBackend.c
	${CMAKE_CURRENT_SOURCE_DIR}/vertex.c
	${CMAKE_CURRENT_SOURCE_DIR}/gpuBuffers.c
	${CMAKE_CURRENT_SOURCE_DIR}/timeWindows.c
	${CMAKE_CURRENT_SOURCE_DIR}/TextureImages/textureImages.c
	${CMAKE_CURRENT_SOURCE_DIR}/CommandBuffers/commandBuffers.c
	${CMAKE_CURRENT_SOURCE_DIR}/TextureSampler/textureSampler.c
)