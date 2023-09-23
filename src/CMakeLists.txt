# 要求的 CMake 最低版本
cmake_minimum_required(VERSION 3.5)

# 项目名称
project(VulkanDemo)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)

# 添加静态库目录
link_directories(${LIBRARY_OUTPUT_PATH})

# 添加宏定义
add_definitions(-DGLFW_INCLUDE_VULKAN)
add_definitions(-DASSET_INCLUDE_PATH=${ASSET_INCLUDE_PATH})
add_definitions(-DGLM_FORCE_RADIANS)
add_definitions(-DGLM_ENABLE_EXPERIMENTAL)
add_definitions(-DSTB_IMAGE_IMPLEMENTATION)
add_definitions(-DGLM_FORCE_DEPTH_ZERO_TO_ONE)
add_definitions(-DTINYOBJLOADER_IMPLEMENTATION)

# 添加头文件目录
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/common
    ${CMAKE_CURRENT_SOURCE_DIR}/Application
    ${Vulkan_INCLUDE_DIRS}
    ${GLFW_INCLUDE_DIR}
    ${GLM_INCLUDE_DIR}
    ${STB_INCLUDE_DIR}
    ${TINYOBJLOADER_INCLUDE_DIR}
)

# 添加源文件
file(GLOB_RECURSE SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

# 输出可执行文件
add_executable(VulkanDemo ${SRC})

# 链接静态库
target_link_libraries(VulkanDemo
    ${Vulkan_LIBRARIES}    
    glfw3
)