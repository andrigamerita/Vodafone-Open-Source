# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.0

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list

# Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/host/usr/bin/cmake

# The command to remove a file.
RM = /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/host/usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0

# Include any dependencies generated for this target.
include src/libicalss/CMakeFiles/icalss_cxx.dir/depend.make

# Include the progress variables for this target.
include src/libicalss/CMakeFiles/icalss_cxx.dir/progress.make

# Include the compile flags for this target's objects.
include src/libicalss/CMakeFiles/icalss_cxx.dir/flags.make

src/libicalss/CMakeFiles/icalss_cxx.dir/icalspanlist_cxx.cpp.o: src/libicalss/CMakeFiles/icalss_cxx.dir/flags.make
src/libicalss/CMakeFiles/icalss_cxx.dir/icalspanlist_cxx.cpp.o: src/libicalss/icalspanlist_cxx.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/CMakeFiles $(CMAKE_PROGRESS_1)
	@echo "Building CXX object src/libicalss/CMakeFiles/icalss_cxx.dir/icalspanlist_cxx.cpp.o"
	cd /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/src/libicalss && /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/host/usr/bin/mips-unknown-linux-uclibc-g++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/icalss_cxx.dir/icalspanlist_cxx.cpp.o -c /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/src/libicalss/icalspanlist_cxx.cpp

src/libicalss/CMakeFiles/icalss_cxx.dir/icalspanlist_cxx.cpp.i: cmake_force
	@echo "Preprocessing CXX source to CMakeFiles/icalss_cxx.dir/icalspanlist_cxx.cpp.i"
	cd /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/src/libicalss && /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/host/usr/bin/mips-unknown-linux-uclibc-g++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/src/libicalss/icalspanlist_cxx.cpp > CMakeFiles/icalss_cxx.dir/icalspanlist_cxx.cpp.i

src/libicalss/CMakeFiles/icalss_cxx.dir/icalspanlist_cxx.cpp.s: cmake_force
	@echo "Compiling CXX source to assembly CMakeFiles/icalss_cxx.dir/icalspanlist_cxx.cpp.s"
	cd /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/src/libicalss && /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/host/usr/bin/mips-unknown-linux-uclibc-g++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/src/libicalss/icalspanlist_cxx.cpp -o CMakeFiles/icalss_cxx.dir/icalspanlist_cxx.cpp.s

src/libicalss/CMakeFiles/icalss_cxx.dir/icalspanlist_cxx.cpp.o.requires:
.PHONY : src/libicalss/CMakeFiles/icalss_cxx.dir/icalspanlist_cxx.cpp.o.requires

src/libicalss/CMakeFiles/icalss_cxx.dir/icalspanlist_cxx.cpp.o.provides: src/libicalss/CMakeFiles/icalss_cxx.dir/icalspanlist_cxx.cpp.o.requires
	$(MAKE) -f src/libicalss/CMakeFiles/icalss_cxx.dir/build.make src/libicalss/CMakeFiles/icalss_cxx.dir/icalspanlist_cxx.cpp.o.provides.build
.PHONY : src/libicalss/CMakeFiles/icalss_cxx.dir/icalspanlist_cxx.cpp.o.provides

src/libicalss/CMakeFiles/icalss_cxx.dir/icalspanlist_cxx.cpp.o.provides.build: src/libicalss/CMakeFiles/icalss_cxx.dir/icalspanlist_cxx.cpp.o

# Object files for target icalss_cxx
icalss_cxx_OBJECTS = \
"CMakeFiles/icalss_cxx.dir/icalspanlist_cxx.cpp.o"

# External object files for target icalss_cxx
icalss_cxx_EXTERNAL_OBJECTS =

lib/libicalss_cxx.so.2.0.0: src/libicalss/CMakeFiles/icalss_cxx.dir/icalspanlist_cxx.cpp.o
lib/libicalss_cxx.so.2.0.0: src/libicalss/CMakeFiles/icalss_cxx.dir/build.make
lib/libicalss_cxx.so.2.0.0: lib/libicalss.so.2.0.0
lib/libicalss_cxx.so.2.0.0: lib/libical_cxx.so.2.0.0
lib/libicalss_cxx.so.2.0.0: lib/libical.so.2.0.0
lib/libicalss_cxx.so.2.0.0: src/libicalss/CMakeFiles/icalss_cxx.dir/link.txt
	@echo "Linking CXX shared library ../../lib/libicalss_cxx.so"
	cd /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/src/libicalss && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/icalss_cxx.dir/link.txt --verbose=$(VERBOSE)
	cd /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/src/libicalss && $(CMAKE_COMMAND) -E cmake_symlink_library ../../lib/libicalss_cxx.so.2.0.0 ../../lib/libicalss_cxx.so.2 ../../lib/libicalss_cxx.so

lib/libicalss_cxx.so.2: lib/libicalss_cxx.so.2.0.0

lib/libicalss_cxx.so: lib/libicalss_cxx.so.2.0.0

# Rule to build all files generated by this target.
src/libicalss/CMakeFiles/icalss_cxx.dir/build: lib/libicalss_cxx.so
.PHONY : src/libicalss/CMakeFiles/icalss_cxx.dir/build

src/libicalss/CMakeFiles/icalss_cxx.dir/requires: src/libicalss/CMakeFiles/icalss_cxx.dir/icalspanlist_cxx.cpp.o.requires
.PHONY : src/libicalss/CMakeFiles/icalss_cxx.dir/requires

src/libicalss/CMakeFiles/icalss_cxx.dir/clean:
	cd /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/src/libicalss && $(CMAKE_COMMAND) -P CMakeFiles/icalss_cxx.dir/cmake_clean.cmake
.PHONY : src/libicalss/CMakeFiles/icalss_cxx.dir/clean

src/libicalss/CMakeFiles/icalss_cxx.dir/depend:
	cd /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0 && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0 /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/src/libicalss /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0 /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/src/libicalss /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/src/libicalss/CMakeFiles/icalss_cxx.dir/DependInfo.cmake
.PHONY : src/libicalss/CMakeFiles/icalss_cxx.dir/depend

