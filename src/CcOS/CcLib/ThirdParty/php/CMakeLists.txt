########
# Add Source Files
########
file (GLOB SOURCE_FILES
      "j*.c" )


########
# Add Header Files
########
file (GLOB HEADER_FILES
      "j*.h" )
      
add_library( jpeg STATIC ${HEADER_FILES} ${SOURCE_FILES} )
set_property( TARGET jpeg PROPERTY FOLDER "ThirdParty")
