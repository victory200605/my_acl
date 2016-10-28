#!/bin/sh
# Usage: make_package_gfq1.sh
# 
# help_make_package_gfq1 to print help
# 
help_make_package_gfq1 () {
  echo ""
  echo "Usage: $0 -m PLATFORM -d FTP_DEST_PATH -h HOST_IP -u FTP_USER -p FTP_PASSWD"
  echo "       Make package script for AIBCDL products." 
  echo ""
  echo "Options: "
  echo " -m PLATFORM,      [Compulsive/Optional] 32 or 64. This would decide the make 32bit or 64bit package"
  echo " -d FTP_DEST_PATH, [Optional] Ftp path to upload the package files, default: ~aibcdist/AIBCGFQ1"
  echo " -h HOST_IP,       [Optional] Ftp host ip. The build package would transfer to HOST_IP."                  
  echo " -u FTP_USER,      [Optional] Ftp account. The build package would transfer to HOST_IP"
  echo "                   with FTP_USER:FTP_PASSWD."
  echo " -p FTP_PASSWD,    [Optional] Ftp passwd. The build package would transfer to HOST_IP"
  echo "                   with FTP_USER:FTP_PASSWD."
  echo ""
  exit 1
}

gfq1_platform=32
gfq1_ftp_destpath="AIBCGFQ1"
gfq1_ftp_user=aibcdist
gfq1_ftp_passwd=d1sta1bc
gfq1_ftp_host=10.3.2.242

# get the options
while getopts m:d:h:u:p: opt
do
  case "$opt" in
    m) gfq1_platform="$OPTARG";;
    d) gfq1_ftp_destpath="$OPTARG";;
    h) gfq1_ftp_host="$OPTARG";;
    u) gfq1_ftp_user="$OPTARG";;
    p) gfq1_ftp_passwd="$OPTARG";;
    \?) help_make_package_gfq1;; esac
done

if [ "$gfq1_platform" = "32" ] ; then
    AIBC_HOME=$AIBC_HOME_32 
    export AIBC_HOME
    GFQ_V2_BIT=32
    export GFQ_V2_BIT
fi

if [ "$gfq1_platform" = "64" ] ; then
    AIBC_HOME=$AIBC_HOME_64
    export AIBC_HOME
    GFQ_V2_BIT=64 
    export GFQ_V2_BIT 
fi

# get the new file
cd $AIBC_SRC_DIR/lib/ailibEx
cvs up -dA
cd $AIBC_SRC_DIR/GFQ/GFQ1
cvs up -dA

# copy the src to the make dir
mkdir $AIBC_SRC_DIR/GFQ/GFQ1-"$gfq1_platform"
cp -r $AIBC_SRC_DIR/GFQ/GFQ1 $AIBC_SRC_DIR/GFQ/GFQ1-"$gfq1_platform"/

# clear the old package
cd $AIBC_SRC_DIR/GFQ/GFQ1-"$gfq1_platform"/GFQ1
rm gfq-*.tar.gz

os=`uname`
if [ "$os" = "HP-UX" ] ; then
    alias make='gmake'
fi

# make ailibEx
cd $AIBC_SRC_DIR/lib/ailibEx
make

if [ $? -ne 0 ] ; then
    echo "Error in make ailibEx ."
    exit 1
fi

make_gfq1 () {
    GFQ_DATA_LEN=$1
    export GFQ_DATA_LEN
    cd $AIBC_SRC_DIR/GFQ/GFQ1-"$gfq1_platform"/GFQ1

    make clean
    if [ $? -eq 0 ] ; then
        make
        if [ $? -eq 0 ] ; then
            make package
            if [ $? -eq 0 ] ; then
                return 0
            fi
        fi
    fi

    return 1
}

# make gfq1
make_gfq1 "1024"
if [ $? -ne 0 ] ; then
    echo "Error in make the package of GFQ1(len=1024) ."
    exit 1
fi
#make_gfq1 "592"
#if [ $? -ne 0 ] ; then
#    echo "Error in make the package of GFQ1(len=592) ."
#    exit 1
#fi
#make_gfq1 "1536"
#if [ $? -ne 0 ] ; then
#    echo "Error in make the package of GFQ1(len=1536) ."
#    exit 1
#fi
#make_gfq1 "4096"
#if [ $? -ne 0 ] ; then
#    echo "Error in make the package of GFQ1(len=4096) ."
#    exit 1
#fi

# ftp the package to the demo server
ac_build_type=`uname -m`-`uname -n`-`uname`
os=`uname`
if [ "$os" = "SunOS" ] ; then
    ac_build_type=`uname -p`-`uname`-`uname -r`
elif [ "$os" = "AIX" ] ; then
    ac_build_type=`uname -p`-`uname`-`uname -v`
elif [ "$os" = "HP-UX" ] ; then
    ac_build_type=`uname -m`-`uname`-`uname -r`
fi
ac_build_type="$ac_build_type""-""$gfq1_platform"

echo "Trasfering packages gfq package to $gfq1_ftp_host ..."

pkgdir_date=`date +"%y-%m-%d"`
pkgdir_time=`date +"%T"`

ftp -n $gfq1_ftp_host <<EOF
quote USER $gfq1_ftp_user
quote PASS $gfq1_ftp_passwd

umask 022
cd $gfq1_ftp_destpath
mkdir $ac_build_type
cd $ac_build_type
mkdir $pkgdir_date
cd $pkgdir_date
mkdir $pkgdir_time
cd $pkgdir_time
binary
prompt off
mput gfq-*.tar.gz
quit
EOF

exit 0
