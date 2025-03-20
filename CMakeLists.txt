cmake_minimum_required(VERSION 3.10)
project(RayTracing)

set(CMAKE_CXX_STANDARD 17)

include_directories(${PROJECT_SOURCE_DIR}/include)

file(GLOB_RECURSE SRC_FILES "${PROJECT_SOURCE_DIR}/src/*.cpp")
add_executable(RayTracing ${SRC_FILES})

#add_executable(RayTracing src/main.cpp include/Object.hpp src/Vector.cpp include/Vector.hpp include/Sphere.hpp include/global.hpp include/Triangle.hpp src/Scene.cpp
#        include/Scene.hpp include/Light.hpp include/AreaLight.hpp src/BVH.cpp include/BVH.hpp include/Bounds3.hpp include/Ray.hpp include/Material.hpp include/Intersection.hpp
#        src/Renderer.cpp include/Renderer.hpp
#        include/Texture.h
#        include/imageTexture.h
#        include/stb_image.h
#        include/ConstantTexture.h
#        src/Texture.cpp
#        src/Material.cpp
#        src/stb_image.cpp)

set(CMAKE_CXX_FLAGS "${CAMKE_CXX_FLAGS} -O3 -fopenmp")