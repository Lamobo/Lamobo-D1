# @Author Andreas Dirmeier
#
# Main File to build all CcLib Components
#
# To Generate a Solution execute the command:
#   cmake -G "Visual Studio 12" -DTARGET_SYSTEM="Windows"
#
cmake_minimum_required (VERSION 2.6)

###################
# Compiler Settings
###################
set(CMAKE_SYSTEM_NAME           Linux )
set(CMAKE_SYSTEM_PROCESSOR      armv5te )
set(CMAKE_SYSROOT               ${CMAKE_CURRENT_SOURCE_DIR}/../../output/busybox/rootfs )
set(CMAKE_C_COMPILER            ${CMAKE_CURRENT_SOURCE_DIR}/../../compiler/arm-2009q3/bin/arm-none-linux-gnueabi-gcc)
set(CMAKE_CXX_COMPILER          ${CMAKE_CURRENT_SOURCE_DIR}/../../compiler/arm-2009q3/bin/arm-none-linux-gnueabi-g++)
set(CMAKE_FIND_ROOT_PATH        ${CMAKE_CURRENT_SOURCE_DIR}/../../output/busybox/rootfs )
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

###################
# Solution Name:
###################
project(Main)

###################
# Global Settings:
###################
set(CMAKE_VERBOSE_MAKEFILE            ON)
set(EXECUTABLE_OUTPUT_PATH            ${CMAKE_CURRENT_SOURCE_DIR}/bin )
set(LIBRARY_OUTPUT_PATH               ${CMAKE_CURRENT_SOURCE_DIR}/bin )
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
if( NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build, options are: None (CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()

###################
# Add CcLib to compile
###################
add_subdirectory(CcLib/)

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}
                     ${CMAKE_CURRENT_SOURCE_DIR}/CcLib/CcCore
                     ${CMAKE_CURRENT_SOURCE_DIR}/CcLib/CcKernel
                     ${CMAKE_CURRENT_SOURCE_DIR}/CcLib/CcUtil
                     ${CMAKE_CURRENT_SOURCE_DIR}/CcLib/CcGui
                     ${CMAKE_CURRENT_SOURCE_DIR}/CcLib/ThirdParty )

add_executable( CcCLIApp main.cpp )

link_directories( ${LIBRARY_OUTPUT_PATH} )

target_link_libraries ( CcCLIApp LINK_PUBLIC
                              CcKernel
                              CcCore
                              CcTarget
                              CcNetwork
                              CcFtp )


#message( ${LIBRARY_OUTPUT_PATH} )
