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
CMAKE_SOURCE_DIR = /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/avro-c-1.7.7

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/avro-c-1.7.7

# Include any dependencies generated for this target.
include tests/CMakeFiles/test_avro_1238.dir/depend.make

# Include the progress variables for this target.
include tests/CMakeFiles/test_avro_1238.dir/progress.make

# Include the compile flags for this target's objects.
include tests/CMakeFiles/test_avro_1238.dir/flags.make

tests/CMakeFiles/test_avro_1238.dir/test_avro_1238.o: tests/CMakeFiles/test_avro_1238.dir/flags.make
tests/CMakeFiles/test_avro_1238.dir/test_avro_1238.o: tests/test_avro_1238.c
	$(CMAKE_COMMAND) -E cmake_progress_report /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/avro-c-1.7.7/CMakeFiles $(CMAKE_PROGRESS_1)
	@echo "Building C object tests/CMakeFiles/test_avro_1238.dir/test_avro_1238.o"
	cd /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/avro-c-1.7.7/tests && /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/host/usr/bin/mips-unknown-linux-uclibc-gcc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/test_avro_1238.dir/test_avro_1238.o   -c /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/avro-c-1.7.7/tests/test_avro_1238.c

tests/CMakeFiles/test_avro_1238.dir/test_avro_1238.i: cmake_force
	@echo "Preprocessing C source to CMakeFiles/test_avro_1238.dir/test_avro_1238.i"
	cd /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/avro-c-1.7.7/tests && /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/host/usr/bin/mips-unknown-linux-uclibc-gcc  $(C_DEFINES) $(C_FLAGS) -E /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/avro-c-1.7.7/tests/test_avro_1238.c > CMakeFiles/test_avro_1238.dir/test_avro_1238.i

tests/CMakeFiles/test_avro_1238.dir/test_avro_1238.s: cmake_force
	@echo "Compiling C source to assembly CMakeFiles/test_avro_1238.dir/test_avro_1238.s"
	cd /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/avro-c-1.7.7/tests && /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/host/usr/bin/mips-unknown-linux-uclibc-gcc  $(C_DEFINES) $(C_FLAGS) -S /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/avro-c-1.7.7/tests/test_avro_1238.c -o CMakeFiles/test_avro_1238.dir/test_avro_1238.s

tests/CMakeFiles/test_avro_1238.dir/test_avro_1238.o.requires:
.PHONY : tests/CMakeFiles/test_avro_1238.dir/test_avro_1238.o.requires

tests/CMakeFiles/test_avro_1238.dir/test_avro_1238.o.provides: tests/CMakeFiles/test_avro_1238.dir/test_avro_1238.o.requires
	$(MAKE) -f tests/CMakeFiles/test_avro_1238.dir/build.make tests/CMakeFiles/test_avro_1238.dir/test_avro_1238.o.provides.build
.PHONY : tests/CMakeFiles/test_avro_1238.dir/test_avro_1238.o.provides

tests/CMakeFiles/test_avro_1238.dir/test_avro_1238.o.provides.build: tests/CMakeFiles/test_avro_1238.dir/test_avro_1238.o

# Object files for target test_avro_1238
test_avro_1238_OBJECTS = \
"CMakeFiles/test_avro_1238.dir/test_avro_1238.o"

# External object files for target test_avro_1238
test_avro_1238_EXTERNAL_OBJECTS =

tests/test_avro_1238: tests/CMakeFiles/test_avro_1238.dir/test_avro_1238.o
tests/test_avro_1238: tests/CMakeFiles/test_avro_1238.dir/build.make
tests/test_avro_1238: src/libavro.a
tests/test_avro_1238: tests/CMakeFiles/test_avro_1238.dir/link.txt
	@echo "Linking C executable test_avro_1238"
	cd /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/avro-c-1.7.7/tests && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/test_avro_1238.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
tests/CMakeFiles/test_avro_1238.dir/build: tests/test_avro_1238
.PHONY : tests/CMakeFiles/test_avro_1238.dir/build

tests/CMakeFiles/test_avro_1238.dir/requires: tests/CMakeFiles/test_avro_1238.dir/test_avro_1238.o.requires
.PHONY : tests/CMakeFiles/test_avro_1238.dir/requires

tests/CMakeFiles/test_avro_1238.dir/clean:
	cd /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/avro-c-1.7.7/tests && $(CMAKE_COMMAND) -P CMakeFiles/test_avro_1238.dir/cmake_clean.cmake
.PHONY : tests/CMakeFiles/test_avro_1238.dir/clean

tests/CMakeFiles/test_avro_1238.dir/depend:
	cd /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/avro-c-1.7.7 && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/avro-c-1.7.7 /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/avro-c-1.7.7/tests /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/avro-c-1.7.7 /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/avro-c-1.7.7/tests /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/avro-c-1.7.7/tests/CMakeFiles/test_avro_1238.dir/DependInfo.cmake
.PHONY : tests/CMakeFiles/test_avro_1238.dir/depend

