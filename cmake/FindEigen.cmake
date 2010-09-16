FIND_PATH(EIGEN_INCLUDE_DIR NAMES Eigen/Eigen)

# handle the QUIETLY and REQUIRED arguments and set LIBXML2_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Eigen DEFAULT_MSG EIGEN_INCLUDE_DIR)

MARK_AS_ADVANCED(EIGEN_INCLUDE_DIR)
