# Install script for directory: /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/avro-c-1.7.7/src

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
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE FILE FILES "/home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/avro-c-1.7.7/src/avro.h")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE DIRECTORY FILES "/home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/avro-c-1.7.7/src/avro" FILES_MATCHING REGEX "/[^/]*\\.h$")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/avro-c-1.7.7/src/libavro.a")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libavro.so.22.0.0"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libavro.so"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      file(RPATH_CHECK
           FILE "${file}"
           RPATH "")
    endif()
  endforeach()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES
    "/home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/avro-c-1.7.7/src/libavro.so.22.0.0"
    "/home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/avro-c-1.7.7/src/libavro.so"
    )
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libavro.so.22.0.0"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libavro.so"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      if(CMAKE_INSTALL_DO_STRIP)
        execute_process(COMMAND "/home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/host/usr/bin/mips-unknown-linux-uclibc-strip" "${file}")
      endif()
    endif()
  endforeach()
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/avro-c-1.7.7/src/avro-c.pc")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/avrocat" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/avrocat")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/avrocat"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "/home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/avro-c-1.7.7/src/avrocat")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/avrocat" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/avrocat")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/host/usr/bin/mips-unknown-linux-uclibc-strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/avrocat")
    endif()
  endif()
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/avroappend" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/avroappend")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/avroappend"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "/home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/avro-c-1.7.7/src/avroappend")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/avroappend" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/avroappend")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/host/usr/bin/mips-unknown-linux-uclibc-strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/avroappend")
    endif()
  endif()
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/avropipe" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/avropipe")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/avropipe"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "/home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/avro-c-1.7.7/src/avropipe")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/avropipe" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/avropipe")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/host/usr/bin/mips-unknown-linux-uclibc-strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/avropipe")
    endif()
  endif()
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/avromod" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/avromod")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/avromod"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "/home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/avro-c-1.7.7/src/avromod")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/avromod" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/avromod")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/host/usr/bin/mips-unknown-linux-uclibc-strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/avromod")
    endif()
  endif()
endif()

