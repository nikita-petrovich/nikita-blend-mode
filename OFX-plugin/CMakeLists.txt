cmake_minimum_required(VERSION 3.10.0...4.0.3)
project(NP_BlendMode VERSION 0.1.0 LANGUAGES C CXX)

# ========================== build platform ==========================

MESSAGE (STATUS "  Platform:")
MESSAGE (STATUS "    Host: "             ${CMAKE_HOST_SYSTEM_NAME} " " ${CMAKE_HOST_SYSTEM_VERSION} " " ${CMAKE_HOST_SYSTEM_PROCESSOR})
if(CMAKE_CROSSCOMPILING)
  MESSAGE (STATUS "    Target: "         ${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_VERSION} ${CMAKE_SYSTEM_PROCESSOR})
endif()
MESSAGE (STATUS "    CMake: "            ${CMAKE_VERSION})
MESSAGE (STATUS "    CMake generator: "  ${CMAKE_GENERATOR})
MESSAGE (STATUS "    CMake build tool: " ${CMAKE_BUILD_TOOL})
if(MSVC)
  MESSAGE (STATUS "    MSVC: "           ${MSVC_VERSION})
endif()
if(CMAKE_GENERATOR MATCHES Xcode)
  MESSAGE (STATUS "    Xcode: "          ${XCODE_VERSION})
endif()
if(NOT CMAKE_GENERATOR MATCHES "Xcode|Visual Studio")
  MESSAGE (STATUS "    Configuration: "  ${CMAKE_BUILD_TYPE})
endif()

# ===================================================================

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "-Wall -Wextra -Wpedantic -Wno-unused-parameter -O0 -DDEBUG")

# Open FX Library
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/openfx/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/openfx/Support/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/openfx/Support/Library) # For the one header in the Library folder

set(OPENFX_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/openfx/Support/Library)
set(OPENFX_SOURCES
    ${OPENFX_SRC_DIR}/ofxsCore.cpp
    ${OPENFX_SRC_DIR}/ofxsImageEffect.cpp
    ${OPENFX_SRC_DIR}/ofxsInteract.cpp
    ${OPENFX_SRC_DIR}/ofxsLog.cpp
    ${OPENFX_SRC_DIR}/ofxsMultiThread.cpp
    ${OPENFX_SRC_DIR}/ofxsParams.cpp
    ${OPENFX_SRC_DIR}/ofxsProperty.cpp
    ${OPENFX_SRC_DIR}/ofxsPropertyValidation.cpp
  )
add_library(OpenFX STATIC ${OPENFX_SOURCES})

# The Plugin
set(PLUGIN_SOURCES
    "main.cpp")

add_library(${PROJECT_NAME} SHARED ${PLUGIN_SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE OpenFX)
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")
set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".ofx")
set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-fvisibility=hidden")

set(OFX_ARCH "MacOS")

INSTALL(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}.ofx.bundle/Contents/${OFX_ARCH})
