# Install script for directory: /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "0")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/libical.pc")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/LibIcal" TYPE FILE FILES
    "/home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/LibIcalConfigVersion.cmake"
    "/home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/LibIcalConfig.cmake"
    )
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/LibIcal/LibIcalTargets.cmake")
    file(DIFFERENT EXPORT_FILE_CHANGED FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/LibIcal/LibIcalTargets.cmake"
         "/home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/CMakeFiles/Export/lib/cmake/LibIcal/LibIcalTargets.cmake")
    if(EXPORT_FILE_CHANGED)
      file(GLOB OLD_CONFIG_FILES "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/LibIcal/LibIcalTargets-*.cmake")
      if(OLD_CONFIG_FILES)
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/LibIcal/LibIcalTargets.cmake\" will be replaced.  Removing files [${OLD_CONFIG_FILES}].")
        file(REMOVE ${OLD_CONFIG_FILES})
      endif()
    endif()
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/LibIcal" TYPE FILE FILES "/home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/CMakeFiles/Export/lib/cmake/LibIcal/LibIcalTargets.cmake")
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/LibIcal" TYPE FILE FILES "/home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/CMakeFiles/Export/lib/cmake/LibIcal/LibIcalTargets-release.cmake")
  endif()
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/design-data/cmake_install.cmake")
  include("/home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/doc/cmake_install.cmake")
  include("/home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/scripts/cmake_install.cmake")
  include("/home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/test-data/cmake_install.cmake")
  include("/home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/src/cmake_install.cmake")
  include("/home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/examples/cmake_install.cmake")

endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

file(WRITE "/home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/${CMAKE_INSTALL_MANIFEST}" "")
foreach(file ${CMAKE_INSTALL_MANIFEST_FILES})
  file(APPEND "/home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/${CMAKE_INSTALL_MANIFEST}" "${file}\n")
endforeach()
