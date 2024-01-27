cmake_minimum_required (VERSION 3.9)

project ("phosphene")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-g")
set(VCPKG_ROOT $ENV{VCPKG_ROOT})

set(cpps src/main.cpp
    src/phosphene.cpp
    src/camera.cpp
    src/phospheneCallback.cpp
    src/phosphenePipeline.cpp
    src/backend/phosStartVk.cpp
    src/backend/vkImpl.cpp
    src/backend/postPipeline.cpp
    src/backend/swapchain.cpp
    src/helper/command.cpp
    src/helper/allocator.cpp
    src/helper/phosHelper.cpp
    src/helper/extensions.cpp
    src/raytracing/sceneBuilder.cpp
    src/raytracing/pipelineBuilder.cpp
    src/raytracing/pipeline.cpp
    src/raytracing/rayPicker.cpp
    src/sceneLoader/sceneLoader.cpp
    src/sceneLoader/objLoader.cpp
    src/sceneLoader/vdbLoader.cpp
    src/sceneLoader/scene.cpp
	src/phosGui.cpp
)

set(hpps src/phosphene.hpp
    src/camera.hpp
    src/backend/phosStartVk.hpp
    src/backend/vkImpl.hpp
    src/helper/phosHelper.hpp
    src/helper/command.hpp
    src/helper/allocator.hpp
    src/helper/extensions.hpp
    src/raytracing/sceneBuilder.hpp
    src/raytracing/pipelineBuilder.hpp
    src/raytracing/rayPicker.hpp
    src/sceneLoader/sceneLoader.hpp
    src/sceneLoader/objLoader.hpp
    src/sceneLoader/vdbLoader.hpp
    src/sceneLoader/scene.hpp
    shaders/hostDevice.h
)

add_executable(${PROJECT_NAME} ${cpps} ${hpps})

include(FindVulkan)
target_link_libraries(${PROJECT_NAME} PRIVATE Vulkan::Vulkan)

include(${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake)

find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw)

find_package(imgui CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE imgui::imgui)

find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)

find_package(tinyobjloader CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE tinyobjloader::tinyobjloader)

find_package(OpenVDB CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC OpenVDB::openvdb)

find_package(TBB CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE TBB::tbb TBB::tbbmalloc)

find_package(blosc CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE blosc_static)
