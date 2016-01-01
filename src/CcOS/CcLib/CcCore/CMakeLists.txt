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

add_library( CcCore STATIC ${SOURCE_FILES} ${HEADER_FILES} )

set_property( TARGET CcCore PROPERTY FOLDER "CcLib")

target_include_directories( CcCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})