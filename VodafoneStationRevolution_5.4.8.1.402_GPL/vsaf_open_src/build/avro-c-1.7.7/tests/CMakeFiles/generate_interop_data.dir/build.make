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
include tests/CMakeFiles/generate_interop_data.dir/depend.make

# Include the progress variables for this target.
include tests/CMakeFiles/generate_interop_data.dir/progress.make

# Include the compile flags for this target's objects.
include tests/CMakeFiles/generate_interop_data.dir/flags.make

tests/CMakeFiles/generate_interop_data.dir/generate_interop_data.o: tests/CMakeFiles/generate_interop_data.dir/flags.make
tests/CMakeFiles/generate_interop_data.dir/generate_interop_data.o: tests/generate_interop_data.c
	$(CMAKE_COMMAND) -E cmake_progress_report /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/avro-c-1.7.7/CMakeFiles $(CMAKE_PROGRESS_1)
	@echo "Building C object tests/CMakeFiles/generate_interop_data.dir/generate_interop_data.o"
	cd /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/avro-c-1.7.7/tests && /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/host/usr/bin/mips-unknown-linux-uclibc-gcc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/generate_interop_data.dir/generate_interop_data.o   -c /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/avro-c-1.7.7/tests/generate_interop_data.c

tests/CMakeFiles/generate_interop_data.dir/generate_interop_data.i: cmake_force
	@echo "Preprocessing C source to CMakeFiles/generate_interop_data.dir/generate_interop_data.i"
	cd /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/avro-c-1.7.7/tests && /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/host/usr/bin/mips-unknown-linux-uclibc-gcc  $(C_DEFINES) $(C_FLAGS) -E /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/avro-c-1.7.7/tests/generate_interop_data.c > CMakeFiles/generate_interop_data.dir/generate_interop_data.i

tests/CMakeFiles/generate_interop_data.dir/generate_interop_data.s: cmake_force
	@echo "Compiling C source to assembly CMakeFiles/generate_interop_data.dir/generate_interop_data.s"
	cd /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/avro-c-1.7.7/tests && /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/host/usr/bin/mips-unknown-linux-uclibc-gcc  $(C_DEFINES) $(C_FLAGS) -S /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/avro-c-1.7.7/tests/generate_interop_data.c -o CMakeFiles/generate_interop_data.dir/generate_interop_data.s

tests/CMakeFiles/generate_interop_data.dir/generate_interop_data.o.requires:
.PHONY : tests/CMakeFiles/generate_interop_data.dir/generate_interop_data.o.requires

tests/CMakeFiles/generate_interop_data.dir/generate_interop_data.o.provides: tests/CMakeFiles/generate_interop_data.dir/generate_interop_data.o.requires
	$(MAKE) -f tests/CMakeFiles/generate_interop_data.dir/build.make tests/CMakeFiles/generate_interop_data.dir/generate_interop_data.o.provides.build
.PHONY : tests/CMakeFiles/generate_interop_data.dir/generate_interop_data.o.provides

tests/CMakeFiles/generate_interop_data.dir/generate_interop_data.o.provides.build: tests/CMakeFiles/generate_interop_data.dir/generate_interop_data.o

# Object files for target generate_interop_data
generate_interop_data_OBJECTS = \
"CMakeFiles/generate_interop_data.dir/generate_interop_data.o"

# External object files for target generate_interop_data
generate_interop_data_EXTERNAL_OBJECTS =

tests/generate_interop_data: tests/CMakeFiles/generate_interop_data.dir/generate_interop_data.o
tests/generate_interop_data: tests/CMakeFiles/generate_interop_data.dir/build.make
tests/generate_interop_data: src/libavro.a
tests/generate_interop_data: tests/CMakeFiles/generate_interop_data.dir/link.txt
	@echo "Linking C executable generate_interop_data"
	cd /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/avro-c-1.7.7/tests && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/generate_interop_data.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
tests/CMakeFiles/generate_interop_data.dir/build: tests/generate_interop_data
.PHONY : tests/CMakeFiles/generate_interop_data.dir/build

tests/CMakeFiles/generate_interop_data.dir/requires: tests/CMakeFiles/generate_interop_data.dir/generate_interop_data.o.requires
.PHONY : tests/CMakeFiles/generate_interop_data.dir/requires

tests/CMakeFiles/generate_interop_data.dir/clean:
	cd /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/avro-c-1.7.7/tests && $(CMAKE_COMMAND) -P CMakeFiles/generate_interop_data.dir/cmake_clean.cmake
.PHONY : tests/CMakeFiles/generate_interop_data.dir/clean

tests/CMakeFiles/generate_interop_data.dir/depend:
	cd /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/avro-c-1.7.7 && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/avro-c-1.7.7 /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/avro-c-1.7.7/tests /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/avro-c-1.7.7 /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/avro-c-1.7.7/tests /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/avro-c-1.7.7/tests/CMakeFiles/generate_interop_data.dir/DependInfo.cmake
.PHONY : tests/CMakeFiles/generate_interop_data.dir/depend

