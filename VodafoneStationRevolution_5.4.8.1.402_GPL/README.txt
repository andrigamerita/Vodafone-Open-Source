OpenRG GPL package compilation manual

Host requirements:
------------------
Kubuntu 14.04.3 LTS 32-bit

================================================================================
Build from SDK:
================================================================================
It is a build which uses a binary versions of toolchains and other tools and
performs an automatical installation;
If some binary tools are incompatible with your OS - you can try perform a build
from sources, which is described in "Build from sources:" section.
================================================================================

Required files:
---------------
- 5.4.8.1.402.sdk.tar.bz2

Pre-configuration:
---------------
#/ sudo apt-get update
#/ sudo apt-get install xsltproc flex libc-dev gcc bison gawk make g++

Set password for root user if it is not specified:
---------------
#/ sudo -i
#/ passwd
#/ exit

Unpack SDK, run installation script and follow the instructions:
---------------
#/ tar -jxvf 5.4.8.1.402.sdk.tar.bz2
#/ cd 5.4.8.1.402.sdk
#/ chmod +x install.sh
#/ ./install.sh

Follow all the installation instructions to build the tree.
OpenRG GPL image will be at ./build/os/linux/vmlinux

================================================================================
Build from sources:
================================================================================

Required files:
---------------
- gcc-4.8.3.tar.bz2
- jpkg_bcm9636x_vodafone_vox25_it_gpl.lic
- jsl-0.3.0-src.tar.gz
- texinfo_4.13a.dfsg.1-10_i386.deb
- uclibc-crosstools-gcc_source-4.2.4.tar.bz2
- libsass-master092916.zip
- sassc-master092916.zip
- VOX_2.5_IT_5.4.8.1.402_gpl.tar.bz2

Host toolchain:
---------------
Build host toolchain:
#/ sudo apt-get update
#/ sudo apt-get install gcc g++ libmpc-dev make
#/ tar -xvjf gcc-4.8.3.tar.bz2
#/ cd gcc-4.8.3
#/ ./configure -v --enable-languages=c,c++ --program-suffix=-4.8 --enable-threads=posix --prefix=/usr
#/ make
#/ sudo make install
#/ cd ..

Target toolchain:
-----------------
1. Downgrade textinfo to 4.13 version:
#/ sudo apt-get remove texinfo
#/ sudo dpkg -i texinfo_4.13a.dfsg.1-10_i386.deb

2. Build toolchain for MIPS target:
#/ sudo apt-get install bison flex libncurses5-dev libncursesw5-dev gettext
#/ tar -xvjf uclibc-crosstools-gcc_source-4.2.4.tar.bz2
#/ cd buildroot-4.2.4/
#/ sudo make
#/ cd ..

3. Create link to toolchain under openrg directory:
#/ sudo mkdir -p /usr/local/openrg
#/ sudo mkdir -p /usr/local/openrg/i686-jungo-linux-gnu
#/ sudo ln -s /opt/toolchains/uclibc-crosstools-gcc-4.2.4/usr /usr/local/openrg/mips-brcm-linux-uclibc

JavaScript analysis tool
------------------------
1. Build JSLint:
#/ tar -xvzf jsl-0.3.0-src.tar.gz
#/ cd jsl-0.3.0/src
#/ make -f Makefile.ref

2. Copy jsl under openrg directory:
#/ sudo mkdir -p /usr/local/openrg/bin/
#/ sudo cp Linux_All_DBG.OBJ/jsl /usr/local/openrg/bin/
#/ cd ../..

SCSS
-----
#/ unzip libsass-master092916.zip
#/ cd libsass-master
#/ sudo make install
(ignore error "usr/local/lib": File exists)

#/ cd ..
#/ unzip sassc-master092916.zip
#/ cd sassc-master/
#/ sudo make install SASS_LIBSASS_PATH=./../libsass-master/
#/ mkdir -p /usr/local/openrg/bin/
#/ sudo ln -rs bin/sassc /usr/local/openrg/bin/
#/ cd ..

OpenRG GPL:
-----------
1. Install utils required for compilation:
#/ sudo apt-get install ccache xsltproc fop msttcorefonts python mklibs libssl-dev dc gawk squashfs-tools

2. Fix SSL library location issue:
#/ sudo ln -s /lib/i386-linux-gnu/libssl.so.1.0.0 /usr/lib/libssl.so

3. Allow root to log in:
If root user is not specified add it because you need root user and pass for compilation
#/ sudo -i
#/ passwd
#/ exit

4. Build OpenRG GPL:
#/ export I386_TOOLS_PREFIX=/usr
#/ tar -xvjf VOX_2.5_IT_5.4.8.1.402_gpl.tar.bz2
#/ cd ./rg
#/ make config CONFIG_RG_USE_LOCAL_TOOLCHAIN=y LIC=./../jpkg_bcm9636x_vodafone_vox25_it_gpl.lic CONFIG_RG_GPL=y DIST=VOX_2.5_IT
#/ make

5. OpenRG GPL image:
./build/os/linux/vmlinux
