# Install script for directory: /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/src/libicalvcal

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
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libicalvcal.so.2.0.0"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libicalvcal.so.2"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libicalvcal.so"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      file(RPATH_CHECK
           FILE "${file}"
           RPATH "")
    endif()
  endforeach()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES
    "/home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/lib/libicalvcal.so.2.0.0"
    "/home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/lib/libicalvcal.so.2"
    "/home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/lib/libicalvcal.so"
    )
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libicalvcal.so.2.0.0"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libicalvcal.so.2"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libicalvcal.so"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      file(RPATH_REMOVE
           FILE "${file}")
      if(CMAKE_INSTALL_DO_STRIP)
        execute_process(COMMAND "/home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/host/usr/bin/mips-unknown-linux-uclibc-strip" "${file}")
      endif()
    endif()
  endforeach()
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/libical" TYPE FILE FILES
    "/home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/src/libicalvcal/icalvcal.h"
    "/home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/src/libicalvcal/vcaltmp.h"
    "/home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/src/libicalvcal/vcc.h"
    "/home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/src/libicalvcal/vobject.h"
    "/home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/src/libicalvcal/libical_vcal_export.h"
    )
endif()

