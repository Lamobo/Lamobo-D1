########
# Add Source Files
########
file (GLOB KERNEL_SOURCE_FILES
      "src/*.c"
      "src/*.cpp" )


########
# Add Header Files
########
file (GLOB KERNEL_HEADER_FILES
      "*.h" )

macro (add_to_kernel_sources)
    foreach (_src ${ARGN})
        list (APPEND KERNEL_SOURCE_FILES "${_src}")
    endforeach()
    set (KERNEL_SOURCE_FILES ${KERNEL_SOURCE_FILES} PARENT_SCOPE)
endmacro()

macro (add_to_kernel_headers)
    foreach (_src ${ARGN})
        list (APPEND KERNEL_HEADER_FILES "${_src}")
    endforeach()
    set (KERNEL_HEADER_FILES ${KERNEL_HEADER_FILES} PARENT_SCOPE)
endmacro()

########
# Add Subdirecotry Files
########
add_subdirectory(com/)
add_subdirectory(dev/)

include_directories( ${TARGET_SYSTEM_DIR}
                     ${CMAKE_CURRENT_SOURCE_DIR}
                     ${CMAKE_CURRENT_SOURCE_DIR}/../CcCore
                     ${CMAKE_CURRENT_SOURCE_DIR}/../CcGui
                     ${CMAKE_CURRENT_SOURCE_DIR}/../CcTarget )

add_library( CcKernel STATIC ${KERNEL_HEADER_FILES} ${KERNEL_SOURCE_FILES} )

link_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../lib )

set_property( TARGET CcKernel PROPERTY FOLDER "CcLib")

target_link_libraries ( CcKernel LINK_PUBLIC
                              CcTarget
                              CcCore )

target_include_directories( CcKernel PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_include_directories( CcKernel PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})