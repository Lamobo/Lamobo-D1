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
                     ${CMAKE_CURRENT_SOURCE_DIR}/../../CcCore
                     ${CMAKE_CURRENT_SOURCE_DIR}/../../CcGui
                     ${CMAKE_CURRENT_SOURCE_DIR}/../../CcTarget
                     ${CMAKE_CURRENT_SOURCE_DIR}/../../ThirdParty )

add_library( CcSql STATIC ${SOURCE_FILES} ${HEADER_FILES} )

link_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../lib )

set_property( TARGET CcSql PROPERTY FOLDER "CcUtil")

target_link_libraries ( CcSql LINK_PUBLIC
                              CcKernel
                              CcCore )
