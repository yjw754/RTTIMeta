cmake_minimum_required(VERSION 3.14...3.22)

# ---- Project ----

# Note: update this to your new project's name and version
project(
  RTTIMeta
  VERSION 0.1
  LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 14)             
set(CMAKE_CXX_STANDARD_REQUIRED ON)   

# ---- Include guards ----

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(
    FATAL_ERROR
      "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there."
  )
endif()

# ---- Add dependencies via CPM ----
# see https://github.com/TheLartians/CPM.cmake for more info

include(cmake/CPM.cmake)

# PackageProject.cmake will be used to make our target installable
CPMAddPackage(
  NAME PackageProject.cmake
  GITHUB_REPOSITORY TheLartians/PackageProject.cmake
  VERSION 1.0
)

# ---- Add source files ----

# Note: globbing sources is considered bad practice as CMake's generators may not detect new files
# automatically. Keep that in mind when changing files, or explicitly mention them here.
# RTTIMetaHeaders 是一个 辅助性目标，专门用于：
# 明确列出头文件列表；
# 提供给 IDE 显示；
# 可用于 install、打包或检查用途；
# 不会编译、不参与构建流程。
# 它不是必须的，但对可维护性、可视化和开发体验非常有帮助
file(GLOB_RECURSE headers CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
add_library(RTTIMetaHeaders EXCLUDE_FROM_ALL ${headers})
set_target_properties(RTTIMetaHeaders PROPERTIES LINKER_LANGUAGE CXX)

# ---- Create library ----
# Note: for header-only libraries change all PUBLIC flags to INTERFACE and create an interface
# target: add_library(${PROJECT_NAME} INTERFACE)
add_library(${PROJECT_NAME} INTERFACE)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 14)

# being a cross-platform target, we enforce standards conformance on MSVC
target_compile_options(${PROJECT_NAME} INTERFACE "$<$<BOOL:${MSVC}>:/permissive->")

# Link dependencies
# target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt)

target_include_directories(
  ${PROJECT_NAME} INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
                         $<INSTALL_INTERFACE:include/${PROJECT_NAME}-${PROJECT_VERSION}>
)

# ---- Create an installable target ----
# this allows users to install and find the library via `find_package()`.

# the location where the project's version header will be placed should match the project's regular
# header paths

packageProject(
  NAME ${PROJECT_NAME}
  VERSION ${PROJECT_VERSION}
  BINARY_DIR ${PROJECT_BINARY_DIR}
  INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include
  INCLUDE_DESTINATION include/${PROJECT_NAME}-${PROJECT_VERSION}
)
