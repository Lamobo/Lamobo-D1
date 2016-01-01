###################
# Depending on Target, just the correct System Library will be loaded:
###################
if(DEFINED TARGET_SYSTEM)
  message(${TARGET_SYSTEM})
  set(TARGET_SYSTEM_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Target/${TARGET_SYSTEM}" )
else ()
  set(TARGET_SYSTEM_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Target/${CMAKE_SYSTEM_NAME}" )
endif()

###################
# Include Target Settings:
###################
message( STATUS "Target System: " ${TARGET_SYSTEM_DIR} )
add_subdirectory( ${TARGET_SYSTEM_DIR} )
set(CC_TARGET_SYSTEM_DIR ${TARGET_SYSTEM_DIR} PARENT_SCOPE)

###################
# Add all CcLib Objects to build-List:
###################
add_subdirectory(CcCore/)
add_subdirectory(CcKernel/)
add_subdirectory(CcGui/)
add_subdirectory(CcUtil/)

###################
# Add thirdparty
###################
add_subdirectory(ThirdParty/)
