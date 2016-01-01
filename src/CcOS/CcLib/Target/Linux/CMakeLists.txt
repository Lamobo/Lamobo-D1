########
# Add Source Files
########
file (GLOB SOURCE_FILES
      "src/*.c"
      "src/*.cpp")


########
# Add Source Files
########
file (GLOB HEADER_FILES
      "*.h")


include_directories( ${CMAKE_CURRENT_SOURCE_DIR}
                     ${CMAKE_CURRENT_SOURCE_DIR}/../../CcKernel
                     ${CMAKE_CURRENT_SOURCE_DIR}/../../CcGui
                     ${CMAKE_CURRENT_SOURCE_DIR}/../../CcCore )

add_library(CcTarget STATIC ${SOURCE_FILES} ${HEADER_FILES} )

link_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../../../bin )

set_property( TARGET CcTarget PROPERTY FOLDER "CcLib")

if( DEFINED CC_USE_GUI )
  target_link_libraries ( CcTarget LINK_PUBLIC
                              CcGui
                              CcKernel
                              CcCore
                              X11 )
else()
  target_link_libraries ( CcTarget LINK_PUBLIC
                              CcGui
                              CcKernel
                              CcCore )
endif()

target_link_libraries ( CcTarget LINK_PUBLIC
                              pthread )
