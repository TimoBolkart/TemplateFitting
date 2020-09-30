SET(Files
	FileLoader.cpp
	FileWriter.cpp
	KDTree3.cpp
	MathHelper.cpp
	TemplateFitting.cpp
	TemplateFittingCostFunction.cpp
	Main.cpp
)

INCLUDE_DIRECTORIES(${ANN_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${CLAPACK_INCLUDE_DIR}) 
INCLUDE_DIRECTORIES(${ITK_INCLUDES}) 
   
ADD_DEFINITIONS(-D_USE_MATH_DEFINES)

ADD_EXECUTABLE(TemplateFitting ${Files})
TARGET_LINK_LIBRARIES(TemplateFitting ${ITK_LIBRARIES} ${ANN_LIBRARIES} ${CLAPACK_LIBRARIES})   