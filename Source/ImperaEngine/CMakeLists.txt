add_library(ImperaEngine STATIC ImperaEngine.hpp
								ImperaEngine.cpp
								JsonFile.hpp
								JsonFile.cpp
								Map/Camera.cpp
								Map/Camera.hpp
								Map/Map.hpp
								Map/Map.cpp
								Map/MapChunkManager.hpp
								Map/MapChunkManager.cpp
								Map/MapCompiler.hpp
								Map/MapCompiler.cpp
								Map/MapRenderer.hpp
								Map/MapRenderer.cpp)
if(MINGW) 
target_compile_options(ImperaEngine PRIVATE -O3 -Wall)
endif()
target_include_directories(ImperaEngine PUBLIC ${IMPERA_ROOT_SOURCE_DIR})
target_link_libraries(ImperaEngine PUBLIC SDL3::SDL3 spdlog::spdlog nlohmann_json::nlohmann_json)