#----------------------------------------------------------------
# Generated CMake target import file for configuration "RelWithDebInfo".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "LDtkLoader::LDtkLoader" for configuration "RelWithDebInfo"
set_property(TARGET LDtkLoader::LDtkLoader APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(LDtkLoader::LDtkLoader PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELWITHDEBINFO "CXX"
  IMPORTED_LOCATION_RELWITHDEBINFO "${_IMPORT_PREFIX}/lib/LDtkLoader.lib"
  )

list(APPEND _cmake_import_check_targets LDtkLoader::LDtkLoader )
list(APPEND _cmake_import_check_files_for_LDtkLoader::LDtkLoader "${_IMPORT_PREFIX}/lib/LDtkLoader.lib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
