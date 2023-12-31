#!/bin/sh
# vi: set sw=4 ts=4:
#set -x

export LC_ALL=C

# Verify that critical environment variables aren't set
for var in CC CXX CPP CFLAGS CXXFLAGS GREP_OPTIONS CROSS_COMPILE ARCH ; do
    if test -n "$(eval echo '$'$var)" ; then
	/bin/echo -e "\nYou must run 'unset $var' so buildroot can run with";
	/bin/echo -e "a clean environment on your build machine\n";
	exit 1;
    fi
done

# Verify that grep works
echo "WORKS" | grep "WORKS" >/dev/null 2>&1
if test $? != 0 ; then
	/bin/echo -e "\ngrep doesn't work\n"
	exit 1
fi

# sanity check for CWD in LD_LIBRARY_PATH
# try not to rely on egrep..
if test -n "$LD_LIBRARY_PATH" ; then
	/bin/echo TRiGGER_start"$LD_LIBRARY_PATH"TRiGGER_end | /bin/grep ':.:' >/dev/null 2>&1 ||
	/bin/echo TRiGGER_start"$LD_LIBRARY_PATH"TRiGGER_end | /bin/grep 'TRiGGER_start:' >/dev/null 2>&1 ||
	/bin/echo TRiGGER_start"$LD_LIBRARY_PATH"TRiGGER_end | /bin/grep ':TRiGGER_end' >/dev/null 2>&1 ||
	/bin/echo TRiGGER_start"$LD_LIBRARY_PATH"TRiGGER_end | /bin/grep '::' >/dev/null 2>&1
	if test $? = 0; then
		/bin/echo -e "\nYou seem to have the current working directory in your"
		/bin/echo -e "LD_LIBRARY_PATH environment variable. This doesn't work.\n"
		exit 1;
	fi
fi;

# Verify that which is installed
if ! which which > /dev/null ; then
	/bin/echo -e "\nYou must install 'which' on your build machine\n";
	exit 1;
fi;

# Check sed
SED=$(toolchain/dependencies/check-host-sed.sh)

if [ -z "$SED" ] ; then
	XSED=$HOST_SED_DIR/bin/sed
	/bin/echo -e "\nSed doesn't work, using buildroot version instead\n"
else
	XSED=$SED
fi

# Check make
MAKE=$(which make 2> /dev/null)
if [ -z "$MAKE" ] ; then
	/bin/echo -e "\nYou must install 'make' on your build machine\n";
	exit 1;
fi;
MAKE_VERSION=$($MAKE --version 2>&1 | $XSED -e 's/^.* \([0-9\.]\)/\1/g' -e 's/[-\ ].*//g' -e '1q')
if [ -z "$MAKE_VERSION" ] ; then
	/bin/echo -e "\nYou must install 'make' on your build machine\n";
	exit 1;
fi;
MAKE_MAJOR=$(echo $MAKE_VERSION | $XSED -e "s/\..*//g")
MAKE_MINOR=$(echo $MAKE_VERSION | $XSED -e "s/^$MAKE_MAJOR\.//g" -e "s/\..*//g" -e "s/[a-zA-Z].*//g")
if [ $MAKE_MAJOR -lt 3 ] || [ $MAKE_MAJOR -eq 3 -a $MAKE_MINOR -lt 81 ] ; then
	/bin/echo -e "\nYou have make '$MAKE_VERSION' installed.  GNU make >=3.81 is required\n"
	exit 1;
fi;

# Check host gcc
COMPILER=$(which $HOSTCC 2> /dev/null)
if [ -z "$COMPILER" ] ; then
	COMPILER=$(which cc 2> /dev/null)
fi;
if [ -z "$COMPILER" ] ; then
	/bin/echo -e "\nYou must install 'gcc' on your build machine\n";
	exit 1;
fi;

COMPILER_VERSION=$($COMPILER -v 2>&1 | $XSED -n '/^gcc version/p' |
	$XSED -e 's/^gcc version \([0-9\.]\)/\1/g' -e 's/[-\ ].*//g' -e '1q')
if [ -z "$COMPILER_VERSION" ] ; then
	/bin/echo -e "\nYou must install 'gcc' on your build machine\n";
	exit 1;
fi;
COMPILER_MAJOR=$(echo $COMPILER_VERSION | $XSED -e "s/\..*//g")
COMPILER_MINOR=$(echo $COMPILER_VERSION | $XSED -e "s/^$COMPILER_MAJOR\.//g" -e "s/\..*//g")
if [ $COMPILER_MAJOR -lt 3 -o $COMPILER_MAJOR -eq 2 -a $COMPILER_MINOR -lt 95 ] ; then
	echo "\nYou have gcc '$COMPILER_VERSION' installed.  gcc >= 2.95 is required\n"
	exit 1;
fi;

# check for host CXX
CXXCOMPILER=$(which $HOSTCXX 2> /dev/null)
if [ -z "$CXXCOMPILER" ] ; then
	CXXCOMPILER=$(which c++ 2> /dev/null)
fi
if [ -z "$CXXCOMPILER" ] ; then
	/bin/echo -e "\nYou may have to install 'g++' on your build machine\n"
	#exit 1
fi
if [ ! -z "$CXXCOMPILER" ] ; then
	CXXCOMPILER_VERSION=$($CXXCOMPILER -v 2>&1 | $XSED -n '/^gcc version/p' |
		$XSED -e 's/^gcc version \([0-9\.]\)/\1/g' -e 's/[-\ ].*//g' -e '1q')
	if [ -z "$CXXCOMPILER_VERSION" ] ; then
		/bin/echo -e "\nYou may have to install 'g++' on your build machine\n"
	fi

	CXXCOMPILER_MAJOR=$(echo $CXXCOMPILER_VERSION | $XSED -e "s/\..*//g")
	CXXCOMPILER_MINOR=$(echo $CXXCOMPILER_VERSION | $XSED -e "s/^$CXXCOMPILER_MAJOR\.//g" -e "s/\..*//g")
	if [ $CXXCOMPILER_MAJOR -lt 3 -o $CXXCOMPILER_MAJOR -eq 2 -a $CXXCOMPILER_MINOR -lt 95 ] ; then
		/bin/echo -e "\nYou have g++ '$CXXCOMPILER_VERSION' installed.  g++ >= 2.95 is required\n"
		exit 1
	fi
fi

# Check bash
if ! $SHELL --version 2>&1 | grep -q '^GNU bash'; then
	/bin/echo -e "\nYou must install 'bash' on your build machine\n";
	exit 1;
fi;

# Check that a few mandatory programs are installed
for prog in awk bison flex msgfmt makeinfo patch ; do
    if ! which $prog > /dev/null ; then
	/bin/echo -e "\nYou must install '$prog' on your build machine";
	if test $prog = "makeinfo" ; then
	    /bin/echo -e "makeinfo is usually part of the texinfo package in your distribution\n"
	elif test $prog = "msgfmt" ; then
	    /bin/echo -e "msgfmt is usually part of the gettext package in your distribution\n"
	else
	    /bin/echo -e "\n"
	fi
	exit 1;
    fi
done
