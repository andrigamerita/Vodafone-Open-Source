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
include src/libicalvcal/CMakeFiles/icalvcal.dir/depend.make

# Include the progress variables for this target.
include src/libicalvcal/CMakeFiles/icalvcal.dir/progress.make

# Include the compile flags for this target's objects.
include src/libicalvcal/CMakeFiles/icalvcal.dir/flags.make

src/libicalvcal/CMakeFiles/icalvcal.dir/icalvcal.c.o: src/libicalvcal/CMakeFiles/icalvcal.dir/flags.make
src/libicalvcal/CMakeFiles/icalvcal.dir/icalvcal.c.o: src/libicalvcal/icalvcal.c
	$(CMAKE_COMMAND) -E cmake_progress_report /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/CMakeFiles $(CMAKE_PROGRESS_1)
	@echo "Building C object src/libicalvcal/CMakeFiles/icalvcal.dir/icalvcal.c.o"
	cd /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/src/libicalvcal && /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/host/usr/bin/mips-unknown-linux-uclibc-gcc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/icalvcal.dir/icalvcal.c.o   -c /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/src/libicalvcal/icalvcal.c

src/libicalvcal/CMakeFiles/icalvcal.dir/icalvcal.c.i: cmake_force
	@echo "Preprocessing C source to CMakeFiles/icalvcal.dir/icalvcal.c.i"
	cd /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/src/libicalvcal && /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/host/usr/bin/mips-unknown-linux-uclibc-gcc  $(C_DEFINES) $(C_FLAGS) -E /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/src/libicalvcal/icalvcal.c > CMakeFiles/icalvcal.dir/icalvcal.c.i

src/libicalvcal/CMakeFiles/icalvcal.dir/icalvcal.c.s: cmake_force
	@echo "Compiling C source to assembly CMakeFiles/icalvcal.dir/icalvcal.c.s"
	cd /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/src/libicalvcal && /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/host/usr/bin/mips-unknown-linux-uclibc-gcc  $(C_DEFINES) $(C_FLAGS) -S /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/src/libicalvcal/icalvcal.c -o CMakeFiles/icalvcal.dir/icalvcal.c.s

src/libicalvcal/CMakeFiles/icalvcal.dir/icalvcal.c.o.requires:
.PHONY : src/libicalvcal/CMakeFiles/icalvcal.dir/icalvcal.c.o.requires

src/libicalvcal/CMakeFiles/icalvcal.dir/icalvcal.c.o.provides: src/libicalvcal/CMakeFiles/icalvcal.dir/icalvcal.c.o.requires
	$(MAKE) -f src/libicalvcal/CMakeFiles/icalvcal.dir/build.make src/libicalvcal/CMakeFiles/icalvcal.dir/icalvcal.c.o.provides.build
.PHONY : src/libicalvcal/CMakeFiles/icalvcal.dir/icalvcal.c.o.provides

src/libicalvcal/CMakeFiles/icalvcal.dir/icalvcal.c.o.provides.build: src/libicalvcal/CMakeFiles/icalvcal.dir/icalvcal.c.o

src/libicalvcal/CMakeFiles/icalvcal.dir/vobject.c.o: src/libicalvcal/CMakeFiles/icalvcal.dir/flags.make
src/libicalvcal/CMakeFiles/icalvcal.dir/vobject.c.o: src/libicalvcal/vobject.c
	$(CMAKE_COMMAND) -E cmake_progress_report /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/CMakeFiles $(CMAKE_PROGRESS_2)
	@echo "Building C object src/libicalvcal/CMakeFiles/icalvcal.dir/vobject.c.o"
	cd /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/src/libicalvcal && /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/host/usr/bin/mips-unknown-linux-uclibc-gcc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/icalvcal.dir/vobject.c.o   -c /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/src/libicalvcal/vobject.c

src/libicalvcal/CMakeFiles/icalvcal.dir/vobject.c.i: cmake_force
	@echo "Preprocessing C source to CMakeFiles/icalvcal.dir/vobject.c.i"
	cd /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/src/libicalvcal && /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/host/usr/bin/mips-unknown-linux-uclibc-gcc  $(C_DEFINES) $(C_FLAGS) -E /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/src/libicalvcal/vobject.c > CMakeFiles/icalvcal.dir/vobject.c.i

src/libicalvcal/CMakeFiles/icalvcal.dir/vobject.c.s: cmake_force
	@echo "Compiling C source to assembly CMakeFiles/icalvcal.dir/vobject.c.s"
	cd /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/src/libicalvcal && /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/host/usr/bin/mips-unknown-linux-uclibc-gcc  $(C_DEFINES) $(C_FLAGS) -S /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/src/libicalvcal/vobject.c -o CMakeFiles/icalvcal.dir/vobject.c.s

src/libicalvcal/CMakeFiles/icalvcal.dir/vobject.c.o.requires:
.PHONY : src/libicalvcal/CMakeFiles/icalvcal.dir/vobject.c.o.requires

src/libicalvcal/CMakeFiles/icalvcal.dir/vobject.c.o.provides: src/libicalvcal/CMakeFiles/icalvcal.dir/vobject.c.o.requires
	$(MAKE) -f src/libicalvcal/CMakeFiles/icalvcal.dir/build.make src/libicalvcal/CMakeFiles/icalvcal.dir/vobject.c.o.provides.build
.PHONY : src/libicalvcal/CMakeFiles/icalvcal.dir/vobject.c.o.provides

src/libicalvcal/CMakeFiles/icalvcal.dir/vobject.c.o.provides.build: src/libicalvcal/CMakeFiles/icalvcal.dir/vobject.c.o

src/libicalvcal/CMakeFiles/icalvcal.dir/vcaltmp.c.o: src/libicalvcal/CMakeFiles/icalvcal.dir/flags.make
src/libicalvcal/CMakeFiles/icalvcal.dir/vcaltmp.c.o: src/libicalvcal/vcaltmp.c
	$(CMAKE_COMMAND) -E cmake_progress_report /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/CMakeFiles $(CMAKE_PROGRESS_3)
	@echo "Building C object src/libicalvcal/CMakeFiles/icalvcal.dir/vcaltmp.c.o"
	cd /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/src/libicalvcal && /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/host/usr/bin/mips-unknown-linux-uclibc-gcc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/icalvcal.dir/vcaltmp.c.o   -c /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/src/libicalvcal/vcaltmp.c

src/libicalvcal/CMakeFiles/icalvcal.dir/vcaltmp.c.i: cmake_force
	@echo "Preprocessing C source to CMakeFiles/icalvcal.dir/vcaltmp.c.i"
	cd /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/src/libicalvcal && /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/host/usr/bin/mips-unknown-linux-uclibc-gcc  $(C_DEFINES) $(C_FLAGS) -E /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/src/libicalvcal/vcaltmp.c > CMakeFiles/icalvcal.dir/vcaltmp.c.i

src/libicalvcal/CMakeFiles/icalvcal.dir/vcaltmp.c.s: cmake_force
	@echo "Compiling C source to assembly CMakeFiles/icalvcal.dir/vcaltmp.c.s"
	cd /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/src/libicalvcal && /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/host/usr/bin/mips-unknown-linux-uclibc-gcc  $(C_DEFINES) $(C_FLAGS) -S /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/src/libicalvcal/vcaltmp.c -o CMakeFiles/icalvcal.dir/vcaltmp.c.s

src/libicalvcal/CMakeFiles/icalvcal.dir/vcaltmp.c.o.requires:
.PHONY : src/libicalvcal/CMakeFiles/icalvcal.dir/vcaltmp.c.o.requires

src/libicalvcal/CMakeFiles/icalvcal.dir/vcaltmp.c.o.provides: src/libicalvcal/CMakeFiles/icalvcal.dir/vcaltmp.c.o.requires
	$(MAKE) -f src/libicalvcal/CMakeFiles/icalvcal.dir/build.make src/libicalvcal/CMakeFiles/icalvcal.dir/vcaltmp.c.o.provides.build
.PHONY : src/libicalvcal/CMakeFiles/icalvcal.dir/vcaltmp.c.o.provides

src/libicalvcal/CMakeFiles/icalvcal.dir/vcaltmp.c.o.provides.build: src/libicalvcal/CMakeFiles/icalvcal.dir/vcaltmp.c.o

src/libicalvcal/CMakeFiles/icalvcal.dir/vcc.c.o: src/libicalvcal/CMakeFiles/icalvcal.dir/flags.make
src/libicalvcal/CMakeFiles/icalvcal.dir/vcc.c.o: src/libicalvcal/vcc.c
	$(CMAKE_COMMAND) -E cmake_progress_report /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/CMakeFiles $(CMAKE_PROGRESS_4)
	@echo "Building C object src/libicalvcal/CMakeFiles/icalvcal.dir/vcc.c.o"
	cd /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/src/libicalvcal && /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/host/usr/bin/mips-unknown-linux-uclibc-gcc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/icalvcal.dir/vcc.c.o   -c /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/src/libicalvcal/vcc.c

src/libicalvcal/CMakeFiles/icalvcal.dir/vcc.c.i: cmake_force
	@echo "Preprocessing C source to CMakeFiles/icalvcal.dir/vcc.c.i"
	cd /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/src/libicalvcal && /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/host/usr/bin/mips-unknown-linux-uclibc-gcc  $(C_DEFINES) $(C_FLAGS) -E /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/src/libicalvcal/vcc.c > CMakeFiles/icalvcal.dir/vcc.c.i

src/libicalvcal/CMakeFiles/icalvcal.dir/vcc.c.s: cmake_force
	@echo "Compiling C source to assembly CMakeFiles/icalvcal.dir/vcc.c.s"
	cd /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/src/libicalvcal && /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/host/usr/bin/mips-unknown-linux-uclibc-gcc  $(C_DEFINES) $(C_FLAGS) -S /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/src/libicalvcal/vcc.c -o CMakeFiles/icalvcal.dir/vcc.c.s

src/libicalvcal/CMakeFiles/icalvcal.dir/vcc.c.o.requires:
.PHONY : src/libicalvcal/CMakeFiles/icalvcal.dir/vcc.c.o.requires

src/libicalvcal/CMakeFiles/icalvcal.dir/vcc.c.o.provides: src/libicalvcal/CMakeFiles/icalvcal.dir/vcc.c.o.requires
	$(MAKE) -f src/libicalvcal/CMakeFiles/icalvcal.dir/build.make src/libicalvcal/CMakeFiles/icalvcal.dir/vcc.c.o.provides.build
.PHONY : src/libicalvcal/CMakeFiles/icalvcal.dir/vcc.c.o.provides

src/libicalvcal/CMakeFiles/icalvcal.dir/vcc.c.o.provides.build: src/libicalvcal/CMakeFiles/icalvcal.dir/vcc.c.o

# Object files for target icalvcal
icalvcal_OBJECTS = \
"CMakeFiles/icalvcal.dir/icalvcal.c.o" \
"CMakeFiles/icalvcal.dir/vobject.c.o" \
"CMakeFiles/icalvcal.dir/vcaltmp.c.o" \
"CMakeFiles/icalvcal.dir/vcc.c.o"

# External object files for target icalvcal
icalvcal_EXTERNAL_OBJECTS =

lib/libicalvcal.so.2.0.0: src/libicalvcal/CMakeFiles/icalvcal.dir/icalvcal.c.o
lib/libicalvcal.so.2.0.0: src/libicalvcal/CMakeFiles/icalvcal.dir/vobject.c.o
lib/libicalvcal.so.2.0.0: src/libicalvcal/CMakeFiles/icalvcal.dir/vcaltmp.c.o
lib/libicalvcal.so.2.0.0: src/libicalvcal/CMakeFiles/icalvcal.dir/vcc.c.o
lib/libicalvcal.so.2.0.0: src/libicalvcal/CMakeFiles/icalvcal.dir/build.make
lib/libicalvcal.so.2.0.0: lib/libical.so.2.0.0
lib/libicalvcal.so.2.0.0: src/libicalvcal/CMakeFiles/icalvcal.dir/link.txt
	@echo "Linking C shared library ../../lib/libicalvcal.so"
	cd /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/src/libicalvcal && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/icalvcal.dir/link.txt --verbose=$(VERBOSE)
	cd /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/src/libicalvcal && $(CMAKE_COMMAND) -E cmake_symlink_library ../../lib/libicalvcal.so.2.0.0 ../../lib/libicalvcal.so.2 ../../lib/libicalvcal.so

lib/libicalvcal.so.2: lib/libicalvcal.so.2.0.0

lib/libicalvcal.so: lib/libicalvcal.so.2.0.0

# Rule to build all files generated by this target.
src/libicalvcal/CMakeFiles/icalvcal.dir/build: lib/libicalvcal.so
.PHONY : src/libicalvcal/CMakeFiles/icalvcal.dir/build

src/libicalvcal/CMakeFiles/icalvcal.dir/requires: src/libicalvcal/CMakeFiles/icalvcal.dir/icalvcal.c.o.requires
src/libicalvcal/CMakeFiles/icalvcal.dir/requires: src/libicalvcal/CMakeFiles/icalvcal.dir/vobject.c.o.requires
src/libicalvcal/CMakeFiles/icalvcal.dir/requires: src/libicalvcal/CMakeFiles/icalvcal.dir/vcaltmp.c.o.requires
src/libicalvcal/CMakeFiles/icalvcal.dir/requires: src/libicalvcal/CMakeFiles/icalvcal.dir/vcc.c.o.requires
.PHONY : src/libicalvcal/CMakeFiles/icalvcal.dir/requires

src/libicalvcal/CMakeFiles/icalvcal.dir/clean:
	cd /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/src/libicalvcal && $(CMAKE_COMMAND) -P CMakeFiles/icalvcal.dir/cmake_clean.cmake
.PHONY : src/libicalvcal/CMakeFiles/icalvcal.dir/clean

src/libicalvcal/CMakeFiles/icalvcal.dir/depend:
	cd /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0 && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0 /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/src/libicalvcal /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0 /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/src/libicalvcal /home/liborklocan/Projects/sagemcom/clones/work/vsaf_work/vf_vox_2_5_it_build/build/libical-2.0.0/src/libicalvcal/CMakeFiles/icalvcal.dir/DependInfo.cmake
.PHONY : src/libicalvcal/CMakeFiles/icalvcal.dir/depend
