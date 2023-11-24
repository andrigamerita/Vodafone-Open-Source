#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "ical" for configuration "Release"
set_property(TARGET ical APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(ical PROPERTIES
  IMPORTED_LINK_INTERFACE_LIBRARIES_RELEASE "-lpthread"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libical.so.2.0.0"
  IMPORTED_SONAME_RELEASE "libical.so.2"
  )

list(APPEND _IMPORT_CHECK_TARGETS ical )
list(APPEND _IMPORT_CHECK_FILES_FOR_ical "${_IMPORT_PREFIX}/lib/libical.so.2.0.0" )

# Import target "ical_cxx" for configuration "Release"
set_property(TARGET ical_cxx APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(ical_cxx PROPERTIES
  IMPORTED_LINK_INTERFACE_LIBRARIES_RELEASE "ical;-lpthread"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libical_cxx.so.2.0.0"
  IMPORTED_SONAME_RELEASE "libical_cxx.so.2"
  )

list(APPEND _IMPORT_CHECK_TARGETS ical_cxx )
list(APPEND _IMPORT_CHECK_FILES_FOR_ical_cxx "${_IMPORT_PREFIX}/lib/libical_cxx.so.2.0.0" )

# Import target "icalss" for configuration "Release"
set_property(TARGET icalss APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(icalss PROPERTIES
  IMPORTED_LINK_INTERFACE_LIBRARIES_RELEASE "ical"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libicalss.so.2.0.0"
  IMPORTED_SONAME_RELEASE "libicalss.so.2"
  )

list(APPEND _IMPORT_CHECK_TARGETS icalss )
list(APPEND _IMPORT_CHECK_FILES_FOR_icalss "${_IMPORT_PREFIX}/lib/libicalss.so.2.0.0" )

# Import target "icalss_cxx" for configuration "Release"
set_property(TARGET icalss_cxx APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(icalss_cxx PROPERTIES
  IMPORTED_LINK_INTERFACE_LIBRARIES_RELEASE "icalss;ical_cxx;-lpthread"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libicalss_cxx.so.2.0.0"
  IMPORTED_SONAME_RELEASE "libicalss_cxx.so.2"
  )

list(APPEND _IMPORT_CHECK_TARGETS icalss_cxx )
list(APPEND _IMPORT_CHECK_FILES_FOR_icalss_cxx "${_IMPORT_PREFIX}/lib/libicalss_cxx.so.2.0.0" )

# Import target "icalvcal" for configuration "Release"
set_property(TARGET icalvcal APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(icalvcal PROPERTIES
  IMPORTED_LINK_INTERFACE_LIBRARIES_RELEASE "ical"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libicalvcal.so.2.0.0"
  IMPORTED_SONAME_RELEASE "libicalvcal.so.2"
  )

list(APPEND _IMPORT_CHECK_TARGETS icalvcal )
list(APPEND _IMPORT_CHECK_FILES_FOR_icalvcal "${_IMPORT_PREFIX}/lib/libicalvcal.so.2.0.0" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
