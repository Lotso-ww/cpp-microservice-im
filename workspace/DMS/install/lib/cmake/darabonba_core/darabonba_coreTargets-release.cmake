#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "darabonba_core::darabonba_core" for configuration "Release"
set_property(TARGET darabonba_core::darabonba_core APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(darabonba_core::darabonba_core PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libdarabonba_core.so.1.0.0"
  IMPORTED_SONAME_RELEASE "libdarabonba_core.so.1"
  )

list(APPEND _cmake_import_check_targets darabonba_core::darabonba_core )
list(APPEND _cmake_import_check_files_for_darabonba_core::darabonba_core "${_IMPORT_PREFIX}/lib/libdarabonba_core.so.1.0.0" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
