#!/bin/bash

is_jtools_installed=
install_app=$0
install_ver=11
# set by install_jpkg
JPKG_REQUIRE=5.3.0
JPKG_BIN=
JPKG_CMD=
JPKG_ARGS=
location=

TOOLCHAINS="i686-toolchain i386-toolchain armv5b-toolchain mipseb-toolchain ppc-toolchain \
    arm-920t-le-toolchain armv5l-2-toolchain armv5l-toolchain \
    mipseb-broadcom-toolchain mips64-octeon-toolchain \
    lx4189-ulibc-toolchain mips-linux-uclibc-toolchain mips-linux-uclibc-eb-toolchain \
    mipsel-jungo-linux-gnu-toolchain armv6b-montavista-linux-uclibceabi \
    mips-xway-5_1_1-linux-uclibc mips-brcm-linux-uclibc"
TOOLS="jrguml uml-utilities bridge-utils"
DISTS="uml mips_eb octeon armv6j jnet_server vox-1.5 vox-1.5-gpl xway vox-2.0 \
       vox-2.0-gpl vox-2.5-it vox-2.5-it-gpl vox-2.5-uk vox-2.5-uk-gpl"
COMPONENTS="$TOOLCHAINS $TOOLS $DISTS"
# Put in SELECTED_DIST a specific selection. if not selected - then interactive.
SELECTED_DIST=
RT_BIN=$HOME/.rg_install.$$
JPKG_BIN_TMP=$HOME/.jpkg.$$
TMP_FILES="$RT_BIN $RT_BIN.c $JPKG_BIN_TMP"

JUNGO_SUPPORT=rg_support@jungo.com

# Enable installation from CD
if [ -d jpkgs ]; then
  export JPKG_SEARCH_PATH=jpkgs
fi

q_which()
{
    which $1 2>/dev/null
}

save_to_log()
{
    # TODO: 'script' application does not seem to work at all with '-c' option.
    # also, if we set SHELL=install.sh before running 'script' it still 
    # does not work.
    return

    local script
    if [ "$1" == --in-log ] ; then
        return
    fi
    # do we have 'script' installed? - if not, do not log
    script=`q_which script`
    if [ -z "$script" ] ; then
        return
    fi
    
    # re-run install, under 'script'
    echo "spawn: $instapp_app $*" >> ~/.jpkg.log
    # XXX: fixme - it work
    script -q -c "$install_app --in-log $*" -a ~/.jpkg.log
    exit $?
}

# 'jpkg 4.0.11' --> 4000011
# Note: only relating to first three numbers. i.e 1.2.3 equals 1.2.3.4.5
jpkg_ver_num()
{
    local ver="$@" p jpkg_1 jpkg_2 jpkg_3
    # remove 'jpkg ' at the beginning, if exists
    p=${ver/jpkg /} # 'jpkg 4.0.11.2' --> '4.0.11.2'
    jpkg_1=${p/.*} # '4'
    p=${p#*.} # '0.11.2'
    jpkg_2=${p/.*} # '0'
    p=${p#*.} # '11.2'
    jpkg_3=${p/.*} # '11'
    echo $((((jpkg_1*1000+jpkg_2)*1000+jpkg_3)))
}

jpkg_fail_message()
{
    echo "jpkg failed to download/extract required package. Please check the"
    echo "following:"
    echo "- Check if disk is full"
    echo "- Check that your Internet connection is up"
    echo "- If your office network firewall requires proxy settings for"
    echo "  Internet browsing, please run:"
    echo "  \$ http_proxy=<proxy URL> $install_app"
    echo "If you still have problems, please contact $JUNGO_SUPPORT"
    die "Exiting due to above failure"
}

install_by_location()
{
    local location=$1
    for i in $COMPONENTS ; do
	if [ "$location" == "`cg $i location`" ]; then
	    install_component_and_required $force $i
	    exit 0
	fi
    done

    echo "Error: No such location: $location"
    exit 1
}

check_toolchain_ver()
{
    local ret_ver=$1
    local comp=$2
    local tc_ver_file=`cg $comp location`/version
    local ver_file="";

    eval $ret_ver=""

    if [ ! -e $tc_ver_file ] ; then
        return
    fi

    ver_file=`cat $tc_ver_file`

    if [ -z "$ver_file" ] ; then
        ver_file="0"
    fi

    if [ $ver_file -lt 1 ] ; then
        ver_file=
    fi
    
    eval "$ret_ver=\"$ver_file\""
}

check_jpkg()
{
    local jpkg_num
    local ret_ver=$1

    eval $ret_ver=""

    # check if we are already installed
    if [ -n "$JPKG_BIN" ] ; then
      # do nothing
      JPKG_BIN=$JPKG_BIN
    # if jpkg is in the local directory - use it without questions.
    elif [ -x ./jpkg ] ; then
      JPKG_BIN=./jpkg
    # we normally install jpkg in /usr/local/bin - so also check there
    elif [ `PATH=/usr/local/bin:$PATH q_which jpkg` ] ; then
      JPKG_BIN=`PATH=/usr/local/bin:$PATH q_which jpkg`
    # we did not find any jpkg installed
    else
      return
    fi
    # check its version.
    # also we need handle the obsolete Jungo internal
    # jpkg that was used for updating hosts inside Jungo with toolchain,
    # host-kernel, jumls file systems, jrguml-rgloader, snmp, snmp-local.
    # this tool we renamed to jpkg-v
    # -> we overwrite that jpkg.
    JPKG_VER=`$JPKG_BIN --version 2>/dev/null` # 'jpkg 4.0.1'
    case "$JPKG_VER" in
	jpkg\ *.*.*) jpkg_num=`jpkg_ver_num $JPKG_VER` ;;
    esac
    if ((jpkg_num<$jpkg_require_num)) ; then
	JPKG_BIN=
	return
    fi

    #all good
    JPKG_CMD="$JPKG_BIN $JPKG_ARGS"
    eval "$ret_ver=\"$JPKG_BIN ($JPKG_VER)\""
}

install_jpkg()
{
    local jpkg_num

    wget_timeout=15
    if [ -n "$JPKG_WGET_TIMEOUT" ]; then
        wget_timeout=$JPKG_WGET_TIMEOUT
    fi
    
    echo "installing jpkg"
    # -T because the default timeouts are too long
    cp -v jpkg  $JPKG_BIN_TMP
    JPKG_BIN=/usr/local/bin/jpkg
    rt "mkdir -p /usr/local/bin"
    rt "install $JPKG_BIN_TMP $JPKG_BIN"

    JPKG_VER=`$JPKG_BIN --version` # 'jpkg 4.0.1'
    # sanity test - make sure jpkg installed OK
    if [ "${JPKG_VER/jpkg */ok}" != ok ] ; then
        die "failed installing jpkg"
    fi
    
    jpkg_num=`jpkg_ver_num $JPKG_VER`
    if ((jpkg_num<jpkg_require_num)) ; then
        die "failed installing jpkg: required version is $JPKG_REQUIRE but installed version is $JPKG_VER"
    fi

    JPKG_VER=`$JPKG_BIN --version`
    JPKG_CMD="$JPKG_BIN $JPKG_ARGS"
}

get_lic_ver()
{
    local comp=$1
    local lic=`cg $comp lic`
    if [ -z "$lic" ] ; then
        return
    fi
    local ver=`grep -s '^VERSION: ' $lic`
    ver=${ver/VERSION: /}
    echo $ver
}

cg()
{
    local comp=$1 var=$2
    case $var in
        ver) get_lic_ver $comp ;;
	rg_src_dir) 
	    local ver=`get_lic_ver $comp`
	    if [ -z "$ver" ] ; then
	        ver=5.4
	    fi
	    echo $HOME/rg-$ver
	    ;;
    esac
    case $comp in
	util-linux)
	    case $var in
	        desc) echo "Miscellaneous system utilities" ;;
	        detect_files) echo "getopt" ;;
		detect_type) echo "which" ;;
		installer) echo "install_linux_package" ;;
	    esac
	    ;;
	make)
	    case $var in
	        desc) echo "GNU make utility to maintain groups of programs" ;;
	        detect_files) echo "make" ;;
		detect_type) echo "which" ;;
		installer) echo "install_linux_package" ;;
	    esac
	    ;;
	gcc)
	    case $var in
	        desc) echo "C and C++ compiler" ;;
	        detect_files) echo "gcc" ;;
		detect_type) echo "which" ;;
		installer) echo "install_linux_package" ;;
	    esac
	    ;;
	libc-dev)
	    case $var in
	        desc) echo "GNU C Library: Development Libraries and Header Files" ;;
	        detect_files) echo "/usr/include/inttypes.h" ;;
		detect_type) echo "exist" ;;
		installer) echo "install_linux_package" ;;
	    esac
	    ;;
	bzip2)
	    case $var in
	        desc) echo "Free and open-source file compression program" ;;
	        detect_files) echo "bzip2" ;;
		detect_type) echo "which" ;;
		installer) echo "install_linux_package" ;;
	    esac
	    ;;
	flex)
	    case $var in
	        desc) echo "Fast Lexical Analyzer Generator" ;;
	        detect_files) echo "flex" ;;
		detect_type) echo "which" ;;
		installer) echo "install_linux_package" ;;
	    esac
	    ;;
	xsltproc)
	    case $var in
	        desc) echo "command line XSLT processor" ;;
	        detect_files) echo "xsltproc" ;;
		detect_type) echo "which" ;;
		installer) echo "install_linux_package" ;;
	    esac
	    ;;
	bison)
	    case $var in
	        desc) echo "GNU Project parser generator (yacc replacement)" ;;
	        detect_files) echo "bison" ;;
		detect_type) echo "which" ;;
		installer) echo "install_linux_package" ;;
	    esac
	    ;;
	gawk)
	    case $var in
	        desc) echo "pattern scanning and processing language" ;;
	        detect_files) echo "gawk" ;;
		detect_type) echo "which" ;;
		installer) echo "install_linux_package" ;;
	    esac
	    ;;
        i386-toolchain)
	    case $var in
	        desc) echo "i386 Toolchain" ;;
		location) echo "/usr/local/openrg/i386-jungo-linux-gnu" ;;
	        detect_files) echo "check_toolchain_ver" ;;
	        pkg) echo "toolchain-i386.jpkg" ;;
		detect_type) echo "callback" ;;
		needed_ver) echo "1" ;;
	        installer) echo "install_toolchain" ;;
	    esac
	    ;;
        i686-toolchain)
	    case $var in
	        desc) echo "i686 Toolchain" ;;
		location) echo "/usr/local/openrg/i686-jungo-linux-gnu" ;;
                detect_files) echo "`cg $comp location`/bin/i686-jungo-linux-gnu-gcc" ;;
	        pkg) echo "toolchain-i686-jungo-linux-gnu.jpkg" ;;
	        installer) echo "install_toolchain" ;;
	    esac
	    ;;	    
	ppc-toolchain)
	    case $var in
	        desc) echo "powerpc Toolchain" ;;
	        detect_files) echo "check_toolchain_ver" ;;
		location) echo "/usr/local/openrg/powerpc-jungo-linux-gnu" ;;
		detect_type) echo "callback" ;;
		needed_ver) echo "20060806" ;;
	        pkg) echo "toolchain-ppc_20060806.jpkg" ;;
	        installer) echo "install_toolchain" ;;
	    esac
	    ;;
	mips-brcm-linux-uclibc)
	        case $var in
	        desc) echo "Broadcom uclibc-crosstools-gcc-4.2.3-3 Toolchain" ;;
		location) echo "/usr/local/openrg/mips-brcm-linux-uclibc" ;;
		detect_files) echo "`cg $comp location`/bin/mips-linux-uclibc-gcc" ;;
	        pkg) echo "toolchain-mips-brcm-linux-uclibc.jpkg" ;;
	        installer) echo "install_toolchain" ;;
	    esac
	    ;;
	mips-xway-5_1_1-linux-uclibc)
	        case $var in
	        desc) echo "Lantiq XWAY UGW-4.2 mips_gcc-3.4.6_uClibc-0.9.29 Toolchain" ;;
		location) echo "/usr/local/openrg/mips-xway-5_1_1-linux-uclibc" ;;
		detect_files) echo "`cg $comp location`/bin/mips-linux-uclibc-gcc" ;;
	        pkg) echo "toolchain-mips-xway-5_1_1-linux-uclibc.jpkg" ;;
	        installer) echo "install_toolchain" ;;
	    esac
	    ;;
	mips-linux-uclibc-toolchain)
	        case $var in
	        desc) echo "mips-uclibc Toolchain" ;;
		location) echo "/usr/local/openrg/mips-linux-uclibc" ;;
                detect_files) echo "`cg $comp location`/bin/mips-linux-uclibc-gcc" ;;
	        pkg) echo "toolchain-mips-linux-uclibc.jpkg" ;;
	        installer) echo "install_toolchain" ;;
	    esac
	    ;;
	mips-linux-uclibc-eb-toolchain)
	        case $var in
	        desc) echo "mips-linux-uclibc-eb Toolchain" ;;
		location) echo "/usr/local/openrg/mips-linux-uclibc-eb" ;;
                detect_files) echo "`cg $comp location`/bin/mips-linux-uclibc-gcc" ;;
	        pkg) echo "toolchain-mips-uclibc-eb.jpkg" ;;
	        installer) echo "install_toolchain" ;;
	    esac
	    ;;
	mipsel-jungo-linux-gnu-toolchain)
	        case $var in
	        desc) echo "mipsel-jungo-linux-gnu Toolchain" ;;
		location) echo "/usr/local/openrg/mipsel-jungo-linux-gnu" ;;
                detect_files) echo "`cg $comp location`/bin/mipsel-jungo-linux-gnu-gcc" ;;
	        pkg) echo "toolchain-mipsel-jungo-linux-gnu.jpkg" ;;
	        installer) echo "install_toolchain" ;;
	    esac
	    ;;
        armv5l-toolchain)
	        case $var in
	        desc) echo "armv5l Toolchain" ;;
                location) echo "/usr/local/openrg/armv5l-jungo-linux-gnu" ;;
                detect_files) echo "`cg $comp location`/bin/armv5l-jungo-linux-gnu-gcc" ;;
	        pkg) echo "toolchain-armv5l.jpkg" ;;
	        installer) echo "install_toolchain" ;;
	    esac
	    ;;
        armv5l-2-toolchain)
	        case $var in
	        desc) echo "armv5l-2 Toolchain (newer version)" ;;
                location) echo "/usr/local/openrg/armv5l-jungo-linux-gnu-2" ;;
		detect_files) echo "check_toolchain_ver" ;;
		detect_type) echo "callback" ;;
		needed_ver) echo "20061019" ;;
	        pkg) echo "toolchain-armv5l-2_20061019.jpkg" ;;
	        installer) echo "install_toolchain" ;;
	    esac
	    ;;
        solos-toolchain)
	        case $var in
	        desc) echo "Conexant Solos virata toolchain" ;;
                location) echo "/usr/local/virata/tools_v10.1c/redhat-9-x86" ;;
		detect_files) echo "check_toolchain_ver" ;;
		detect_type) echo "callback" ;;
	        pkg) echo "toolchain-solos.jpkg" ;;
	        installer) echo "install_toolchain" ;;
	    esac
	    ;;
        arm-920t-le-toolchain)
	        case $var in
	        desc) echo "arm-920t-le Toolchain" ;;
                location) echo "/usr/local/openrg/arm_920t_le" ;;
                detect_files) echo "`cg $comp location`/bin/arm_920t_le-gcc" ;;
	        pkg) echo "toolchain-arm-920t-le.jpkg" ;;
	        installer) echo "install_toolchain" ;;
	    esac
	    ;;
        mipseb-toolchain)
	        case $var in
	        desc) echo "mipseb old Toolchain for bcm9634x boards" ;;
                location) echo "/usr/local/openrg/mipseb-jungo-linux-gnu" ;;
		detect_files) echo "check_toolchain_ver" ;;
		detect_type) echo "callback" ;;
		needed_ver) echo "20061024" ;;
	        pkg) echo "toolchain-mipseb_20061024.jpkg" ;;
	        installer) echo "install_toolchain" ;;
	    esac
	    ;;
        mipseb-broadcom-toolchain)
	        case $var in
	        desc) echo "Broadcom mipseb Toolchain" ;;
                location) echo "/usr/local/openrg/mipseb-broadcom-linux-uclibc" ;;
                detect_files) echo "`cg $comp location`/bin/mipseb-broadcom-linux-uclibc-gcc" ;;
	        pkg) echo "toolchain-mipseb-broadcom-linux-uclibc.jpkg" ;;
	        installer) echo "install_toolchain" ;;
	    esac
	    ;;
        mipseb-sb1250-toolchain)
	        case $var in
	        desc) echo "mipseb-sb1250 Toolchain" ;;
                location) echo "/usr/local/openrg/mips-jungo_sb1_soft-linux-gnu" ;;
		detect_files) echo "check_toolchain_ver" ;;
	        pkg) echo "toolchain-mipseb-sb1250_20061206.jpkg" ;;
		detect_type) echo "callback" ;;
	        installer) echo "install_toolchain" ;;
		needed_ver) echo "20061206" ;;
	    esac
	    ;;
        mips64-octeon-toolchain)
	        case $var in
	        desc) echo "mips64-octeon Toolchain" ;;
		detect_files) echo "check_toolchain_ver" ;;
		detect_type) echo "callback" ;;
	        pkg) echo "toolchain-mips64-octeon_20090323.jpkg" ;;
	        installer) echo "install_toolchain" ;;
                location) echo "/usr/local/openrg/mips64-octeon-linux-gnu" ;;
		needed_ver) echo "20090323" ;;
	    esac
	    ;;	    
        armv5b-toolchain)
	        case $var in
	        desc) echo "armv5b Toolchain" ;;
	        detect_files) echo "check_toolchain_ver" ;;
		detect_type) echo "callback" ;;
                location) echo "/usr/local/openrg/armv5b-jungo-linux-gnu" ;;
		needed_ver) echo "20090303" ;;
	        pkg) echo "toolchain-armv5b_20090303.jpkg" ;;
	        installer) echo "install_toolchain" ;;
	    esac
	    ;;
        armv6b-montavista-linux-uclibceabi)
            case $var in
                desc) echo "armv6b-montavista-linux-uclibceabi Toolchain" ;;
                detect_files) echo "`cg $comp location`/bin/arm_v6_be_uclibc-gcc" ;;
                location) echo "/usr/local/openrg/armv6b-montavista-linux-uclibceabi" ;;
                pkg) echo "toolchain-armv6b-montavista_20081216.jpkg" ;;
                installer) echo "install_toolchain" ;;
            esac
            ;;
        mips_eb)
	    case $var in
	        desc) echo "OpenRG `cg $comp ver` for Broadcom 6358 reference board" ;;
	        lic) echo "jpkg_mipseb.lic" ;;
	        require) echo "i386-toolchain mipseb-broadcom-toolchain" ;;
	        installer) echo "install_src" ;;
		maketargets) echo "bcm96358" ;;
		post_install) echo "post_install_openrg" ;;
	    esac
	    ;;
        vox-1.5)
	    case $var in
	        desc) echo "OpenRG `cg $comp ver` for VOX 1.5" ;;
	        lic) echo "jpkg_bcm9636x_vodafone.lic" ;;
	        require) echo "i386-toolchain mips-brcm-linux-uclibc" ;;
	        installer) echo "install_src" ;;
		makeflags) echo "DIST=VOX_1.5_IT" ;;
		post_install) echo "post_install_openrg" ;;
	    esac
	    ;;
	vox-1.5-gpl)
	    case $var in
	        desc) echo "OpenRG `cg $comp ver` GPL for VOX 1.5" ;;
	        lic) echo "jpkg_bcm9636x_vodafone_gpl.lic" ;;
	        require) echo "i386-toolchain mips-brcm-linux-uclibc jslint" ;;
	        installer) echo "install_src" ;;
		makeflags) echo "DIST=VOX_1.5_IT CONFIG_RG_GPL=y" ;;
		post_install) echo "post_install_openrg" ;;
	    esac
	    ;;
        vox-2.0)
	    case $var in
	        desc) echo "OpenRG `cg $comp ver` for VOX 2.0" ;;
	        lic) echo "jpkg_xway_vodafone_de.lic" ;;
	        require) echo "i386-toolchain mips-xway-5_1_1-linux-uclibc" ;;
	        installer) echo "install_src" ;;
		makeflags) echo "DIST=VOX_2.0_DE" ;;
		post_install) echo "post_install_openrg" ;;
	    esac
	    ;;
	vox-2.0-gpl)
	    case $var in
	        desc) echo "OpenRG `cg $comp ver` GPL for VOX 2.0" ;;
	        lic) echo "jpkg_xway_vodafone_de_gpl.lic" ;;
	        require) echo "i386-toolchain mips-xway-5_1_1-linux-uclibc" ;;
	        installer) echo "install_src" ;;
		makeflags) echo "DIST=VOX_2.0_DE CONFIG_RG_GPL=y" ;;
		post_install) echo "post_install_openrg" ;;
	    esac
	    ;;
        vox-2.5-it)
	    case $var in
	        desc) echo "OpenRG `cg $comp ver` for VOX 2.5 Italy" ;;
	        lic) echo "jpkg_bcm9636x_vodafone_vox25_it.lic" ;;
	        require) echo "i386-toolchain i686-toolchain mips-brcm-linux-uclibc" ;;
	        installer) echo "install_src" ;;
		makeflags) echo "DIST=VOX_2.5_IT" ;;
		post_install) echo "post_install_openrg" ;;
	    esac
	    ;;
	vox-2.5-it-gpl)
	    case $var in
	        desc) echo "OpenRG `cg $comp ver` GPL for VOX 2.5 Italy" ;;
	        lic) echo "jpkg_bcm9636x_vodafone_vox25_it_gpl.lic" ;;
	        require) echo "i386-toolchain i686-toolchain mips-brcm-linux-uclibc jslint sassc" ;;
	        installer) echo "install_src" ;;
		makeflags) echo "DIST=VOX_2.5_IT CONFIG_RG_GPL=y" ;;
		post_install) echo "post_install_openrg" ;;
	    esac
	    ;;
        vox-2.5-uk)
	    case $var in
	        desc) echo "OpenRG `cg $comp ver` for VOX 2.5 UK" ;;
	        lic) echo "jpkg_bcm9636x_vodafone_vox25_uk.lic" ;;
	        require) echo "i386-toolchain i686-toolchain mips-brcm-linux-uclibc" ;;
	        installer) echo "install_src" ;;
		makeflags) echo "DIST=VOX_2.5_UK" ;;
		post_install) echo "post_install_openrg" ;;
	    esac
	    ;;
	vox-2.5-uk-gpl)
	    case $var in
	        desc) echo "OpenRG `cg $comp ver` GPL for VOX 2.5 UK" ;;
	        lic) echo "jpkg_bcm9636x_vodafone_vox25_uk_gpl.lic" ;;
	        require) echo "i386-toolchain i686-toolchain mips-brcm-linux-uclibc jslint sassc" ;;
	        installer) echo "install_src" ;;
		makeflags) echo "DIST=VOX_2.5_UK CONFIG_RG_GPL=y" ;;
		post_install) echo "post_install_openrg" ;;
	    esac
	    ;;
        bcm96358)
	    case $var in
	        desc) echo "OpenRG `cg $comp ver` for Broadcom 6358 reference board" ;;
		makeflags) echo "DIST=BCM96358" ;;
	    esac
	    ;;
        octeon)
	    case $var in
	        desc) echo "OpenRG `cg $comp ver` for Cavium Octeon CN3XXX" ;;
	        lic) echo "jpkg_octeon.lic" ;;
	        require) echo "i386-toolchain mips64-octeon-toolchain" ;;
	        installer) echo "install_src" ;;
		makeflags) echo "DIST=CN3XXX" ;;
		post_install) echo "post_install_openrg" ;;
	    esac
	    ;;	    
        asus6020vi_26)
	    case $var in
	        desc) echo "OpenRG `cg $comp ver` for Broadcom 6348 Asus AAM6020VI" ;;
                makeflags) echo "DIST=ASUS6020VI_26" ;;
	    esac
	    ;;
        lx4189-ulibc-toolchain)
	    case $var in
	        desc) echo "LX4189 Ulibc Toolchain" ;;
		location) echo "/usr/local/openrg/lx4189-uclibc" ;;
                detect_files) echo "/usr/local/openrg/lx4189-uclibc/bin/lx4189-uclibc-gcc" ;;
	        pkg) echo "toolchain-lx4189-ulibc.jpkg" ;;
	        installer) echo "install_toolchain" ;;
	    esac
	    ;;
	armv6j)
	    case $var in
	        desc) echo "OpenRG `cg $comp ver` for Minspeed Comcerto-100" ;;
		lic) echo "jpkg_armv6j.lic" ;;
		require) echo "i386-toolchain armv5l-2-toolchain" ;;
		maketargets) echo "bb-router" ;;
	        installer) echo "install_src" ;;
		post_install) echo "post_install_openrg" ;;
	    esac
	    ;;
	bb-router)
	    case $var in
	        desc) echo "OpenRG `cg armv6j ver` for Mindspeed BB-Router" ;;
                makeflags) echo "DIST=BB-ROUTER" ;;
	    esac
            ;;
        rgloader_uml)
            case $var in
                desc) echo "OpenRG Boot Loader `cg $comp ver` for UML" ;;
	        detect_files) echo "/usr/local/jungo/etc/jrguml/rgloader.img" ;;
		# 5.0.8 is the first version, there is not need to upgrade any
		# existing rgloader.img
		detect_type) echo "exist" ;;
                pkg) echo "jrguml-rgloader_5.0.8.jpkg" ;;
                installer) echo "install_toolchain" ;;
            esac
            ;;
	xway)
	    case $var in
	        desc) echo "OpenRG `cg $comp ver` for XWAY reference boards: ARX and VRX";;
	        lic) echo "jpkg_mipseb_xway.lic" ;;
	        require) echo "i386-toolchain mips-xway-5_1_1-linux-uclibc" ;;
	        installer) echo "install_src" ;;
		maketargets) echo "arx arx_eth vrx" ;;
		post_install) echo "post_install_openrg" ;;
	    esac
	    ;;
        arx)
	    case $var in
	        desc) echo "OpenRG `cg $comp ver` for ARX188 with DSL WAN (Accelerated) and Ethernet WAN (Non-accelerated)" ;;
		makeflags) echo "DIST=ARX188" ;;
	    esac
	    ;;
        arx_eth)
	    case $var in
	        desc) echo "OpenRG `cg $comp ver` for ARX188 with Ethernet WAN only (Accelerated)" ;;
		makeflags) echo "DIST=ARX188_ETH" ;;
	    esac
	    ;;
        vrx)
	    case $var in
	        desc) echo "OpenRG `cg $comp ver` for VRX288 reference board" ;;
		makeflags) echo "DIST=VRX288" ;;
	    esac
	    ;;

        jnet_server)
	    case $var in
	        desc) echo "Jungo.Net Server `cg $comp ver`" ;;
	        lic) echo "jpkg_jnet_server.lic" ;;
	        require) echo "i386-toolchain" ;;
	        installer) echo "install_src" ;;
		makeflags) echo "DIST=JNET_SERVER" ;;
		post_install) echo "post_install_jnet_server" ;;
	    esac
	    ;;
	jpkg-exe)
	    case $var in
	        installer) echo "install_jpkg" ;;
		detect_files) echo "check_jpkg" ;;
		detect_type) echo "callback" ;;
	    esac
	    ;;
        uml-src)
	    case $var in
                desc) echo "UML sources - OpenRG Emulated over Linux" ;;
                lic) echo "jpkg_uml.lic" ;;
                installer) echo "install_src" ;;
	    esac
	    ;;
        uml_24)
	    case $var in
	        desc) echo "OpenRG `cg $comp ver` emulated over user mode linux (UML) kernel 2.4" ;;
                makeflags) echo "DIST=UML" ;;
	    esac
	    ;;
	uml_26)
	    case $var in
	        desc) echo "OpenRG `cg $comp ver` emulated over user mode linux (UML) kernel 2.6" ;;
                makeflags) echo "DIST=UML_26" ;;
	    esac
	    ;;
        uml)
	    case $var in
                desc) echo "OpenRG `cg $comp ver` emulated over user mode linux (UML) kernel 2.4 and 2.6" ;;
                lic) echo "jpkg_uml.lic" ;;
                require) echo "jrguml uml-src i386-toolchain uml-utilities bridge-utils" ;;
                maketargets) echo "uml_24 uml_26" ;;
		post_install) echo "post_install_openrg" ;;
	    esac
	    ;;
	# This is actually pkg/tools later will be renames to tools
	jrguml)
	    case $var in
	        desc) echo "Virtual board for OpenRG UML" ;;
                lic) echo "jpkg_uml.lic" ;;
                require) echo "i386-toolchain uml-src rgloader_uml" ;;
                post_install) echo "post_install_jrguml" ;;
		detect_files) echo /usr/local/jungo/bin/jrguml ;;
		detect_type) echo "exist" ;;
	    esac
	    ;;
        uml-utilities)
	    case $var in
	        desc) echo "User Mode Linux Utilities" ;;
		installer) echo "install_linux_package" ;;
                detect_files) echo "tunctl uml_mconsole" ;;
		detect_type) echo "which" ;;
	    esac
	    ;;
	bridge-utils)
	    case $var in
	        desc) echo "utilities for configuring the linux ethernet bridge." ;;
		installer) echo "install_linux_package" ;;
		detect_files) echo "brctl" ;;
		detect_type) echo "which" ;;
	    esac
	    ;;
        jslint)
	    case $var in
	        desc) echo "The JavaScript Code Quality Tool" ;;
                detect_files) echo "/usr/local/openrg/bin/jsl" ;;
		detect_type) echo "exist" ;;
	        pkg) echo "jslint_20090702.jpkg" ;;
	        installer) echo "install_toolchain" ;;
	    esac
	    ;;	    
        sassc)
	    case $var in
	        desc) echo "libsass command line driver" ;;
                detect_files) echo "/usr/local/openrg/bin/sassc" ;;
		detect_type) echo "exist" ;;
	        pkg) echo "sassc_20150626.jpkg" ;;
	        installer) echo "install_toolchain" ;;
	    esac
	    ;;	    
    esac
}

usage()
{
    if [ -n "$1" ] ; then
        echo "$@"
    fi

    echo "usage: $0 [options] [ component | --location component_location ]"
    echo "Options:"
    echo "  --version|-V: type install script version and exit"
    echo "  --force: ignore detections, and install anyway the component"
    echo "  -h, --help: this help page"
    echo "Components:"
    echo "  all - OpenRG and required Packges (default)"
    for i in $COMPONENTS ; do
        echo "  $i - `cg $i desc`"
    done
    echo "Component_location:"
    echo "  Install the component whose location is as specified, e.g.:"
    echo "    $0 --location /usr/local/openrg/i386-jungo-linux-gnu"
    echo "  will install the component \"i386-toolchain\""
    exit 1
}

die()
{
    if [ -n "$1" ] ; then
        echo "$@"
    fi
    rm -f $TMP_FILES
    exit 1
}

read_yesno()
{
    local prompt=$1
    local def=$2
    local ans_var=$3
    local ans_value=
    while ((1)) ; do
        echo -n "$prompt "
        read ans_value
	if [ -z "$ans_value" -a -n "$def" ] ; then
	    ans_value=$def
	    break
	fi
	if [ "$ans_value" == Y -o "$ans_value" == y ] ; then
	    ans_value=Y
	    break
	fi
	if [ "$ans_value" == N -o "$ans_value" == n ] ; then
	    ans_value=N
	    break
	fi
	echo -n "Expected Y or N"
	if [ -n "$def" ] ; then
	    echo -n ", or ENTER for $def"
	fi
	echo
    done
    eval $ans_var=$ans_value
}

install_toolchain()
{
    local comp=$1
    local pkg=`cg $comp pkg`

    $JPKG_CMD --update-cache $pkg || jpkg_fail_message
    rt "HOME=$HOME $JPKG_CMD -x $pkg -C /" || jpkg_fail_message
}

install_linux_package()
{
    local comp=$1
    local desc=`cg $comp desc`

    echo Package $comp: $desc
    echo To install $comp, please open another terminal and install the general
    echo linux package $comp.
    echo "To install $comp, open a command prompt as root and:"
    echo "Debian/Knoppix:"
    echo "    Type:"
    echo "    apt-get install $comp"
    echo "RedHat/Mandrake/Fedora/Suse:"
    echo "    Download/locate $comp-x.y.z-arch.rpm package."
    echo "    To find this package you can google '$comp rpm'."
    echo "    After downloading, type:"
    echo "    rpm -Uvh $comp-x.y.z-arch.rpm"

    echo

    # Allow user to install until user either aborts or completes installation
    while ((1)) ; do
        read_yesno "To abort installation of $comp press N, to continue after you are done installing press Y or Enter: " \
	    C ans

        if [ "$ans" == N ]; then 
            echo Aborting installation of $comp
            break
        fi

        detect_comp $comp
        if [ $? -eq 0 ]; then
	    return
	else
	    echo "Package $comp still not installed"
	fi
 
    done
}

# Returned by install_src()
install_src_dir=

set_install_src_dir()
{
    local comp=$1
    local dir ans
    local default_dir=`cg $comp rg_src_dir`
    if [ -n "$install_src_dir" ]; then
        return
    fi
    while ((1)) ; do
	read -p "Enter directory to install sources [$default_dir]: " dir
	echo

	if [ -z "$dir" ] ; then
	    dir=$default_dir
	else
	    # Tilde expansion
	    eval dir=$dir
	fi
	if [ "${dir/*\/*/ok}" != ok ] ; then
	    echo "Expected a directory name (must include a '/')."
	    continue
	fi

	if [ ! -d $dir ] ; then
	    mkdir $dir || die
	else
	    echo "$dir already exists. Installing an additional architecture on "
	    echo "same source tree will enable you to compile the same single "
	    echo "source tree to multiple targets."
	    read_yesno "Continue installation to $dir? [Y/n] " Y ans
	    if [ N == "$ans" ] ; then
	        continue
	    fi
	fi
	break
    done
    install_src_dir=$dir
}
    
install_src()
{
    local comp=$1
    local lic=`cg $comp lic`

    set_install_src_dir $comp

    if [ -n "$lic" ] ; then
	cp $lic $install_src_dir || die
    fi

    $JPKG_CMD -x -C $install_src_dir -T $lic || jpkg_fail_message
}

JRGUML_IMPORTANT_NOTE="IMPORTANT: To use the newly installed tools you must open a new BASH shell (to let changes 'PATH' take effect)"

uml_install_instructions()
{
    local lic=$1
    local toconsole=$2
    local dir=$install_src_dir
    local file=$dir/compilation_readme.txt
    local console=/dev/null

    if [ "$toconsole" == Y ]; then
        console=/dev/stdout
    fi

    cat << END | tee -a $console >> $file
Instructions for manual installation of uml host tools
------------------------------------------------------
$ cd $dir/rg
$ make config DIST=HOSTTOOLS  LIC=$dir/$lic
$ make 
Obtain root permission:
$ JUSER=$USER su
# make make_install_uml

$JRGUML_IMPORTANT_NOTE

END
}

post_install_jrguml()
{
    local comp=$1
    local lic=`cg jrguml lic`
    local dir=$install_src_dir

    echo
    echo "The jrguml package is a set of host tools that control OpenRG/UML."
    echo "These tools need to be compiled, and then installed in /usr/local/bin."
    echo "This compilation process will take between 10 to 30 minutes."
    read_yesno "Compile and install now the jrguml host tools [Y/n]? " Y ans
    if [ "$ans" == "N" ]; then
	uml_install_instructions $lic Y
	return        
    fi
 
    (cd $dir/rg && make config DIST=HOSTTOOLS LIC=$dir/$lic) && \
    (cd $dir/rg && make) && \
    JUSER=$USER rt "cd $dir/rg && make make_install_uml"

    if [ "$?" == 0 ]; then
	uml_install_instructions $lic N
        echo "Installation of jrguml completed successfully!"
	echo
    else
        echo "Installation of jrguml failed"
	uml_install_instructions $lic Y
	echo
        read_yesno "Continue with installations [Y/n]? " Y ans
	if [ "$ans" == "N" ]; then
	    die "Aborting"
	fi
    fi
    install_jtools 
}

one_compilation_instruction()
{
    local desc=$1
    local dir=$2
    local lic=$3
    local makeflags=$4
    local file=$5
    local img_file=$6



    cat << END | tee -a $file
$desc Compilation
--------------------------------------------------
To build the $img_file run:
\$ cd $dir/rg
\$ make config $makeflags LIC=$dir/$lic && make

END
}

install_jtools()
{
    if [ -n "$is_jtools_installed" ]; then
        return
    fi
    if [ -n "$NO_JTOOLS" ]; then
        return
    fi
    is_jtools_installed=y
    local dir=$install_src_dir

    if [ ! -f $dir/rg/pkg/tools/Makefile.jtools ]; then
        return
    fi
    if ! JUSER=$USER rt "cd $dir/rg && make -C pkg/tools -f Makefile.jtools install_jtools"; then
        die "Failed installing jtools, aborting." 
    fi
}

post_install_openrg()
{
    install_jtools 
    print_compilation_instructions "$@"
}

print_compilation_instructions()
{ 
    local comp=$1
    local maketargets=`cg $comp maketargets`
    local lic=`cg $comp lic`
    local dir=$install_src_dir
    local file=$dir/compilation_readme.txt
    local desc maketargets
    
    if [ -z "$maketargets" ]; then
        maketargets=$comp
    fi
    
    echo | tee -a $file
    echo "********************************************************"
    echo

    # If license file include makeflags, use it and not default makeflags
    if [ -n "`echo $(grep "MAKEFLAGS:" $lic | cut -d ":" -f 2)`" ]; then
        one_compilation_instruction "$desc" "$dir" "$lic" "" "$file" "openrg.img"

	return;
    fi

    for i in $maketargets; do
        desc=`cg $i desc`
        makeflags=`cg $i makeflags`

	if 
		echo $i | grep "^rgloader_" > /dev/null
	then
		img_file=rgloader.img
	elif 
	        echo "$makeflags" | grep "CONFIG_RG_GPL" > /dev/null
	then
	        img_file=binaries
	else
		img_file=openrg.img
	fi

	
        one_compilation_instruction "$desc" "$dir" "$lic" "$makeflags" "$file"  "$img_file"

    done
}

jnet_change_ip_addr()
{
    local conf_file=$1
    local old_ip=$2
    local new_ip=$3

    echo "Replacing $old_ip to $new_ip in $conf_file"
    sed "s/$old_ip/$new_ip/g" $conf_file > new_file
    mv new_file $conf_file
}

read_and_set_jnet_server_ips()
{
    local conn_mplex_ip jnet_cgi_ip jrms_cgi_ip
    local def_jnet_cgi_ip=194.90.113.104
    local def_jrms_cgi_ip=194.90.113.105
    local def_conn_mplex_ip=194.90.113.114
    local httpd_conf=$install_src_dir/rg/pkg/jnet/server/common/httpd.conf.jnet
    local jnet_conf=$install_src_dir/rg/pkg/jnet/server/common/jnet.conf
    
    read -p "Please enter IP Address for Jungo ACS (CONN_MPLEX) [$def_conn_mplex_ip]: " conn_mplex_ip
    if [ ! -z "$conn_mplex_ip" ]; then
        jnet_change_ip_addr $jnet_conf $def_conn_mplex_ip $conn_mplex_ip
	jnet_change_ip_addr $jnet_conf "acs.jungo.net" $conn_mplex_ip
    fi

    read -p "Please enter IP Address for Jungo.NET CGI [$def_jnet_cgi_ip]: " jnet_cgi_ip
    if [ ! -z "$jnet_cgi_ip" ]; then
        jnet_change_ip_addr $jnet_conf $def_jnet_cgi_ip $jnet_cgi_ip
        jnet_change_ip_addr $jnet_conf "www.jungo.net" $jnet_cgi_ip
        jnet_change_ip_addr $httpd_conf $def_jnet_cgi_ip $jnet_cgi_ip
    fi

    read -p "Please enter IP Address for JRMS CGI [$def_jrms_cgi_ip]: " jrms_cgi_ip
    if [ ! -z "$jrms_cgi_ip" ]; then
    	jnet_change_ip_addr $jnet_conf $def_jrms_cgi_ip $jrms_cgi_ip
    	jnet_change_ip_addr $jnet_conf "maint.jungo.net" $jrms_cgi_ip
    	jnet_change_ip_addr $httpd_conf $def_jrms_cgi_ip $jrms_cgi_ip
    fi
}

post_install_jnet_server()
{ 
    local comp=$1
    local maketargets=`cg $comp maketargets`
    local lic=`cg $comp lic`
    local dir=$install_src_dir
    local file=$dir/compilation_readme.txt
    local desc maketargets ans
    
    echo "*********************************************************************"
    echo "Jungo.net server uses 3 IP addresses: one for J-ACS, one for "
    echo "Jungo.net CGI, and one for JRMS CGI. If you chose not to define "
    echo "these IPs now, you will be required to set these IPs in "
    echo "$dir/rg/pkg/jnet/server/common/jnet.conf and "
    echo "$dir/rg/pkg/jnet/server/common/httpd.conf.jnet manually, "
    echo "before running make install_all."
    read_yesno "Do you want to define these IPs now? [Y/n]" Y ans
    if [ "$ans" == Y ]; then
        read_and_set_jnet_server_ips
    fi
    
    if [ -z "$maketargets" ]; then
        maketargets=$comp
    fi
    
    echo | tee -a $file
    echo "********************************************************"
    echo

    # If license file include makeflags, use it and not default makeflags
    if [ -n "`echo $(grep "MAKEFLAGS:" $lic | cut -d ":" -f 2)`" ]; then
        one_compilation_instruction "$desc" "$dir" "$lic" "" "$file" "jnet server"
	return;
    fi

    img_file="jnet server"
    for i in $maketargets; do
        desc=`cg $i desc`
        makeflags=`cg $i makeflags`
        one_compilation_instruction "$desc" "$dir" "$lic" "$makeflags" "$file"  "$img_file"
    done
}

mk_rt_bin()
{
    local file=$RT_BIN
    local ans

    cat << END > $file.c
    #include <sys/types.h>
    #include <unistd.h> 

    int main(int argc, char* argv[])
    {
        setuid(0);
        setgid(0);
        execvp(argv[1], argv+1);
        return 1;
    }
END
    `q_which gcc || q_which cc` -o $file $file.c || die could not compile $file

    echo "The package requires installation in /usr/local/bin directory,"
    echo "which requires root permissions."
    echo "To continue installation - please enter the root password:"
    while ((1)) ; do
        su root -c "chown root.root $file && chmod 4755 $file"
        if [ "$?" == 0 ] ; then
	    break
        fi
	echo "Failed login as root."
	read_yesno "Do you want to retry? [Y/n]: " Y ans
	if [ "$ans" == N ] ; then
	    die "Aborting installation"
	fi
    done
}

BUILT_RT_BIN=
rt()
{
    # if we have root permissions, just run the command
    if [ $EUID -eq 0 ] ; then
      /bin/bash -c "$@"
      return
    fi
    if [ "$BUILT_RT_BIN" != Y ] ; then
        mk_rt_bin
        BUILT_RT_BIN=Y
    fi
    $RT_BIN su -c "$@"
}

# Update $PATH for detection with 'which'
export PATH="/sbin:/usr/sbin:$PATH"
detect_comp()
{
    local comp=$1
    local rv=0
    
    local detect_files=`cg $comp detect_files`

    echo "Detecting $comp"
    
    for i in $detect_files; do 
	echo "  Detecting $i: "
	local detect_type=`cg $comp detect_type`
	if [ "$detect_type" == which ]; then
	    ver=`q_which $i`
	elif [ "$detect_type" == exist ]; then
	    ver=`ls $i 2>/dev/null`
	elif [ "$detect_type" == callback ]; then
	    $i ver $comp
	else
	    ver=`$i --version  2>/dev/null | head -n 1`
	fi

	local needed_ver=`cg $comp needed_ver`

	if [ -n "$ver" ] ; then
	    if [ -n "$needed_ver" ] && [ $needed_ver -gt $ver ] ; then
	        echo "    found ($ver), but needs ($needed_ver)"
		rv=1
	    else
	        echo "    found ($ver)"
	    fi
	else
	    echo "    not found"
	    rv=1
	fi
    done

    if [ $rv -eq 0 ]; then
        echo "Component $comp is installed"
    else
        echo "Component $comp is not installed"
    fi
    return $rv
}

in_list()
{
    local item=$1
    local list=$2

    for i in $list; do
        if [ $i == "$item" ]; then 
	    echo 1
	    return
	fi
    done
    
    echo 0
}

prepare_required()
{
    local comp
    local recdep
    local ans detect_files ver force_flag

    while ((1)) ; do
        case "$1" in
	--force) force_flag=Y ;;
	--recdep)
	    shift
	    recdep=$1
	    ;;
	*) break ;;
	esac
	shift
    done
    
    # Avoid too deep recursion
    if [ -n "$recdep" ] ; then
	if ((recdep>5)) ; then
	    die "Internal error: Recursion too deep"
	fi
    fi
    recdep=$((recdep + 1))

    comp=$1
   
    # do we already have this component in the list?
    if ((`in_list $comp "$REQUIRED"`)); then
        return
    fi
    while ((1)) ; do
	# did the user specifically request to install it anyway?
	if [ -n "$force_flag" ] ; then
	    break
	fi

	detect_files=`cg $comp detect_files`
	if [ -n "$detect_files" ]; then
	    detect_comp $comp

	    if [ $? -eq 0 ] ; then
		echo "  (you may run $install_app --force $comp to re-install)"
		return
	    fi
	fi
	break
    done
    REQUIRED="$comp $REQUIRED"
    for i in `cg $comp require`; do
	prepare_required --recdep $recdep $i
    done
 }

install_component()
{
    local comp=$1
    local installer=`cg $comp installer`

    if [ -z "$installer" ]; then
        return
    fi

    $installer $comp
}

post_install_component()
{
    local comp=$1
    local post_installer=`cg $comp post_install`

    if [ -z "$post_installer" ]; then
        return
    fi

    $post_installer $comp
}

install_component_and_required()
{
    local comp=$1
    local i ans
    local file

    echo "Preparing the list of dependent components required for installation."

    prepare_required $force $comp

    # Following components are always required and must be installed before any
    # other action
    prepare_required jpkg-exe
    prepare_required gcc
    prepare_required make
    prepare_required libc-dev
    prepare_required bzip2
    prepare_required flex
    prepare_required xsltproc
    prepare_required bison
    prepare_required gawk
    # XXX in Fedora Core the libc-dev package called glibc-devel, so we need
    # better error handling

    if [ -z "$REQUIRED" ]; then
	echo
	echo "Nothing to be installed"
	exit 0
    fi
    
    echo "The following components are about to be installed: $REQUIRED."
    read_yesno "Continue? [Y/n]" Y ans
    if [ $ans != Y ] ; then
        die "Aborting"
    fi

    license_agreement

    for i in $REQUIRED; do
	install_component $i
    done
    
    for i in $REQUIRED; do
	post_install_component $i
    done

    file=$install_src_dir/compilation_readme.txt
    if [ -e $file ] ; then
	echo "A copy of the compilation instructions are in '$file'."
    fi

    # jrguml was installed but PATH does not yet recognize it 
    # if more "post post" install tasks arise: make it a callback:
    # (cg $comp last_task)
    if [ -z "`q_which jrguml`" -a -x /usr/local/jungo/bin/jrguml ]; then
	echo $JRGUML_IMPORTANT_NOTE
    fi
}

# sets $SELECTED_DIST
select_dist()
{
    local ans desc cnt msg
    while ((1)) ; do
	available_dists=
	msg=
        cnt=0
	for i in $DISTS; do
	    if [ -e "`cg $i lic`" ]; then
	        msg="$msg  [$i] `cg $i desc`\n"
		available_dists="$available_dists $i"
		cnt=$((cnt+1))
		ans=$i
	    fi
	done

        if [ $cnt -eq 0 ]; then 
	    die "Internal error, no dist available for installation (or no"\
	    	"matching license files were found in the current dir)"
        elif [ $cnt -eq 1 ]; then
	    echo "Selected dist is $ans"
	    desc=$ans
	else
	    echo "Please select the distribution to install:"
	    echo -e -n "$msg"
	    echo -n "Select one of the above: "
	    read ans
	    desc=`cg "$ans" desc`
	fi

	if [ -n "$desc" ] ; then
	    break
        fi
	echo "invalid selection - expected one of: $available_dists"
	echo
    done
    SELECTED_DIST=$ans
}

license_agreement()
{
    local ans desc err=Y

    LICTEXT=`ls LICENSE ../../../LICENSE 2>/dev/null  | head -n 1`
    if [ -z "$LICTEXT" ]; then
        die "Critical error: Cannot find license agreement, aborting."
    fi
 
    echo
    more $LICTEXT
    echo

    echo "Do you agree to the above license agreement?"
    while ((1)) ; do
	echo -n "Please type 'I AGREE' or 'I DO NOT AGREE': "
	read ans
	if [ "$ans" == "I AGREE" ]; then
	    echo
	    return;
	elif [ "$ans" == "I DO NOT AGREE" ]; then
	    echo "Thank you."
	    exit 0;
	else
	    echo "Expected one of: 'I AGREE' or 'I DO NOT AGREE'"
	fi
    done
}

install_all()
{
    local desc=
    local comp=$SELECTED_DIST
    if [ -n "$comp" ] ; then
        desc=" - `cg $comp desc`"
        echo "Welcome to OpenRG $desc"
    else
        echo "Welcome to OpenRG installation"
    fi
    echo
    echo "OpenRG is an advanced software platform for networking gateways."
    echo "You are about to install a complete development envirnoment:"
    echo "- Development tools - Compiler, Assembler, Linker and Debugger"
    echo "- Operating System - Linux"
    echo "- Protocol stacks - Including TCP/IP, ATM, VLAN, Wireless, PPP, "
    echo "  IPSec, RIP, SNMP, Radius, HTTP and UPnP"
    echo "- Applications - Including Firewall, NAT, VoIP, QoS, File Server,"
    echo "  Print Server and Mail Server"
    echo
    if [ -z "$comp" ] ; then
        select_dist
	comp=$SELECTED_DIST
    fi

    install_component_and_required $force $comp
}

jpkg_require_num=`jpkg_ver_num $JPKG_REQUIRE`

if ! detect_comp util-linux > /dev/null ; then
    echo "$0 uses the local host utility getopt, which is part of the linux"
    echo "package util-linux, Use the following instructions to install it."
    echo
    install_linux_package util-linux
fi

if ! args="`getopt -l 'help,verbose,version,force,debug,location:' Vhvfd \"$@\"`"; then
    usage
    exit 2
fi

eval set -- $args

while : ; do
    case "$1" in
	--)
	  shift;
	  break;
	  ;;
	--help|-h)
	  usage
	  ;;
	--force|-f)
	  shift
	  force=--force
	  ;;
	--verbose|-v)
	  shift
	  JPKG_ARGS="$JPKG_ARGS -v"
	  ;;
	--debug|-d)
	  JPKG_ARGS="$JPKG_ARGS -d"
	  shift
	  ;;
	--location)
	  shift
	  location=$1
	  shift
	  break
	  ;;
	--version|-V)
	  echo $install_ver
	  exit 0
	  ;;
	-*)
	  echo Error: invalid option "'$1'"
	  usage
	  ;;
    esac
done

# save_to_log "$@"

if [ "$force" == "--force" ] ; then
    #user wrote only "--force" with no comp description
    if [ $# == 0 ] ; then
	echo Error: --force must be followed by a component
        usage
    fi
fi

if [ $# == 0 ] ; then
    install_all
elif [ -n "$location" ] ; then
    install_by_location $location
elif [ "$1" == all ] ; then
    install_all
elif [ $# == 1 ] && [ -n "`cg \"$1\" desc`" ] ; then
    install_component_and_required $force $1
elif [ $# -gt 1 ] ; then
    echo "Error: select only one component"
    usage
else
    echo "Error: invalid component '$1'"
    usage
fi

exit 0

