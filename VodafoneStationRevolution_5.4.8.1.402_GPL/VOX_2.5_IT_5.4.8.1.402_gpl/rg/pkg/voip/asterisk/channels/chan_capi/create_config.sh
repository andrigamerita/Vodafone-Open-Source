#!/bin/sh
#
# create_config.sh
#
# Script to create config.h for compatibility with
# different asterisk versions.
#
# (C) 2005-2007 Cytronics & Melware
# Armin Schindler <armin@melware.de>
#

CONFIGFILE="config.h"
rm -f "$CONFIGFILE"

VER=1_2

if [ $# -lt 1 ]; then
	echo >&2 "Missing argument"
	exit 1
fi

INCLUDEDIR="$1/asterisk"

if [ ! -d "$INCLUDEDIR" ]; then
	echo >&2 "Include directory '$INCLUDEDIR' does not exist"
	exit 1
fi

echo -n "Checking Asterisk version... "
AVERSIONNUM=`sed -n '/.*ASTERISK_VERSION_NUM /s/^.*ASTERISK_VERSION_NUM //p' $INCLUDEDIR/version.h`
AVERSION=`sed -n '/.*ASTERISK_VERSION /s/^.*ASTERISK_VERSION //p' $INCLUDEDIR/version.h`
AVERSION=`echo $AVERSION | sed 's/\"//g'`
if [ "$AVERSION" = "" ]; then
	AVERSION="trunk"
	VER="1_6"
fi
echo $AVERSION

echo "/*" >$CONFIGFILE
echo " * automatically generated by $0 `date`" >>$CONFIGFILE
echo " */" >>$CONFIGFILE
echo >>$CONFIGFILE
echo "#ifndef CHAN_CAPI_CONFIG_H" >>$CONFIGFILE
echo "#define CHAN_CAPI_CONFIG_H" >>$CONFIGFILE
echo >>$CONFIGFILE

case "$AVERSIONNUM" in
	106*)
		echo "#define CC_AST_HAS_VERSION_1_6" >>$CONFIGFILE
		echo " * found Asterisk version 1.6"
		VER=1_6
		;;
	104*)
		echo "#define CC_AST_HAS_VERSION_1_4" >>$CONFIGFILE
		echo " * found Asterisk version 1.4"
		VER=1_4
		;;
	99999)
		echo "#define CC_AST_HAS_VERSION_1_4" >>$CONFIGFILE
		echo " * assuming Asterisk version 1.4"
		VER=1_4
		;;
	999999)
		echo "#define CC_AST_HAS_VERSION_1_6" >>$CONFIGFILE
		echo " * assuming Asterisk version 1.6"
		VER=1_6
		;;
	*)
		if [ "$VER" = "1_6" ]; then
			echo "#define CC_AST_HAS_VERSION_1_6" >>$CONFIGFILE
			echo " * assuming Asterisk version 1.6"
		else
			echo "#undef CC_AST_HAS_VERSION_1_4" >>$CONFIGFILE
		fi
		;;
esac

check_two_and_four()
{
	if grep -q "AST_STRING_FIELD(name)" $INCLUDEDIR/channel.h; then
		echo "#define CC_AST_HAS_STRINGFIELD_IN_CHANNEL" >>$CONFIGFILE
		echo " * found stringfield in ast_channel"
	else
		echo "#undef CC_AST_HAS_STRINGFIELD_IN_CHANNEL" >>$CONFIGFILE
		echo " * no stringfield in ast_channel"
	fi
	
	if grep -q "const indicate.*datalen" $INCLUDEDIR/channel.h; then
		echo "#define CC_AST_HAS_INDICATE_DATA" >>$CONFIGFILE
		echo " * found 'indicate' with data"
	else
		echo "#undef CC_AST_HAS_INDICATE_DATA" >>$CONFIGFILE
		echo " * no data on 'indicate'"
	fi
	
	if grep -q "ast_channel_alloc.*name_fmt" $INCLUDEDIR/channel.h; then
		echo "#define CC_AST_HAS_EXT_CHAN_ALLOC" >>$CONFIGFILE
		echo " * found extended ast_channel_alloc"
	else
		echo "#undef CC_AST_HAS_EXT_CHAN_ALLOC" >>$CONFIGFILE
		echo " * no extended ast_channel_alloc"
	fi

	if grep -q "ast_channel_alloc.*amaflag" $INCLUDEDIR/channel.h; then
		echo "#define CC_AST_HAS_EXT2_CHAN_ALLOC" >>$CONFIGFILE
		echo " * found second extended ast_channel_alloc"
	else
		echo "#undef CC_AST_HAS_EXT2_CHAN_ALLOC" >>$CONFIGFILE
		echo " * no second extended ast_channel_alloc"
	fi

	if grep -q "send_digit_end.*duration" $INCLUDEDIR/channel.h; then
		echo "#define CC_AST_HAS_SEND_DIGIT_END_DURATION" >>$CONFIGFILE
		echo " * found send_digit_end with duration"
	else
		echo "#undef CC_AST_HAS_SEND_DIGIT_END_DURATION" >>$CONFIGFILE
		echo " * no duration with send_digit_end"
	fi

	if [ "$VER" = "1_2" ]; then
	if grep -q "AST_JB" $INCLUDEDIR/channel.h; then
		if [ ! -f "$INCLUDEDIR/../../lib/asterisk/modules/chan_sip.so" ]; then
			echo "/* AST_JB */" >>$CONFIGFILE
			echo "#define CC_AST_HAS_JB_PATCH" >>$CONFIGFILE
			echo " * assuming generic jitter-buffer patch"
		else
			if grep -q "ast_jb" "$INCLUDEDIR/../../lib/asterisk/modules/chan_sip.so"; then
				echo "/* AST_JB */" >>$CONFIGFILE
				echo "#define CC_AST_HAS_JB_PATCH" >>$CONFIGFILE
				echo " * found generic jitter-buffer patch"
			else
				echo "#undef CC_AST_HAS_JB_PATCH" >>$CONFIGFILE
				echo " * found DISABLED generic jitter-buffer patch"
			fi
		fi
	else
		echo "#undef CC_AST_HAS_JB_PATCH" >>$CONFIGFILE
		echo " * without generic jitter-buffer patch"
	fi
	fi
}

check_version_onesix()
{
	echo "#define CC_AST_HAS_VERSION_1_4" >>$CONFIGFILE
	echo "#define CC_AST_HAS_STRINGFIELD_IN_CHANNEL" >>$CONFIGFILE
	echo "#define CC_AST_HAS_INDICATE_DATA" >>$CONFIGFILE
	echo "#define CC_AST_HAS_EXT_CHAN_ALLOC" >>$CONFIGFILE
	echo "#define CC_AST_HAS_EXT2_CHAN_ALLOC" >>$CONFIGFILE
	echo "#define CC_AST_HAS_SEND_DIGIT_END_DURATION" >>$CONFIGFILE

	if grep -q "int ast_dsp_set_digitmode" $INCLUDEDIR/dsp.h; then
		echo "#define CC_AST_HAS_DSP_SET_DIGITMODE" >>$CONFIGFILE
		echo " * found new 'ast_dsp_set_digitmode' function"
	else
		echo "#undef CC_AST_HAS_DSP_SET_DIGITMODE" >>$CONFIGFILE
		echo " * no new 'ast_dsp_set_digitmode' function"
	fi
	if grep -q "union .* data" $INCLUDEDIR/frame.h; then
		echo "#define CC_AST_HAS_UNION_DATA_IN_FRAME" >>$CONFIGFILE
		echo " * found new union data in ast_frame structure"
	else
		echo "#undef CC_AST_HAS_UNION_DATA_IN_FRAME" >>$CONFIGFILE
		echo " * no new union data in ast_frame structure"
	fi
	if grep -q "ast_channel_release.*struct" $INCLUDEDIR/channel.h; then
		echo "#define CC_AST_HAS_CHANNEL_RELEASE" >>$CONFIGFILE
		echo " * found ast_channel_release function"
	else
		echo "#undef CC_AST_HAS_CHANNEL_RELEASE" >>$CONFIGFILE
		echo " * no new ast_channel_release function"
	fi
	if grep -q "ast_devstate2str.*enum" $INCLUDEDIR/devicestate.h; then
		echo "#define CC_AST_HAS_AST_DEVSTATE2STR" >>$CONFIGFILE
		echo " * found new ast_devstate2str function"
	else
		echo "#undef CC_AST_HAS_AST_DEVSTATE2STR" >>$CONFIGFILE
		echo " * obsolete devstate2str function"
	fi
	if grep -q "ast_request.*requestor" $INCLUDEDIR/channel.h; then
		echo "#define CC_AST_HAS_REQUEST_REQUESTOR" >>$CONFIGFILE
		echo " * found requestor in ast_request"
	else
		echo "#undef CC_AST_HAS_REQUEST_REQUESTOR" >>$CONFIGFILE
		echo " * no requestor in ast_request"
	fi
	if grep -q "ast_register_application2.*void " $INCLUDEDIR/module.h; then
		echo "#undef CC_AST_HAS_CONST_CHAR_IN_REGAPPL" >>$CONFIGFILE
		echo " * no const char in ast_register_application"
	else
		echo "#define CC_AST_HAS_CONST_CHAR_IN_REGAPPL" >>$CONFIGFILE
		echo " * found const char in ast_register_application"
	fi
	if grep -q "ast_channel_alloc.*linkedid" $INCLUDEDIR/channel.h; then
		echo "#define CC_AST_HAS_LINKEDID_CHAN_ALLOC" >>$CONFIGFILE
		echo " * found linkedid in ast_channel_alloc"
	else
		echo "#undef CC_AST_HAS_LINKEDID_CHAN_ALLOC" >>$CONFIGFILE
		echo " * no linkedid in ast_channel_alloc"
	fi
}

case $VER in
	1_2)
		echo "Using Asterisk 1.2 API"
		check_two_and_four
		;;
	1_4)
		echo "Using Asterisk 1.4 API"
		check_two_and_four
		;;
	1_6)
		echo "Using Asterisk 1.6 API"
		check_version_onesix
		;;
	*)
		echo >&2 "Asterisk version invalid."
		exit 1
		;;
esac

echo "" >>$CONFIGFILE
echo "#endif /* CHAN_CAPI_CONFIG_H */" >>$CONFIGFILE
echo "" >>$CONFIGFILE

echo "config.h complete."
echo
exit 0

