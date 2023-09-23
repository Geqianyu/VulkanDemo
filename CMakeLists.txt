# 要求的 CMake 最低版本
cmake_minimum_required(VERSION 3.5)

# 项目名称
project(VulkanDemo)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)

# 设置输出可执行文件路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
# message(${EXECUTABLE_OUTPUT_PATH})

# 设置输出静态库文件路径
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
# message(${LIBRARY_OUTPUT_PATH})

# 设置 GLFW 的头文件路径
set(GLFW_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/glfw/include)
# message(${GLFW_INCLUDE_DIR})

# 设置 glm 的头文件路径
set(GLM_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/glm)
# message(${GLM_INCLUDE_DIR})

# 设置 stb 的头文件路径
set(STB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/stb)
# message(${STB_INCLUDE_DIR})

set(TINYOBJLOADER_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/tinyobjloader)
# message(${TINYOBJLOADER_INCLUDE_DIR})

# 设置 asset 的路径
set(ASSET_INCLUDE_PATH "\"${CMAKE_CURRENT_SOURCE_DIR}/assets/\"")

# 寻找 Vulkan 的路径，Vulkan_LIBRARIES 为 Vulkan 库，Vulkan_INCLUDE_DIRS 为 Vlukan 头文件所在地
find_package(Vulkan REQUIRED)
# message(${Vulkan_FOUND})
# message(${Vulkan_LIBRARIES})
# message(${Vulkan_INCLUDE_DIRS})

# 设置包含的文件
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src)
# message(${CMAKE_CURRENT_SOURCE_DIR}/src)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/glfw)
# message(${CMAKE_CURRENT_SOURCE_DIR}/vendor/glfw)