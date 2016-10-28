#!/bin/sh
# Usage: make_package_cs.sh
# 
# help_make_package_cs to print help
# 
help_make_package_cs () {
  echo ""
  echo "Usage: $0 -m PLATFORM -d FTP_DEST_PATH -h HOST_IP -u FTP_USER -p FTP_PASSWD"
  echo "       Make package script for AIBCCountServer products." 
  echo ""
  echo "Options: "
  echo " -m PLATFORM,      [Compulsive/Optional] 32 or 64. This would decide the make 32bit or 64bit package"
  echo " -d FTP_DEST_PATH, [Optional] Ftp path to upload the package files, default: ~aibcdist/AIBCCountServer"
  echo " -h HOST_IP,       [Optional] Ftp host ip. The build package would transfer to HOST_IP."                  
  echo " -u FTP_USER,      [Optional] Ftp account. The build package would transfer to HOST_IP"
  echo "                   with FTP_USER:FTP_PASSWD."
  echo " -p FTP_PASSWD,    [Optional] Ftp passwd. The build package would transfer to HOST_IP"
  echo "                   with FTP_USER:FTP_PASSWD."
  echo ""
  exit 1
}

cs_platform=32
cs_ftp_destpath="AIBCCountServer"
cs_ftp_user=aibcdist
cs_ftp_passwd=d1sta1bc
cs_ftp_host=10.3.2.242

# get the options
while getopts m:d:h:u:p: opt
do
  case "$opt" in
    m) cs_platform="$OPTARG";;
    d) cs_ftp_destpath="$OPTARG";;
    h) cs_ftp_host="$OPTARG";;
    u) cs_ftp_user="$OPTARG";;
    p) cs_ftp_passwd="$OPTARG";;
    \?) help_make_package_cs;; esac
done

if [ "$cs_platform" = "32" ] ; then
    AIBC_HOME=$AIBC_HOME_32 
    export AIBC_HOME
    CS_TMP_DIR=$CS_TMP_DIR_32
    export CS_TMP_DIR
    CS_INST_DIR=$CS_INST_DIR_32
    export CS_INST_DIR
fi

if [ "$cs_platform" = "64" ] ; then
    AIBC_HOME=$AIBC_HOME_64
    export AIBC_HOME
    CS_TMP_DIR=$CS_TMP_DIR_64
    export CS_TMP_DIR
    CS_INST_DIR=$CS_INST_DIR_64
    export CS_INST_DIR
fi

cd $CS_SRC_DIR
cvs up -dA
cd $CS_SRC_DIR
autoreconf -i
automake -a
mkdir $CS_TMP_DIR
cd $CS_TMP_DIR

cs_cflags="-g -Wall -m$cs_platform"
cs_cxxflags="-g -Wall -m$cs_platform"
cs_host=""
os=`uname`
if [ "$os" = "HP-UX" ] ; then
    alias make='gmake'
    if [ "$cs_platform" = "32" ]; then
        cs_cflags="-g -Wall -milp$cs_platform"
        cs_cxxflags="-g -Wall -milp$cs_platform"
    elif [ "$cs_platform" = "64" ] ; then
        cs_cflags="-g -Wall -mlp$cs_platform"
        cs_cxxflags="-g -Wall -mlp$cs_platform"
    fi
elif [ "$os" = "AIX" ] ; then
    cs_cflags="-g -Wall -maix$cs_platform"
    cs_cxxflags="-g -Wall -maix$cs_platform"
elif [ "$os" = "Linux" ] ; then
    host_alias=`uname -m`-`uname -n`-`uname -s`
    host_alias=`echo $host_alias | tr '[:upper:]' '[:lower:]'`
    cs_host="--host=""$host_alias"
fi

if [ "$os" = "AIX" ] ; then
    $CS_SRC_DIR/configure LDFLAGS="-Wl,-brtl" CFLAGS="""$cs_cflags""" CXXFLAGS="""$cs_cxxflags""" $cs_host --prefix=$CS_INST_DIR
else
    $CS_SRC_DIR/configure CFLAGS="""$cs_cflags""" CXXFLAGS="""$cs_cxxflags""" $cs_host --prefix=$CS_INST_DIR
fi

make clean
make
make install
if [ $? -ne 0 ] ; then
    echo "Error in make the package of CountServer ."
    exit 1
fi

cd $HOME/aibc/tools/AutoBuild/
./aibc_package.sh -s $CS_SRC_DIR -b $CS_TMP_DIR -i $CS_INST_DIR -d $cs_ftp_destpath -h $cs_ftp_host -u $cs_ftp_user -p $cs_ftp_passwd -m $cs_platform
