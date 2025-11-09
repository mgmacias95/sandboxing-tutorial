# Stage 2: Namespaces

## Setup

While `chroot` only changes the apparent root directory, Linux Namespaces allow for the virtualization of various system resources. This means a process can have its own private view of the system, including mounts, networking, users, and process IDs (PIDs). On a high level, Linux namespaces allow the isolation of access to certain system resources. To processes inside the namespace it appears that they have their own instance of the resource and changes to the resource are only visible to processes inside the same namespace. There are seven different types of namespaces, named after the resource they provide isolation for:

- `cgroup` namespaces for Linux cgroups

- `ipc` namespaces for inter process communication

- `net` namespaces for network interfaces

- `mnt` namespaces for mount points

- `pid` namespaces for process IDs

- `user` namespaces for user and group IDs

- `uts` namespaces for the hostname

In this challenge, the binary attempts to create a stronger sandbox than `chroot` by using `unshare(CLONE_NEWNS)`. This creates a new **Mount Namespace (`mnt`)**, decoupling the process's mount points from the host. It then uses `pivot_root` to switch the root filesystem entirely.

At first glance, this seems secure. The original host root is gone from the mount list.

## Attack

The vulnerability lies in *incomplete* isolation. The challenge only unshared the Mount Namespace, but it left the **PID Namespace (`pid`)** shared with the host. Because the sandboxed process is still running as `root` (even if it's inside a container) and shares the host's PID view, it can interact with host processes. Specifically, it can mount the `proc` filesystem.

In Linux, `/proc/[pid]/root` is a symbolic link to the root directory of a given process. Since we can still see PID 1 (the host's `init` process), we can use its `/root` symlink as a backdoor to the real host filesystem. Run the following commands inside the sandbox to escape:

```bash
# Create a mount point for proc if it doesn't exist
mkdir -p /proc

# Mount the proc filesystem.
# Since we are root and in our own mount namespace, this is allowed.
mount -t proc none /proc

# Access the host's filesystem via PID 1's view of the world
cat /proc/1/root/flag
```

## Hardening

To fix this, the sandbox must also isolate process IDs using `CLONE_NEWPID`.

When `CLONE_NEWPID` is used, the first process in the new namespace becomes PID 1 (the container's `init`). It will be unable to see any processes outside its own namespace, rendering the `/proc/1/root` attack impossible.

*Note: `CLONE_NEWPID` only applies to children of the calling process. A properly secured sandbox must `unshare(CLONE_NEWPID)` and immediately `fork()`, creating the jailed environment inside the child process.*

## Beyond Namespaces

Even with complete namespace isolation (PID, Mount, Network, IPC, UTS, User), the process still shares the same kernel as the host.

A sophisticated attacker could use malicious system calls to exploit kernel vulnerabilities and break out. 

**The next layer of defense is restricting which system calls the sandboxed process is allowed to make.**

**-\> Move to Stage 3: Seccomp**