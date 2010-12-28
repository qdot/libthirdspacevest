######################################################################################
# Build function for thirdspacevest_790IT_test
######################################################################################

SET(LIBTHIRDSPACEVEST_EXAMPLE_LIBS ${libthirdspacevest_LIBRARY} ${LIBTHIRDSPACEVEST_REQUIRED_LIBS})

SET(EXAMPLES thirdspacevest_test)

FOREACH(EX ${EXAMPLES})
  SET(SRCS ${EX}.c)
  
  BUILDSYS_BUILD_EXE(
    NAME ${EX}
    SOURCES "${SRCS}" 
    CXX_FLAGS FALSE
    LINK_LIBS "${LIBTHIRDSPACEVEST_EXAMPLE_LIBS}"
    LINK_FLAGS FALSE 
    DEPENDS ${thirdspacevest_DEPEND}
    SHOULD_INSTALL TRUE
    )
ENDFOREACH()
