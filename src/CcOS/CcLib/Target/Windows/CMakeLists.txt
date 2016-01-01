########
# Add Source Files
########
file (GLOB SOURCE_FILES
      "src/*.c"
      "src/*.cpp"
      "USB/src/*.cpp")


########
# Add Source Files
########
file (GLOB HEADER_FILES
      "*.h"
      "USB/*.h")


include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )

add_library(CcTarget STATIC ${SOURCE_FILES} ${HEADER_FILES} )

link_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../lib )

set_property( TARGET CcTarget PROPERTY FOLDER "CcLib")

target_link_libraries ( CcTarget LINK_PUBLIC
                              CcGui
                              CcKernel
                              CcCore
                              gdiplus
                              setupapi
                              hid
                              Ws2_32 )

target_include_directories( CcTarget PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})