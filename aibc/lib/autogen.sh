#!/bin/sh


# Usage: autogen -t -m -a -c
# 
# help_autogen to print help
# 
help_autogen () {
  echo "Usage: $0 -t -m -a -c"
  echo "       You can run the make command to build the project directly" 
  echo "       after running this script."
  echo "Options: "
  echo "These are optional argument:"
  echo " -t target type in [solaris|suse|hpux|aix]"
  echo " -m platform [32|64]. By default, it is set to 32."
  echo " -L extra LDFLAGS. By default, apl install home is set."
  echo " -l extra LIBS to link. By default, it is set to NULL except set"
  echo "    to xnet in HPUX." 
  echo " -a extra options passing to ./configure, except CFLAGS, "
  echo "    CXXFLAGS, LDFLAGS and LIBS."
  echo " -c overwirte all the options (which set by autogen.sh) passing "
  echo "    to ./configure. Be careful, With this option you must"
  echo "    take care of all the options including platform yourself."
  exit 1
}

# set the default value
apl_conf=
apl_confacpath=
apl_m=32
apl_exacfopt=
apl_ovopt=
apl_confopt=
apl_cflags="CFLAGS=\"-g -Wall"
apl_cxxflags="CXXFLAGS=\"-g -Wall"
apl_shell=sh
apl_ldflag=
apl_libs=
CPPUNIT64=/usr/local/lib/cppunit64/lib/ 

# functions
apl_autoreconf () {
    apl_confacpath=`echo $0 | sed "s@\(.*/\)\(.*\)@\1configure.ac@"`
    "$apl_shell" -c "autoreconf -vi $apl_confacpath"
}

apl_configure () {
    # get the path of configure script, same prefix as autogen.sh
    apl_conf=`echo $0 | sed "s@\(.*/\)\(.*\)@\1configure@"`
    "$apl_shell" -c "$apl_conf $apl_confopt $apl_exacfopt"
}

apl_pltvalid() {
    if [ "$1" != "32" ] && [ "$1" != "64" ] ; then 
        help_autogen
    fi
}


# get the options
while getopts t:m:L:l:a:c: opt
do
  case "$opt" in
    t) apl_host="$OPTARG";;
    m) apl_m="$OPTARG";;
    L) apl_ldflag="$OPTARG";;
    l) apl_libs="$OPTARG";;
    a) apl_exacfopt="$OPTARG";;
    c) apl_ovopt="$OPTARG";;
    \?) help_autogen;; esac
done

# check the target host valid or not
if [ "apl$apl_host" = apl ] ; then
    # target host not set, try to get it
    apl_os=`uname`
    if [ "$apl_os" = "SunOS" ] ; then
        apl_host="solaris"
    elif [ "$apl_os" = "AIX" ] ; then
        apl_host="aix"
    elif [ "$apl_os" = "HP-UX" ] ; then
        apl_host="hpux"
    elif [ "$apl_os" = "Linux" ] ; then
        apl_host="suse"
# add by zhangqu at 2011-1-5, for AIBCLib's package about Linux can diff the store dir
        host_alias=`uname -m`-`uname -n`-`uname -s`
        host_alias=`echo $host_alias | tr '[:upper:]' '[:lower:]'`
        apl_exacfopt="$apl_exacfopt"" --host="$host_alias
    else
        echo "Error: Cann't get the target host type, please specify it via the -t option."
        help_autogen
    fi
fi

#take care of the LDFLAGS
if [ "apl$apl_ldflag" = "apl" ] ; then
    # in 64 mode, when lfdlag is not specify, try to add cppunit64 lib if exist
    if [ "$apl_m" = "64" ] && [ -d $CPPUNIT64 ] ; then 
        apl_ldflag="LDFLAGS=\"-L$CPPUNIT64\""
    # in 32 mode, add LDFLAGS for aix 
    elif [ "$apl_m" = "32" ] && [ "$apl_host" = "aix" ] ; then 
        apl_ldflag="LDFLAGS=\"-Wl,-brtl -L/usr/local/lib\""
    fi
else
    apl_ldflag="LDFLAGS=\"-L$apl_ldflag\""
fi

case "$apl_host" in
    solaris)
    # SOLARIS need the sh to support -c option
    apl_shell=/usr/xpg4/bin/sh
    apl_exacfopt="$apl_exacfopt"" --disable-largefile"
    ;;

    suse)
    if [ "$apl_m" = "32" ] ; then
        apl_exacfopt="$apl_exacfopt"" --disable-largefile"
    fi 
    ;;

    aix)
    # add extra option for aix
    apl_exacfopt="$apl_exacfopt"" --disable-largefile"
    ;;

    hpux)
    if [ "$apl_m" = "32" ] ; then
        apl_exacfopt="$apl_exacfopt"" --disable-largefile"
    fi 
    ;;

    *)
    echo "Error: not support target host type!"
    help_autogen
    ;;
esac

# overwrite the default options or not
if [ "$apl_ovopt" != "" ] ; then
    apl_confopt=$apl_ovopt
    # extra options is ignored when overwrite enable
    apl_exacfopt=
else
    apl_pltvalid $apl_m

    if [ "$apl_m" = "32" ] ; then
    	platform=`uname -i 2>/dev/null`
	if [ "$platform" = "x86_64" ] ; then
            apl_m=" -m32\""
	else
            apl_m="\""
	fi
    # add 64 flags based on platform
    else
        case "$apl_host" in
            solaris|suse)
            apl_m=" -m64\""
            ;;

            hpux)
            apl_m=" -mlp64\""
            # hpux64 need the xnet lib
            if [ "apl$apl_libs" = "apl" ] ; then
                apl_libs="LIBS=-lxnet"
            else
                apl_libs="LIBS=\"-l$apl_libs -lxnet\""
            fi
            ;;

            aix)
            apl_m=" -maix64\""
            ;;

            *)
            help_autogen
            ;;
        esac
    fi

    apl_confopt="$apl_cflags""$apl_m ""$apl_cxxflags""$apl_m"" $apl_ldflag"" $apl_libs"

fi

# autoreconf
apl_autoreconf

# configure with options
apl_configure  


exit 0
