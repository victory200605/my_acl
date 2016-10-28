#!/bin/sh
#
#
#   Copyright (C) Asiainfo, Inc.
#
# Copying and distribution of this file, with or without modification, are 
# permitted provided the copyright notice and this notice are preserved.
#
#
#



# Usage: aibc_package.sh
# 
# help_aibc_package to print help
# 
help_aibc_package () {
  echo ""
  echo "Usage: $0 -s PKG_SRC_PATH -b PKG_BUILD_PATH -i PKG_INST_PATH"
  echo "       Package script for AIBC products." 
  echo ""
  echo "Options: "
  echo " -s PKG_SRC_PATH,  [Compulsive] source path. Specified where your source code of the"
  echo "                   package."
  echo " -b PKG_BUILD_PATH,[Compulsive] build path. Specified where you executed \"configure\"."
  echo " -i PKG_INST_PATH, [Compulsive] install path. Specified where you made install to."
  echo "                   Should same as the \"--prefix\" when configure. "
  echo "                   The PKG_INST_PATH should contain at least two "
  echo "                   directory: \"include\" and \"lib\"."
  echo " -d FTP_DEST_PATH, [Optional] Ftp path to upload the package files, default: ~aibcdist/AIBCLib"
  echo " -h HOST_IP,       [Optional] Ftp host ip. The build package would transfer to HOST_IP."                  
  echo " -u FTP_USER,      [Optional] Ftp account. The build package would transfer to HOST_IP"
  echo "                   with FTP_USER:FTP_PASSWD."
  echo " -p FTP_PASSWD,    [Optional] Ftp passwd. The build package would transfer to HOST_IP"
  echo "                   with FTP_USER:FTP_PASSWD."
  echo " -m PLATFORM,      [Compulsive/Optional] 32 or 64. This would decide the path of the"
  echo "                   package in HOST_IP. When option h/u/p is set, this option is compulsive,"
  echo "                   or it is optional."
  echo ""
  exit 1
}


aibc_src_path=
aibc_build_path=
aibc_inst_path=
aibc_ftp_destpath="AIBCLib"
aibc_target_path=
aibc_ftp_user=
aibc_ftp_passwd=
aibc_ftp_host=
aibc_platform=
working_dir=
current_dir=`pwd`

# get the options
while getopts s:b:i:d:t:u:p:m:h: opt
do
  case "$opt" in
    s) aibc_src_path="$OPTARG";;
    b) aibc_build_path="$OPTARG";;
    i) aibc_inst_path="$OPTARG";;
    d) aibc_ftp_destpath="$OPTARG";;
    t) aibc_target_path="$OPTARG";;
    u) aibc_ftp_user="$OPTARG";;
    p) aibc_ftp_passwd="$OPTARG";;
    h) aibc_ftp_host="$OPTARG";;
    m) aibc_platform="$OPTARG";;
    \?) help_aibc_package;; esac
done

if [ "aibc$aibc_src_path" = "aibc" ] || [ "aibc$aibc_build_path" = "aibc" ] || [ "aibc$aibc_inst_path" = "aibc" ] ; then
    help_aibc_package
    exit 1
fi

if [ $? -ne 0 ] ; then
    echo "Can't create temporary directory."
    exit 1
fi


# Get package name and version thru configure.ac
if [ ! -f "$aibc_src_path/configure.ac" ] ; then
    echo "Error: Can't find configure.ac file in $aibc_src_path!"
    exit 1
fi

#AC_INIT([libaibc],[1.0.0],[fengzz@asiainfo.com])
ac_init_line=`grep AC_INIT "$aibc_src_path/configure.ac"`
package_name=`echo $ac_init_line | sed 's/AC_INIT(\[\(.*\)\],\[.*\],\[.*/\1/g'`
package_version=`echo $ac_init_line | sed 's/AC_INIT(\[.*\],\[\(.*\)\],\[.*/\1/g'`

_os=`uname`
if [ "$_os" = "HP-UX" ] ; then
    working_dir=`mktemp -d /tmp`
    `mkdir $working_dir`
else 
    working_dir=`mktemp -d 2>/dev/null`
fi

if [ "$working_dir" = "" ] ; then
    working_dir=/tmp/__aibc_pkg_tmp
    `rm -rf $working_dir > /dev/null`
    `mkdir $working_dir`
fi

# copy path/src to dest/path/src
#1 path/src
#2 dest
#3 absolute path of src except $1
copy_path_files () {

    if [ "aibc$1" = "aibc" ] || [ "aibc$2" = "aibc" ] || [ "aibc$3" = "aibc" ]; then
        return
    fi

    copy_src_dir=`dirname $1`

    #change to working directory
    cd $3

    if [ ! -d "$2/$copy_src_dir" ] ; then
        mkdir -p "$2/$copy_src_dir"
    fi

    cp -f $1 "$2/$copy_src_dir"
}


#$1 filename
#$2 find type in [lib include bin test ]
find_file_in_srcbuild () {

    cd $current_dir
    ret_build=`find $aibc_build_path -name $1 2>/dev/null`

    ret_src=`find $aibc_src_path -name $1 2>/dev/null`

    if [ ! -n "$ret_build" ] && [ ! -n "$ret_src" ] ; then 
        adl=`echo $aibc_inst_path/$2/$1 | grep adl`
        if [ ! -n "$adl" ] ; then
            return 0
        fi
    fi

    if [ ! -d "$working_dir/$2" ] ; then 
        mkdir "$working_dir/$2"
    fi

    return 1
}

#$1 find type in [lib include bin test ]
extract_package_files () {

    if [ -d "$aibc_inst_path/$1" ] ; then
        inst_list=`cd "$aibc_inst_path/$1" && find *`
        for f in $inst_list
        do
            #find lib files
            if [ -f "$aibc_inst_path/$1/$f" ] ; then
                h=`basename $f`
                find_file_in_srcbuild $h "$1"
                if [ $? -ne 0 ] ; then 
                    copy_path_files "$f" "$working_dir/$1" "$aibc_inst_path/$1"
                fi
            fi
        done
    fi
}


# modify *.la libraries
#$1 library name
#$2 depend libraries. [1 2 3] means [apl acl anf], and 0 means nothing.
modify_lib_la () {
    if [ $2 -eq 1 ] ; then
        sed "s/^dependency_libs=.*/dependency_libs=\'-lapl\'/g" "$working_dir/$1" | sed "s@^libdir=.*@libdir=\'/usr/local/lib/\'@g" > "$working_dir/$1_mo"
    elif [ $2 -eq 2 ] ; then
        sed "s/^dependency_libs=.*/dependency_libs=\'-lapl\ -lacl\'/g" "$working_dir/$1" | sed "s@^libdir=.*@libdir=\'/usr/local/lib/\'@g" > "$working_dir/$1_mo"
    elif [ $2 -eq 3 ] ; then
        sed "s/^dependency_libs=.*/dependency_libs=\'-lapl\ -lacl\ -lanf\'/g" "$working_dir/$1" | sed "s@^libdir=.*@libdir=\'/usr/local/lib/\'@g" > "$working_dir/$1_mo"
    else
        sed "s/^dependency_libs=.*/dependency_libs=\'\ \'/g" "$working_dir/$1" | sed "s@^libdir=.*@libdir=\'/usr/local/lib/\'@g" > "$working_dir/$1_mo"
    fi

    mv "$working_dir/$1_mo" "$working_dir/$1" 
}


#1 package name
targz_package () {
    pkg_path=`dirname $1`
    pkg=`basename $1`
    cd $pkg_path
    tar -cf "$pkg.tar" $pkg 2>/dev/null
    gzip -c "$pkg.tar" > "$pkg.tar.gz"
    rm -rf "$pkg.tar" $1
}


# Get all the lib files
extract_package_files "lib" 

# Get all include files
extract_package_files "include" 

# Get all bin files
extract_package_files "bin" 

# Get all tests files
# extract_package_files "test" 

# Get all changelog files
changelogs=`cd $current_dir && cd $aibc_src_path && find * | grep ChangeLog`
for cl in $changelogs
do
    cl_suffix=`dirname $cl`
    # get suffix for changelog
    if [ "$cl_suffix" != "." ] ; then
        cl_suffix=`echo $cl_suffix | sed 's/\//\./g'`
        cl_suffix=".$cl_suffix"
    else
        cl_suffix=
    fi
    cd $current_dir
    cp -pR "$aibc_src_path/$cl" "$working_dir/ChangeLog$cl_suffix"
done

# Get all config files
cfg_files=`cd $current_dir && find $aibc_src_path -type f -name \*.ini`
if [ "$cfg_files" != "" ] ; then
    mkdir "$working_dir/config"
    for cfgfile in $cfg_files
    do
        cp "$cfgfile" "$working_dir/config"
    done
fi

# Modify *.la
lib_las=`cd $working_dir && find * | grep .la`
for la in $lib_las
do
    #special handle for aibc lib
    la_name=`basename $la`
    if [ "$la_name" = "libapl.la" ] ; then
        modify_lib_la $la 0
    elif [ "$la_name" = "libacl.la" ] ; then
        modify_lib_la $la 1
    elif [ "$la_name" = "libanf.la" ] ; then
        modify_lib_la $la 2
    elif [ "$la_name" = "libScpProxyApi.la" ] ; then
        modify_lib_la $la 3
    else 
        modify_lib_la $la 2
    fi
done


# testing which md5 tools we use
tmp_touch=`mktemp > /dev/null 2>&1`
if [ $? -ne 0 ] ; then
    tmp_touch=/tmp/__aibc_pkg_tmp_touch
fi

#detect md5gen.py first
if [ -x "$current_dir/md5gen.py" ] ; then
    #use md5gen.py
    md5_tool="$current_dir/md5gen.py"
else #use shell md5sum 
    md5sum $tmp_touch > /dev/null 2>&1
    if [ $? -eq 0 ] ; then
        md5_tool=md5sum
    else 
        echo "Can't detect any md5sum tool in shell or in current directory."
        echo "Make sure \"md5sum\" is executable in shell or md5gen.py is in"
        echo "current directory"
        rm -rf $tmp_touch 
        rm -rf $working_dir
        exit 1
    fi
fi

rm -rf $tmp_touch 
bin_pkg="$current_dir/$package_name-bin-$package_version"
dev_pkg="$current_dir/$package_name-dev-$package_version"


# Generate packages
if [ -d "$dev_pkg" ] ; then
    rm -rf "$dev_pkg"
fi

if [ -d "$bin_pkg" ] ; then
    rm -rf "$bin_pkg"
fi

cp -pR $working_dir "$dev_pkg"
cp -pR $working_dir "$bin_pkg"

#remove include files from bin package
if [ -d "$bin_pkg/include" ] ; then
    rm -rf "$bin_pkg/include"
fi

#remove static libs from bin package
static_libs=`cd "$bin_pkg/lib/" && find * | grep "\.a"`
for lib in $static_libs  
do
    rm -rf "$bin_pkg/lib/$lib"
done

# add md5
cd "$bin_pkg" && find * -type f -exec $md5_tool {} > "$bin_pkg/md5sum" \; 
cd "$dev_pkg" && find * -type f -exec $md5_tool {} > "$dev_pkg/md5sum" \; 

# tar packages
targz_package  "$bin_pkg"
targz_package  "$dev_pkg"

rm -rf $working_dir

if [ "aibc$aibc_ftp_user" = "aibc" ] || [ "aibc$aibc_ftp_passwd" = "aibc" ] || [ "aibc$aibc_ftp_host" = "aibc" ] ; then
    exit 0
fi

if [ "aibc$aibc_platform" = "aibc" ] ; then
    print "Please set platform with -m option: -m32 or -m64"
    exit -1
fi

# ftp to release-machine and distribute
binpkg=`basename $bin_pkg`
devpkg=`basename $dev_pkg`
binpkg=$binpkg.tar.gz
devpkg=$devpkg.tar.gz
pkglist="$binpkg $devpkg"
#get build platform name from config.log
#ac_build_type=`grep ac_cv_build $aibc_build_path/config.log`
#ac_build_type=`echo $ac_build_type | sed 's/ac_cv_build=\(.*\)/\1/g'`
#modify by zhangqu at 2011-1-5, for AIBCLib's package about Linux can diff the store dir
ac_build_type=`grep ac_cv_host $aibc_build_path/config.log`
ac_build_type=`echo $ac_build_type | sed 's/ac_cv_host=\(.*\)/\1/g'`
ac_build_type=$ac_build_type-$aibc_platform
pkgdir_date=`date +"%y-%m-%d"`
pkgdir_time=`date +"%T"`

echo "Trasfering packages $pkglist to $aibc_ftp_host ..."

ftp -n $aibc_ftp_host <<EOF
quote USER $aibc_ftp_user
quote PASS $aibc_ftp_passwd

umask 022
cd $aibc_ftp_destpath
mkdir $ac_build_type
cd $ac_build_type
mkdir $pkgdir_date
cd $pkgdir_date
mkdir $pkgdir_time
cd $pkgdir_time
binary
put $binpkg
put $devpkg
quit
EOF

exit 0
