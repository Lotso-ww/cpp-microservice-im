#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "alibabacloud_open_api_v2::alibabacloud_open_api_v2" for configuration "Release"
set_property(TARGET alibabacloud_open_api_v2::alibabacloud_open_api_v2 APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(alibabacloud_open_api_v2::alibabacloud_open_api_v2 PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libalibabacloud_open_api_v2.so.1.0.0"
  IMPORTED_SONAME_RELEASE "libalibabacloud_open_api_v2.so.1"
  )

list(APPEND _cmake_import_check_targets alibabacloud_open_api_v2::alibabacloud_open_api_v2 )
list(APPEND _cmake_import_check_files_for_alibabacloud_open_api_v2::alibabacloud_open_api_v2 "${_IMPORT_PREFIX}/lib/libalibabacloud_open_api_v2.so.1.0.0" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
