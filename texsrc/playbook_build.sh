#point this to where you have the native SDK environment command installed
. /Developer/SDKs/bbndk-2.1.0/bbndk-env.sh

#point this to the TL source directory
SRC_DIR=texlive-20120701-source

#insert your build machine type here (you could use something like "config.guess" to figure this out)
BUILD_TYPE=x86_64-apple-darwin11.4.0

#add extra autoconf NDK variables (tested with BB NDK 2.1.0)
RANLIB="${QNX_HOST}/usr/bin/ntoarmv7-ranlib "
CPP="${QNX_HOST}/usr/bin/qcc -V4.4.2,gcc_ntoarmv7le_cpp -E "
CC="${QNX_HOST}/usr/bin/qcc -V4.4.2,gcc_ntoarmv7le_cpp "
LD="${QNX_HOST}/usr/bin/ntoarmv7-ld "
CPPFLAGS="-D__PLAYBOOK__ -D__QNXNTO__ "
CFLAGS=" -g "
LDFLAGS=""

export RANLIB CPP CC LD CPPFLAGS CFLAGS LDFLAGS

DISABLED_PROGS="\
    --disable-bibtexu \
    --disable-dvisvgm \
    --disable-lcdf-typetools \
    --disable-luatex \
    --disable-xdv2pdf \
    --disable-xdvik \
    --disable-xdvipdfmx \
    --disable-xetex	\
"
OTHER_OPTS="\
    --without-mf-x-toolkit \
    --without-x \
"

#NDK variables
NDK_CONF_VAR="--build=${BUILD_TYPE} --host=arm-unknown-nto-qnx6.5.0eabi"
SW_ADDCONF_VAR="${DISABLED_PROGS} ${OTHER_OPTS}" 

#clean old builds
rm -r ${SRC_DIR}/Work ${SRC_DIR}/inst

#now configure and build
${SRC_DIR}/Build ${NDK_CONF_VAR} ${SW_ADDCONF_VAR}