cmake_minimum_required(VERSION 3.13)

project(simple_variant VERSION 0.1)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  set(SV_PRIMARY_PROJECT true)
  set(CMAKE_EXPORT_COMPILE_COMMANDS true)
endif()

if(NOT TARGET simple_mp)
  find_package(simple_mp REQUIRED)
endif()

add_library(${PROJECT_NAME} INTERFACE)
add_library(simple::variant ALIAS ${PROJECT_NAME})

target_include_directories(
  ${PROJECT_NAME} INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

target_link_libraries(${PROJECT_NAME} INTERFACE simple_mp)

if(BUILD_TESTING AND SV_PRIMARY_PROJECT)
  add_subdirectory(test)
endif()

install(
  TARGETS ${PROJECT_NAME}
  EXPORT "${PROJECT_NAME}Targets"
  PUBLIC_HEADER DESTINATION include
)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
  "${PROJECT_NAME}ConfigVersion.cmake"
  VERSION ${PACKAGE_VERSION}
  COMPATIBILITY AnyNewerVersion
)

install(
  EXPORT "${PROJECT_NAME}Targets"
  FILE "${PROJECT_NAME}Config.cmake"
  DESTINATION lib/cmake/${PROJECT_NAME}
)

install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/ DESTINATION include)
