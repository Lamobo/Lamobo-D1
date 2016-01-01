########
# Add Source Files
########
file (GLOB SOURCE_FILES
      "src/*.cpp" )


########
# Add Source Files
########
file (GLOB HEADER_FILES
      "*.h" )

add_to_kernel_sources(${SOURCE_FILES})
add_to_kernel_headers(${HEADER_FILES})
