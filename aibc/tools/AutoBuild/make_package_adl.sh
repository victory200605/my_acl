#!/bin/sh
# Usage: make_package_adl.sh
# 
# help_make_package_adl to print help
# 
help_make_package_adl () {
  echo ""
  echo "Usage: $0 -m PLATFORM -d FTP_DEST_PATH -h HOST_IP -u FTP_USER -p FTP_PASSWD"
  echo "       Make package script for AIBCDL products." 
  echo ""
  echo "Options: "
  echo " -m PLATFORM,      [Compulsive/Optional] 32 or 64. This would decide the make 32bit or 64bit package"
  echo " -o PLATFORM,      [Optional] Enable oracle (1:enable, 0:disable). This would decide the make package of oracle"
  echo " -y PLATFORM,      [Optional] Enable mysql (1:enable, 0:disable). This would decide the make package of mysql"
  echo " -s PLATFORM,      [Optional] Enable sqlite (1:enable, 0:disable). This would decide the make package of sqlite"
  echo " -d FTP_DEST_PATH, [Optional] Ftp path to upload the package files, default: ~aibcdist/AIBCLib"
  echo " -h HOST_IP,       [Optional] Ftp host ip. The build package would transfer to HOST_IP."                  
  echo " -u FTP_USER,      [Optional] Ftp account. The build package would transfer to HOST_IP"
  echo "                   with FTP_USER:FTP_PASSWD."
  echo " -p FTP_PASSWD,    [Optional] Ftp passwd. The build package would transfer to HOST_IP"
  echo "                   with FTP_USER:FTP_PASSWD."
  echo ""
  exit 1
}

adl_platform=32
adl_ftp_destpath="AIBCDL"
adl_ftp_user=aibcdist
adl_ftp_passwd=d1sta1bc
adl_ftp_host=10.3.2.242
adl_enable_oracle=1
adl_enable_mysql=1
adl_enable_sqlite=1
adl_db_conf=

# get the options
while getopts m:o:y:s:d:h:u:p: opt
do
  case "$opt" in
    m) adl_platform="$OPTARG";;
    o) adl_enable_oracle="$OPTARG";;
    y) adl_enable_mysql="$OPTARG";;
    s) adl_enable_sqlite="$OPTARG";;
    d) adl_ftp_destpath="$OPTARG";;
    h) adl_ftp_host="$OPTARG";;
    u) adl_ftp_user="$OPTARG";;
    p) adl_ftp_passwd="$OPTARG";;
    \?) help_make_package_adl;; esac
done

if [ "$adl_enable_oracle" = "0" ] && [ "$adl_enable_mysql" = "0" ] && [ "$adl_enable_sqlite" = "0" ] ; then
    echo "Can't set -o && -y && -s = 0 !"
    exit 1
fi

if [ "$adl_enable_oracle" = "0" ] ; then
    adl_db_conf="$adl_db_conf"" --disable-oracle"
fi

if [ "$adl_enable_mysql" = "0" ] ; then
    adl_db_conf="$adl_db_conf"" --disable-mysql"
fi

if [ "$adl_enable_sqlite" = "0" ] ; then
    adl_db_conf="$adl_db_conf"" --disable-sqlite"
fi

if [ "$adl_platform" = "32" ] ; then
    AIBC_HOME=$AIBC_HOME_32 
    export AIBC_HOME
    ADL_TMP_DIR=$ADL_TMP_DIR_32
    export ADL_TMP_DIR
    ADL_INST_DIR=$ADL_INST_DIR_32
    export ADL_INST_DIR
fi

if [ "$adl_platform" = "64" ] ; then
    AIBC_HOME=$AIBC_HOME_64
    export AIBC_HOME
    ADL_TMP_DIR=$ADL_TMP_DIR_64
    export ADL_TMP_DIR
    ADL_INST_DIR=$ADL_INST_DIR_64
    export ADL_INST_DIR
fi

cd $ADL_SRC_DIR
cvs up -dA
cd $ADL_SRC_DIR
autoreconf -i
mkdir $ADL_TMP_DIR
cd $ADL_TMP_DIR

adl_cflags="-g -Wall -m$adl_platform"
adl_cxxflags="-g -Wall -m$adl_platform"
adl_host=""
os=`uname`
if [ "$os" = "HP-UX" ] ; then
    alias make='gmake'
    if [ "$adl_platform" = "32" ]; then
        adl_cflags="-g -Wall -milp$adl_platform"
        adl_cxxflags="-g -Wall -milp$adl_platform"
    elif [ "$adl_platform" = "64" ] ; then
        adl_cflags="-g -Wall -mlp$adl_platform"
        adl_cxxflags="-g -Wall -mlp$adl_platform"
    fi
elif [ "$os" = "AIX" ] ; then
    adl_cflags="-g -Wall -maix$adl_platform"
    adl_cxxflags="-g -Wall -maix$adl_platform"
elif [ "$os" = "Linux" ] ; then
    host_alias=`uname -m`-`uname -n`-`uname -s`
    host_alias=`echo $host_alias | tr '[:upper:]' '[:lower:]'`
    adl_host="--host=""$host_alias"
fi

if [ "$os" = "AIX" ] ; then
    $ADL_SRC_DIR/configure LDFLAGS="-Wl,-brtl" CFLAGS="""$adl_cflags""" CXXFLAGS="""$adl_cxxflags""" $adl_host --prefix=$ADL_INST_DIR $adl_db_conf
else
    $ADL_SRC_DIR/configure CFLAGS="""$adl_cflags""" CXXFLAGS="""$adl_cxxflags""" $adl_host --prefix=$ADL_INST_DIR $adl_db_conf
fi

make clean
make
make install

if [ $? -ne 0 ] ; then
    echo "Error in make the package of adl ."
    exit 1
fi  

cd $HOME/aibc/tools/AutoBuild/
./aibc_package.sh -s $ADL_SRC_DIR -b $ADL_TMP_DIR -i $ADL_INST_DIR -d $adl_ftp_destpath -h $adl_ftp_host -u $adl_ftp_user -p $adl_ftp_passwd -m $adl_platform
