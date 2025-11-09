# Stage 1: Chroot

## Setup

`Chroot` is used to lock a given process into a specific directory. Once inside, the process will only be able to see the files inside that directory but not anything outside of it.

In order to create a chroot environment, copy all binaries and libraries required. In this example, `bash` and `ls` will be used. Use the `ldd` command to find out which libraries are required:

```shellsession
$ ldd /bin/ls
	linux-vdso.so.1 (0x00007fff025ce000)
	libselinux.so.1 => /lib/x86_64-linux-gnu/libselinux.so.1 (0x00007fd8da6d2000)
	libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x00007fd8da4f0000)
	libpcre2-8.so.0 => /lib/x86_64-linux-gnu/libpcre2-8.so.0 (0x00007fd8da456000)
	/lib64/ld-linux-x86-64.so.2 (0x00007fd8da72e000)
```

The first one, `linux-vdso.so.1`, is part of the kernel so it's not phisically in the disk and hence, doesn't need to be copied.

## Attack

There is a well-known weakness in `chroot` that allows a process with `root` privileges (specifically `CAP_SYS_CHROOT`) inside the jail to escape.

It works by creating a new jail inside the current one, but not changing the Current Working Directory (CWD) into it immediately. Because the CWD is now outside the new (nested) jail root, `chdir("..")` can be used to traverse up past the original chroot boundary.
