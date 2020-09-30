#
# Just finds the relevant include directory and libraries used to develop
# with itk

#Necessary include directories
#	$(ITK_DIR)/Modules/ThirdParty/VNL/src/vxl/core
#	$(ITK_DIR)/Modules/ThirdParty/VNL/src/vxl/vcl
#	$(ITK_DIR)/VS2010/Modules/ThirdParty/VNL/src/vxl/vcl		
#	$(ITK_DIR)/VS2010/Modules/ThirdParty/VNL/src/vxl/core	

#Used libraries
#	itkvnl_algo-4.1.lib						$(ITK_DIR)\VS2010\lib\Release
#	itkvnl-4.1.lib							$(ITK_DIR)\VS2010\lib\Release
#	itkv3p_netlib-4.1.lib					$(ITK_DIR)\VS2010\lib\Release

FIND_PATH(ITK_VNL_CORE_INCLUDE_DIR vnl/vnl_vector.h
  /usr/local/include
  /usr/include
  /opt/local/include
)

FIND_PATH(ITK_BIN_VNL_CORE_INCLUDE_DIR vxl_config.h
  /usr/local/include
  /usr/include
  /opt/local/include
)

FIND_PATH(ITK_VCL_INCLUDE_DIR vcl_iosfwd.h
  /usr/local/include
  /usr/include
  /opt/local/include
)

FIND_PATH(ITK_BIN_VCL_INCLUDE_DIR vcl_config_manual.h
  /usr/local/include
  /usr/include
  /opt/local/include
)

FIND_LIBRARY(ITK_KVNL_LIB
  NAMES
	itkvnl-4.2
  PATHS
   /usr/local/lib
   /usr/lib
   /opt/local/lib
)   

FIND_LIBRARY(ITK_KVNL_ALGO_LIB
  NAMES
	itkvnl_algo-4.2
  PATHS
   /usr/local/lib
   /usr/lib
   /opt/local/lib
)

FIND_LIBRARY(ITK_KV3P_NETLIB_LIB
  NAMES
	itkv3p_netlib-4.2
  PATHS
   /usr/local/lib
   /usr/lib
   /opt/local/lib
)

SET(ITK_FOUND 0)
IF(ITK_VNL_CORE_INCLUDE_DIR AND ITK_BIN_VNL_CORE_INCLUDE_DIR AND ITK_VCL_INCLUDE_DIR AND ITK_BIN_VCL_INCLUDE_DIR)
  IF(ITK_KVNL_LIB AND ITK_KVNL_ALGO_LIB AND ITK_KV3P_NETLIB_LIB)
    SET(ITK_FOUND 1)
  ENDIF(ITK_KVNL_LIB AND ITK_KVNL_ALGO_LIB AND ITK_KV3P_NETLIB_LIB)
ENDIF(ITK_VNL_CORE_INCLUDE_DIR AND ITK_BIN_VNL_CORE_INCLUDE_DIR AND ITK_VCL_INCLUDE_DIR AND ITK_BIN_VCL_INCLUDE_DIR)

SET(ITK_INCLUDES ${ITK_VNL_CORE_INCLUDE_DIR} ${ITK_BIN_VNL_CORE_INCLUDE_DIR} ${ITK_VCL_INCLUDE_DIR} ${ITK_BIN_VCL_INCLUDE_DIR})
SET(ITK_LIBRARIES ${ITK_KVNL_LIB} ${ITK_KVNL_ALGO_LIB} ${ITK_KV3P_NETLIB_LIB} )

IF(ITK_FOUND)
  INCLUDE_DIRECTORIES(${ITK_INCLUDES})
ELSE(ITK_FOUND)
  MESSAGE("PROBLEM: ITK not found.")
ENDIF(ITK_FOUND)
