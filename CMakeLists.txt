cmake_minimum_required(VERSION 2.7)

INCLUDE(ConfigureANN.cmake)
INCLUDE(ConfigureCLAPACK.cmake)
INCLUDE(ConfigureITK.cmake)


find_package(OpenMP)
IF(OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
ELSE(OPENMP_FOUND)
  Message("OpenMP not supported.")  
ENDIF(OPENMP_FOUND)


IF(ANN_FOUND AND CLAPACK_FOUND AND ITK_FOUND)
  ADD_SUBDIRECTORY(TemplateFitting)
ELSE(ANN_FOUND AND CLAPACK_FOUND AND ITK_FOUND)
  Message("PROBLEM: One of the required libraries not found. TemplateFitting will not be compiled.")  
ENDIF(ANN_FOUND AND CLAPACK_FOUND AND ITK_FOUND) 