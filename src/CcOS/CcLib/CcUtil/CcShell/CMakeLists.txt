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


include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )

add_library( CcShell STATIC ${SOURCE_FILES} ${HEADER_FILES} )

link_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../lib )

set_property( TARGET CcShell PROPERTY FOLDER "CcUtil")

target_link_libraries ( CcShell LINK_PUBLIC
                              CcKernel
                              CcCore )
                              
target_include_directories( CcShell PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})