#/bin/bash

mkdir jail
mkdir jail/bin
mkdir jail/lib64
mkdir jail/lib

cp /bin/ls jail/bin/
cp /bin/bash jail/bin/
cp jailbreak jail/

# dependencies of /bin/bash -> $ ldd /bin/bash
cp /lib/x86_64-linux-gnu/libtinfo.so.6 jail/lib/
cp /lib/x86_64-linux-gnu/libc.so.6 jail/lib/
cp /lib64/ld-linux-x86-64.so.2 jail/lib64/

# dependencies of ls
cp /lib/x86_64-linux-gnu/libselinux.so.1 jail/lib/
cp /lib/x86_64-linux-gnu/libpcre2-8.so.0 jail/lib/

sudo chroot jail 
