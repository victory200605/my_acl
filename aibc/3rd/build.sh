usage_help()
{
    echo "Usage : $0 [options]"
    echo "  options:"
    echo "     -m platform [32|64] default is 32 bit"
    echo "     -h help"
    exit 1
}

FLAGS_M=32

while getopts m:h opt
do
    case $opt in
    m) FLAGS_M=$OPTARG;;
    h) usage_help;;
    \?) usage_help;;
    esac
done

CFLAGS="-m$FLAGS_M"

OLD_DIR=`pwd`

PLATFORM=`uname | tr ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz`

MAKE=${MAKE:-make}

if [ $PLATFORM = "sunos" ]; then
  LIBS="-lpthread -lposix4"
  export LIBS
  CFLAGS="-m$FLAGS_M"
fi

if [ $PLATFORM = "hp-ux" ]; then
  AILDAP_EXTRA_OPTIONS="--host=ia64-hp-hpux"
  if [ $FLAGS_M = "32" ]; then
    CFLAGS="-ilp$FLAGS_M"
  else
    CFLAGS="-lp$FLAGS_M"
  fi
fi

if [ $PLATFORM = "aix" ]; then
  CFLAGS="-maix$FLAGS_M"
fi

echo "******************************** now building 'openldap'... ********************************"
rm -rf openldap-2.4.23
gunzip -c openldap-2.4.23.tgz | tar xvf -
cd openldap-2.4.23

if [ $PLATFORM = "aix" ]; then
  ./configure LDFLAGS="-Wl,-brtl" CFLAGS="$CFLAGS" --prefix=$HOME/aibc/3rd --enable-ldapd=no --enable-slapd=no --enable-slurpd=no --without-threads --with-cyrus-sasl=no --with-tls=no $AILDAP_EXTRA_OPTIONS
else
  ./configure CFLAGS="$CFLAGS" --prefix=$HOME/aibc/3rd --enable-ldapd=no --enable-slapd=no --enable-slurpd=no --without-threads --with-cyrus-sasl=no --with-tls=no $AILDAP_EXTRA_OPTIONS
fi
cd include
$MAKE clean all install
cd ../libraries
$MAKE clean all install
cd $OLD_PWD

