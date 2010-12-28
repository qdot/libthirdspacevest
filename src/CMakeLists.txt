######################################################################################
# Build function for main library
######################################################################################

SET(LIBRARY_SRCS 
  thirdspacevest.c
  )

IF(WIN32)
  LIST(APPEND LIBRARY_SRCS thirdspacevest_win32.c ${LIBTHIRDSPACEVEST_INCLUDE_FILES})
ELSEIF(UNIX)
  LIST(APPEND LIBRARY_SRCS thirdspacevest_libusb.c ${LIBTHIRDSPACEVEST_INCLUDE_FILES})
ENDIF(WIN32)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include)

BUILDSYS_BUILD_LIB(
  NAME thirdspacevest
  SOURCES "${LIBRARY_SRCS}" 
  CXX_FLAGS FALSE 
  LINK_LIBS "${LIBTHIRDSPACEVEST_REQUIRED_LIBS}"
  LINK_FLAGS FALSE 
  DEPENDS FALSE
  SHOULD_INSTALL TRUE
  VERSION ${LIBTHIRDSPACEVEST_VERSION}
  )
