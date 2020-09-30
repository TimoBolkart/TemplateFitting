#
# Just finds the relevant include directory and libraries used to develop
# with ann

FIND_PATH(ANN_INCLUDE_DIR ANN/ANN.h
  /usr/local/include
  /usr/include
  /opt/local/include
 )

FIND_LIBRARY(ANN_LIBRARIES
  NAMES
   ANN
  PATHS
   /usr/local/lib
   /usr/lib
   /opt/local/lib
)

SET(ANN_FOUND 0)
IF(ANN_INCLUDE_DIR)
  IF(ANN_LIBRARIES)
    SET(ANN_FOUND 1)
  ENDIF(ANN_LIBRARIES)
ENDIF(ANN_INCLUDE_DIR)

IF(ANN_FOUND)
  INCLUDE_DIRECTORIES(${ANN_INCLUDE_DIR})
ELSE(ANN_FOUND)
  MESSAGE("PROBLEM: ANN not found.")
ENDIF(ANN_FOUND)

MARK_AS_ADVANCED(ANN_INCLUDE_DIR
  ANN_LIBRARIES)
