# Gemini the GOAT
## Solve
While the challenge uses unshare(CLONE_NEWNS) to create a new mount namespace and pivot_root to jail the process, it does not unshare the PID (Process ID) namespace.

Because you are still in the host's PID namespace and running as root, you can:

Mount the proc filesystem (which contains information about all running processes).

Access the root filesystem of the init process (PID 1), which, by definition, is the true host root.

## Harden it
To patch this challenge and prevent the standard escapes (specifically accessing the host filesystem via /proc), you must isolate the Process ID (PID) namespace. Additionally, as a best practice, you should make the bind mounts read-only to prevent the sandboxed shell from modifying host system files.

Here is the patch applied to your code, maintaining the arcane theme.

The Patch
You need to make three key changes:

Include <sys/wait.h>: Necessary for the parent process to wait for the jailed child.

Unshare CLONE_NEWPID and fork(): The PID namespace only applies to children of the process that unshares it. You must fork immediately after unsharing so the child becomes PID 1 in the new "demi-plane".

Remount as Read-Only: Reinforce the "ley lines" so they cannot be tampered with from inside the vessel.

