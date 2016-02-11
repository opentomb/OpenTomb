# - Try to find GLM
# Once done, this will define
#
#  GLM_FOUND - system has GLM
#  GLM_INCLUDE_DIR - the GLM include directory

find_package(PkgConfig)
if (PKG_CONFIG_FOUND)
    pkg_check_modules(PKG_GLM QUIET glm)
endif()

find_path(GLM_INCLUDE_DIR
    glm/glm.hpp
    HINTS ${PKG_GLM_INCLUDE_DIRS} ${PKG_GLM_INCLUDEDIR} ${GLM_ROOT}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GLM REQUIRED_VARS GLM_INCLUDE_DIR)
mark_as_advanced(GLM_INCLUDE_DIR)
