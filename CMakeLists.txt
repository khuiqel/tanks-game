cmake_minimum_required(VERSION 3.16...3.31)
project(tanks-game)

#note: when changing these defaults, you have to delete CMakeCache.txt
option(USE_SYSTEM_GLFW "Use the system GLFW package" ON)
option(USE_RPMALLOC "Enable rpmalloc for extra performance" ON)
option(USE_TRACY "Enable Tracy for performance profiling" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED False)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_C_STANDARD 17)

if(NOT CMAKE_BUILD_TYPE)
	#options: Debug, Release, RelWithDebInfo, MinSizeRel
	set(CMAKE_BUILD_TYPE "Release")
	add_compile_definitions(RELEASE_DISTRIBUTION) #turns off a few things to feel more like an end-user product
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	#nothing
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	# maybe MATCHES would be better than STREQUAL as there are several Clang derivatives: https://cmake.org/cmake/help/latest/variable/CMAKE_LANG_COMPILER_ID.html
	add_compile_options(-Wno-switch)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
	#TODO: for some reason every file is compiled twice
	add_compile_options(/MP)
	if(USE_RPMALLOC)
		add_compile_options(/experimental:c11atomics)
	endif()
else()
	message(WARNING "Unknown compiler, assuming it's fine...")
endif()

if(MSVC)
	#change this yourself if desired
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:SSE4.2")
	set(CMAKE_C_FLAGS     "${CMAKE_C_FLAGS} /arch:SSE4.2")
else()
	if(NOT CMAKE_CXX_FLAGS MATCHES "-march")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=x86-64-v2") #SSE4.2
		# NOTE: if your compiler does not support x86-64 feature levels (which were defined in 2020), then use "-msse4.2 -mpopcnt" or "-march=nehalem" instead
		# "-mpopcnt" shouldn't be required because all CPUs that have SSE4.2 also have POPCNT, but some compilers don't enable it, however I don't even know if this project would use the instruction
		# alternatively, use "-march=native"
	endif()
	if(NOT CMAKE_C_FLAGS MATCHES "-march")
		# feels super unnecessary when there's literally only two C files
		set(CMAKE_C_FLAGS     "${CMAKE_C_FLAGS} -march=x86-64-v2") #SSE4.2
	endif()
endif()

if(MSVC)
	add_compile_options(/wd4018 /wd4244 /wd4267 /wd4305 /wd4312 /wd4838)
else()
	set(CMAKE_C_FLAGS     "${CMAKE_C_FLAGS} -Wno-narrowing -Wno-int-to-pointer-cast")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-narrowing -Wno-int-to-pointer-cast -Wno-delete-incomplete")
	if(USE_TRACY)
		SET_SOURCE_FILES_PROPERTIES(Externals/tracy/public/TracyClient.cpp PROPERTIES COMPILE_FLAGS "--no-warnings") #"-w" is so archaic
	endif()
endif()

find_package(OpenGL REQUIRED)
if(USE_SYSTEM_GLFW)
	if(NOT MSVC)
		find_package(glfw3 REQUIRED)
	endif()
else()
	set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
	set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
	set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
	if(NOT WIN32)
		# If you are having display issues, you might want to disable Wayland to force X11:
		#set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "" FORCE)
	endif()
	add_subdirectory(Externals/glfw EXCLUDE_FROM_ALL)
endif()

if(USE_RPMALLOC)
	set(RPMALLOC_SOURCE_FILES Dependencies/rpmalloc/rpmalloc.c)
else()
	set(RPMALLOC_SOURCE_FILES)
endif()

if(USE_TRACY)
	add_compile_definitions(TRACY_ENABLE)
	set(TRACY_SOURCE_FILES Externals/tracy/public/TracyClient.cpp)
else()
	set(TRACY_SOURCE_FILES)
endif()

add_executable(${PROJECT_NAME}
	Dependencies/enkiTS/TaskScheduler.cpp
	Dependencies/glad2/src/gl.c
	${RPMALLOC_SOURCE_FILES}
	${TRACY_SOURCE_FILES}
	tanks-game/_source/aaa_first.cpp
	tanks-game/_source/background-rect.cpp
	tanks-game/_source/basic-ini-parser.cpp
	tanks-game/_source/bullet-manager.cpp
	tanks-game/_source/bullet.cpp
	tanks-game/_source/circle-hazard.cpp
	tanks-game/_source/collision-handler.cpp
	tanks-game/_source/color-cache.cpp
	tanks-game/_source/color-mixer.cpp
	tanks-game/_source/color-value-holder.cpp
	tanks-game/_source/constants.cpp
	tanks-game/_source/custom-level-interpreter.cpp
	tanks-game/_source/custom-power-interpreter.cpp
	tanks-game/_source/developer-manager.cpp
	tanks-game/_source/end-game-handler.cpp
	tanks-game/_source/frame-time-graph.cpp
	tanks-game/_source/game-main-loop.cpp
	tanks-game/_source/game-manager.cpp
	tanks-game/_source/game-scene-manager.cpp
	tanks-game/_source/game-scene.cpp
	tanks-game/_source/game-settings.cpp
	tanks-game/_source/game-thing.cpp
	tanks-game/_source/generalized-lava.cpp
	tanks-game/_source/generalized-lightning.cpp
	tanks-game/_source/generalized-no-bullet-zone.cpp
	tanks-game/_source/generic-factory-construction-data.cpp
	tanks-game/_source/graphics/index-buffer.cpp
	tanks-game/_source/graphics/opengl-index-buffer.cpp
	tanks-game/_source/graphics/opengl-rendering-context.cpp
	tanks-game/_source/graphics/opengl-shader.cpp
	tanks-game/_source/graphics/opengl-vertex-array-object.cpp
	tanks-game/_source/graphics/opengl-vertex-buffer.cpp
	tanks-game/_source/graphics/shader.cpp
	tanks-game/_source/graphics/vertex-array-object.cpp
	tanks-game/_source/graphics/vertex-buffer.cpp
	tanks-game/_source/hazard-data-governor.cpp
	tanks-game/_source/hazard-manager.cpp
	tanks-game/_source/hazards/circular-lava-hazard.cpp
	tanks-game/_source/hazards/circular-lightning-hazard.cpp
	tanks-game/_source/hazards/circular-no-bullet-zone-hazard.cpp
	tanks-game/_source/hazards/cloud-hazard.cpp
	tanks-game/_source/hazards/dev-wall-hazard.cpp
	tanks-game/_source/hazards/ginormous-turret-hazard.cpp
	tanks-game/_source/hazards/gravity-well-hazard.cpp
	tanks-game/_source/hazards/horizontal-lightning-hazard.cpp
	tanks-game/_source/hazards/mother-turret-hazard.cpp
	tanks-game/_source/hazards/patrolling-turret-hazard.cpp
	tanks-game/_source/hazards/rectangular-lava-hazard.cpp
	tanks-game/_source/hazards/rectangular-lightning-hazard.cpp
	tanks-game/_source/hazards/rectangular-no-bullet-zone-hazard.cpp
	tanks-game/_source/hazards/reflecktor-hazard.cpp
	tanks-game/_source/hazards/spiral-lava-hazard.cpp
	tanks-game/_source/hazards/stationary-turret-hazard.cpp
	tanks-game/_source/hazards/targeting-turret-hazard.cpp
	tanks-game/_source/hazards/vertical-lightning-hazard.cpp
	tanks-game/_source/keypress-manager.cpp
	tanks-game/_source/level-data-governor.cpp
	tanks-game/_source/level-effect.cpp
	tanks-game/_source/level-effects/dev-symmetric-tanks-level-effect.cpp
	tanks-game/_source/level-effects/dev-tiny-tanks-level-effect.cpp
	tanks-game/_source/level-effects/ice-level-effect.cpp
	tanks-game/_source/level-effects/invisible-walls-level-effect.cpp
	tanks-game/_source/level-effects/magnetism-level-effect.cpp
	tanks-game/_source/level-effects/minefield-level-effect.cpp
	tanks-game/_source/level-effects/respawning-powerups-level-effect.cpp
	tanks-game/_source/level-effects/wind-level-effect.cpp
	tanks-game/_source/level-helper.cpp
	tanks-game/_source/level-manager.cpp
	tanks-game/_source/level.cpp
	tanks-game/_source/levels/big-fun-level.cpp
	tanks-game/_source/levels/concealed-powerups-level.cpp
	tanks-game/_source/levels/dangerous-center-level.cpp
	tanks-game/_source/levels/default-random-level.cpp
	tanks-game/_source/levels/dev-no-walls-level-1.cpp
	tanks-game/_source/levels/developer-level-0.cpp
	tanks-game/_source/levels/developer-level-1.cpp
	tanks-game/_source/levels/developer-level-2.cpp
	tanks-game/_source/levels/developer-level-3.cpp
	tanks-game/_source/levels/developer-level-4.cpp
	tanks-game/_source/levels/empty-level.cpp
	tanks-game/_source/levels/evened-corridors-level.cpp
	tanks-game/_source/levels/few-obstacles-level.cpp
	tanks-game/_source/levels/hiding-places-level.cpp
	tanks-game/_source/levels/lightning-corners-level.cpp
	tanks-game/_source/levels/lone-turret-level.cpp
	tanks-game/_source/levels/many-hazards-level.cpp
	tanks-game/_source/levels/mine-heaven-level.cpp
	tanks-game/_source/levels/old-empty-level.cpp
	tanks-game/_source/levels/old-evened-corridors-level.cpp
	tanks-game/_source/levels/old-few-obstacles-level.cpp
	tanks-game/_source/levels/old-hiding-places-level.cpp
	tanks-game/_source/levels/old-portal-level.cpp
	tanks-game/_source/levels/showcase-level-1.cpp
	tanks-game/_source/levels/showcase-level-2.cpp
	tanks-game/_source/levels/showcase-level-3.cpp
	tanks-game/_source/levels/sneaky-reward-level.cpp
	tanks-game/_source/levels/terrifying-chaos-level.cpp
	tanks-game/_source/levels/tight-patrolling-corridor-level.cpp
	tanks-game/_source/levels/timed-reward-level.cpp
	tanks-game/_source/levels/tricky-maneuvering-level.cpp
	tanks-game/_source/levels/unnamed-level-1.cpp
	tanks-game/_source/levels/unnamed-level-10.cpp
	tanks-game/_source/levels/unnamed-level-11.cpp
	tanks-game/_source/levels/unnamed-level-12.cpp
	tanks-game/_source/levels/unnamed-level-13.cpp
	tanks-game/_source/levels/unnamed-level-2.cpp
	tanks-game/_source/levels/unnamed-level-3.cpp
	tanks-game/_source/levels/unnamed-level-4.cpp
	tanks-game/_source/levels/unnamed-level-6.cpp
	tanks-game/_source/levels/unnamed-level-9.cpp
	tanks-game/_source/levels/untraditional-divider-level.cpp
	tanks-game/_source/levels/winning-path-level.cpp
	tanks-game/_source/mod-processor.cpp
	tanks-game/_source/mylib.cpp
	tanks-game/_source/physics-handler.cpp
	tanks-game/_source/power-function-helper.cpp
	tanks-game/_source/power-square.cpp
	tanks-game/_source/power.cpp
	tanks-game/_source/powers/ancient-mega-death-power.cpp
	tanks-game/_source/powers/annoying-power.cpp
	tanks-game/_source/powers/banana-power.cpp
	tanks-game/_source/powers/banana-split-power.cpp
	tanks-game/_source/powers/barrier-power.cpp
	tanks-game/_source/powers/big-named-power.cpp
	tanks-game/_source/powers/blast-power.cpp
	tanks-game/_source/powers/bounce-power.cpp
	tanks-game/_source/powers/dev-backwards-movement-power.cpp
	tanks-game/_source/powers/dev-colorless-long-invincible-named-power.cpp
	tanks-game/_source/powers/dev-long-invincible-named-power.cpp
	tanks-game/_source/powers/dev-movement-restarter-power.cpp
	tanks-game/_source/powers/dev-other-stuff-is-poison-power.cpp
	tanks-game/_source/powers/dev-pusher-power.cpp
	tanks-game/_source/powers/dev-weird-extra-cannons-power.cpp
	tanks-game/_source/powers/edge-sparks-power.cpp
	tanks-game/_source/powers/fire-named-power.cpp
	tanks-game/_source/powers/god-mode-power.cpp
	tanks-game/_source/powers/grenade-power.cpp
	tanks-game/_source/powers/homing-power.cpp
	tanks-game/_source/powers/inversion-power.cpp
	tanks-game/_source/powers/invincible-named-power.cpp
	tanks-game/_source/powers/mega-death-power.cpp
	tanks-game/_source/powers/mine-layer-power.cpp
	tanks-game/_source/powers/mines-power.cpp
	tanks-game/_source/powers/multishot-power.cpp
	tanks-game/_source/powers/old-big-named-power.cpp
	tanks-game/_source/powers/old-bounce-power.cpp
	tanks-game/_source/powers/old-mines-power.cpp
	tanks-game/_source/powers/ring-shooter-power.cpp
	tanks-game/_source/powers/shield-power.cpp
	tanks-game/_source/powers/shotgun-power.cpp
	tanks-game/_source/powers/speed-power.cpp
	tanks-game/_source/powers/swarm-power.cpp
	tanks-game/_source/powers/the-super-named-power.cpp
	tanks-game/_source/powers/tracking-power.cpp
	tanks-game/_source/powers/trickster-circle-power.cpp
	tanks-game/_source/powers/trickster-snake-power.cpp
	tanks-game/_source/powers/triple-named-power.cpp
	tanks-game/_source/powers/triple-spread-named-power.cpp
	tanks-game/_source/powers/ultra-bounce-power.cpp
	tanks-game/_source/powers/wall-sparks-power.cpp
	tanks-game/_source/powers/wallhack-power.cpp
	tanks-game/_source/powerup-data-governor.cpp
	tanks-game/_source/powerup-manager.cpp
	tanks-game/_source/priority-handler.cpp
	tanks-game/_source/rect-hazard.cpp
	tanks-game/_source/renderer.cpp
	tanks-game/_source/reset-things.cpp
	tanks-game/_source/rng.cpp
	tanks-game/_source/simple-vector-2d.cpp
	tanks-game/_source/tank-manager.cpp
	tanks-game/_source/tank.cpp
	tanks-game/_source/wall-manager.cpp
	tanks-game/_source/wall.cpp
	tanks-game/_source/window-initializer.cpp
	tanks-game/_source/main.cpp
)

if (MSVC)
	# Application icon:
	target_sources(${PROJECT_NAME} PRIVATE tanks-game/tanks.rc)
	# Startup project:
	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME}) #TODO: no longer works, is it because tanks-game.sln still exists?
	# Set the running directory to the build dir:
	set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/build")
endif()

if(MSVC AND USE_SYSTEM_GLFW)
	target_include_directories(${PROJECT_NAME} PRIVATE Dependencies/_misc/win32/glfw/include)
endif()
if(USE_TRACY)
	target_include_directories(${PROJECT_NAME} PRIVATE Externals/tracy/public)
endif()
target_include_directories(${PROJECT_NAME} PRIVATE Dependencies/enkiTS)
target_include_directories(${PROJECT_NAME} PRIVATE Dependencies/glad2/include)
target_include_directories(${PROJECT_NAME} PRIVATE Dependencies/glm)
target_include_directories(${PROJECT_NAME} PRIVATE Dependencies/rpmalloc)
target_include_directories(${PROJECT_NAME} PRIVATE Dependencies/stb)
target_include_directories(${PROJECT_NAME} PRIVATE Dependencies/_misc)

if(WIN32)
	if(MSVC AND USE_SYSTEM_GLFW)
		target_link_libraries(${PROJECT_NAME} PRIVATE ../Dependencies/_misc/win32/glfw/lib-vc2022/glfw3)
	else()
		target_link_libraries(${PROJECT_NAME} PRIVATE glfw)
	endif()
else()
	target_link_libraries(${PROJECT_NAME} PRIVATE GL)
	target_link_libraries(${PROJECT_NAME} PRIVATE glfw)
endif()
