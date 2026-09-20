#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "alibabacloud_credentials::alibabacloud_credentials" for configuration "Release"
set_property(TARGET alibabacloud_credentials::alibabacloud_credentials APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(alibabacloud_credentials::alibabacloud_credentials PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libalibabacloud_credentials.so.0.1.1"
  IMPORTED_SONAME_RELEASE "libalibabacloud_credentials.so.0"
  )

list(APPEND _cmake_import_check_targets alibabacloud_credentials::alibabacloud_credentials )
list(APPEND _cmake_import_check_files_for_alibabacloud_credentials::alibabacloud_credentials "${_IMPORT_PREFIX}/lib/libalibabacloud_credentials.so.0.1.1" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
