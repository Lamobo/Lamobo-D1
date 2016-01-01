cmake_minimum_required (VERSION 2.6)
project(CcTarget)

########
# Add Source Files
########
file (GLOB SOURCE_FILES
      "src/*.c"
      "src/*.cpp"
      "extern/src/stm32f4-hal/*.c"
      "extern/src/cmsis/*.c"
      "extern/src/cortexm/*.c"
      "extern/src/diag/*.c"
      "extern/src/newlib/*.c")


########
# Add Source Files
########
file (GLOB HEADER_FILES
      "*.h"
      "extern/include/*.h"
      "extern/include/arm/*.h"
      "extern/include/cmsis/*.h"
      "extern/include/cortexm/*.h"
      "extern/include/diag/*.h"
      "extern/include/stm32f4-hal/*.h" )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/
                     ${CMAKE_CURRENT_SOURCE_DIR}/extern/include
                     ${CMAKE_CURRENT_SOURCE_DIR}/extern/include/arm
                     ${CMAKE_CURRENT_SOURCE_DIR}/extern/include/cmsis
                     ${CMAKE_CURRENT_SOURCE_DIR}/extern/include/cortexm
                     ${CMAKE_CURRENT_SOURCE_DIR}/extern/include/diag
                     ${CMAKE_CURRENT_SOURCE_DIR}/extern/include/stm32f4-hal
                     ${CMAKE_CURRENT_SOURCE_DIR}/../../CcKernel
                     ${CMAKE_CURRENT_SOURCE_DIR}/../../CcApp
                     ${CMAKE_CURRENT_SOURCE_DIR}/../../CcDev
                     ${CMAKE_CURRENT_SOURCE_DIR}/../../CcGui
                     ${CMAKE_CURRENT_SOURCE_DIR}/../../CcCore
                     ${CMAKE_CURRENT_SOURCE_DIR}/../../CcUtil )

add_library(CcTarget STATIC ${SOURCE_FILES} ${HEADER_FILES} )

link_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../lib )

target_link_libraries ( CcTarget LINK_PUBLIC
                              CcDev
                              CcGui
                              CcKernel
                              CcCore )
