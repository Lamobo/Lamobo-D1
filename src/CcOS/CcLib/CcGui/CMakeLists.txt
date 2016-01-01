########
# Add Source Files
########
file (GLOB SOURCE_FILES
      "src/*.c"
      "src/*.cpp"
      "StyleSheets/src/*.c"
      "StyleSheets/src/*.cpp")


########
# Add Source Files
########
file (GLOB HEADER_FILES
      "*.h"
      "StyleSheets/*.h")


include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )
                     
add_library(CcGui STATIC ${SOURCE_FILES} ${HEADER_FILES} )

link_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../lib )

set_property( TARGET CcGui PROPERTY FOLDER "CcLib")

target_link_libraries ( CcGui LINK_PUBLIC
                              CcKernel
                              CcCore )

target_include_directories( CcGui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})