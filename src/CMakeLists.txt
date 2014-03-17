# src/

# include settings
#INCLUDE_DIRECTORIES(${${CMAKE_PROJECT_NAME}_SOURCE_DIR}/lib)
#INCLUDE_DIRECTORIES(${${CMAKE_PROJECT_NAME}_SOURCE_DIR}/lib/include)
INCLUDE_DIRECTORIES(${${CMAKE_PROJECT_NAME}_SOURCE_DIR}/include)

# lib settings
LINK_DIRECTORIES(${${CMAKE_PROJECT_NAME}_BINARY_DIR}/lib)


# all related files
AUX_SOURCE_DIRECTORY(. DIR_TARGET)
ADD_LIBRARY (relfiles ${DIR_TARGET})

################################################################################

ADD_EXECUTABLE(eiparser eiparser.cc)

#  executable dependencies
ADD_DEPENDENCIES(eiparser iniparser)
ADD_DEPENDENCIES(eiparser relfiles)
ADD_DEPENDENCIES(eiparser gumbo)

#  executable target links
TARGET_LINK_LIBRARIES(eiparser relfiles)
TARGET_LINK_LIBRARIES(eiparser iniparser)
TARGET_LINK_LIBRARIES(eiparser pthread)
TARGET_LINK_LIBRARIES(eiparser gumbo)

#  executable properties
SET_TARGET_PROPERTIES(eiparser PROPERTIES OUTPUT_NAME "eiparser")








