# Install script for directory: /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/src/libicalss

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
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libicalss.so.2.0.0"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libicalss.so.2"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libicalss.so"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      file(RPATH_CHECK
           FILE "${file}"
           RPATH "")
    endif()
  endforeach()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES
    "/home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/lib/libicalss.so.2.0.0"
    "/home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/lib/libicalss.so.2"
    "/home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/lib/libicalss.so"
    )
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libicalss.so.2.0.0"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libicalss.so.2"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libicalss.so"
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
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libicalss_cxx.so.2.0.0"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libicalss_cxx.so.2"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libicalss_cxx.so"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      file(RPATH_CHECK
           FILE "${file}"
           RPATH "")
    endif()
  endforeach()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES
    "/home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/lib/libicalss_cxx.so.2.0.0"
    "/home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/lib/libicalss_cxx.so.2"
    "/home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/lib/libicalss_cxx.so"
    )
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libicalss_cxx.so.2.0.0"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libicalss_cxx.so.2"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libicalss_cxx.so"
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
    "/home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/src/libicalss/icalss.h"
    "/home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/src/libicalss/icalcalendar.h"
    "/home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/src/libicalss/icalclassify.h"
    "/home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/src/libicalss/icalcluster.h"
    "/home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/src/libicalss/icaldirset.h"
    "/home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/src/libicalss/icaldirsetimpl.h"
    "/home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/src/libicalss/icalfileset.h"
    "/home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/src/libicalss/icalfilesetimpl.h"
    "/home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/src/libicalss/icalgauge.h"
    "/home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/src/libicalss/icalgaugeimpl.h"
    "/home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/src/libicalss/icalmessage.h"
    "/home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/src/libicalss/icalset.h"
    "/home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/src/libicalss/icalspanlist.h"
    "/home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/src/libicalss/icalssyacc.h"
    "/home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/src/libicalss/libical_icalss_export.h"
    )
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/libical" TYPE FILE FILES "/home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/src/libicalss/icalspanlist_cxx.h")
endif()

