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
include src/test/CMakeFiles/stow.dir/depend.make

# Include the progress variables for this target.
include src/test/CMakeFiles/stow.dir/progress.make

# Include the compile flags for this target's objects.
include src/test/CMakeFiles/stow.dir/flags.make

src/test/CMakeFiles/stow.dir/stow.c.o: src/test/CMakeFiles/stow.dir/flags.make
src/test/CMakeFiles/stow.dir/stow.c.o: src/test/stow.c
	$(CMAKE_COMMAND) -E cmake_progress_report /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/CMakeFiles $(CMAKE_PROGRESS_1)
	@echo "Building C object src/test/CMakeFiles/stow.dir/stow.c.o"
	cd /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/src/test && /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/host/usr/bin/mips-unknown-linux-uclibc-gcc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/stow.dir/stow.c.o   -c /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/src/test/stow.c

src/test/CMakeFiles/stow.dir/stow.c.i: cmake_force
	@echo "Preprocessing C source to CMakeFiles/stow.dir/stow.c.i"
	cd /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/src/test && /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/host/usr/bin/mips-unknown-linux-uclibc-gcc  $(C_DEFINES) $(C_FLAGS) -E /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/src/test/stow.c > CMakeFiles/stow.dir/stow.c.i

src/test/CMakeFiles/stow.dir/stow.c.s: cmake_force
	@echo "Compiling C source to assembly CMakeFiles/stow.dir/stow.c.s"
	cd /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/src/test && /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/host/usr/bin/mips-unknown-linux-uclibc-gcc  $(C_DEFINES) $(C_FLAGS) -S /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/src/test/stow.c -o CMakeFiles/stow.dir/stow.c.s

src/test/CMakeFiles/stow.dir/stow.c.o.requires:
.PHONY : src/test/CMakeFiles/stow.dir/stow.c.o.requires

src/test/CMakeFiles/stow.dir/stow.c.o.provides: src/test/CMakeFiles/stow.dir/stow.c.o.requires
	$(MAKE) -f src/test/CMakeFiles/stow.dir/build.make src/test/CMakeFiles/stow.dir/stow.c.o.provides.build
.PHONY : src/test/CMakeFiles/stow.dir/stow.c.o.provides

src/test/CMakeFiles/stow.dir/stow.c.o.provides.build: src/test/CMakeFiles/stow.dir/stow.c.o

# Object files for target stow
stow_OBJECTS = \
"CMakeFiles/stow.dir/stow.c.o"

# External object files for target stow
stow_EXTERNAL_OBJECTS =

src/test/stow: src/test/CMakeFiles/stow.dir/stow.c.o
src/test/stow: src/test/CMakeFiles/stow.dir/build.make
src/test/stow: lib/libical.so.2.0.0
src/test/stow: lib/libicalss.so.2.0.0
src/test/stow: lib/libicalvcal.so.2.0.0
src/test/stow: lib/libical_cxx.so.2.0.0
src/test/stow: lib/libicalss_cxx.so.2.0.0
src/test/stow: lib/libicalss.so.2.0.0
src/test/stow: lib/libical_cxx.so.2.0.0
src/test/stow: lib/libical.so.2.0.0
src/test/stow: src/test/CMakeFiles/stow.dir/link.txt
	@echo "Linking C executable stow"
	cd /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/src/test && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/stow.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/test/CMakeFiles/stow.dir/build: src/test/stow
.PHONY : src/test/CMakeFiles/stow.dir/build

src/test/CMakeFiles/stow.dir/requires: src/test/CMakeFiles/stow.dir/stow.c.o.requires
.PHONY : src/test/CMakeFiles/stow.dir/requires

src/test/CMakeFiles/stow.dir/clean:
	cd /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/src/test && $(CMAKE_COMMAND) -P CMakeFiles/stow.dir/cmake_clean.cmake
.PHONY : src/test/CMakeFiles/stow.dir/clean

src/test/CMakeFiles/stow.dir/depend:
	cd /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0 && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0 /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/src/test /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0 /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/src/test /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/src/test/CMakeFiles/stow.dir/DependInfo.cmake
.PHONY : src/test/CMakeFiles/stow.dir/depend

