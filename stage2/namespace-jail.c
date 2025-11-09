#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <sched.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <sys/syscall.h>
#include <fcntl.h>
#include <string.h>
#include <limits.h>

int main(int argc, char **argv) {
    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);

    puts("### The Ritual of Binding begins...");
    puts("You shall be sealed within a demi-plane. A false artifact lies within.");
    puts("Only true wizardry can breach the veil to find the true relic outside.\n");

    // Sealing open portals (closing FDs)
    for (int i = 3; i < 10000; i++) close(i);

    assert(geteuid() == 0 && "ERROR: The ritual requires Archmage (root) privileges.");

    puts("Separating this reality strand from the grand tapestry (unshare)...");
    assert(unshare(CLONE_NEWNS) != -1);

    char vessel[] = "/tmp/demiplane-XXXXXX";
    assert(mkdtemp(vessel) != NULL);
    printf("Forged a new dimensional vessel at `%s`.\n", vessel);

    puts("Obscuring the True Reality from scrying (making / private)...");
    assert(mount(NULL, "/", NULL, MS_REC | MS_PRIVATE, NULL) != -1);
    assert(mount(vessel, vessel, NULL, MS_BIND, NULL) != -1);

    char void_dir[PATH_MAX];
    snprintf(void_dir, sizeof(void_dir), "%s/void", vessel);
    assert(mkdir(void_dir, 0777) != -1);

    puts("Performing the Great Inversion (pivot_root)!");
    assert(syscall(SYS_pivot_root, vessel, void_dir) != -1);

    // Weaving essential ley lines (bind mounts) via a loop for simplicity
    // Mounting /lib and /lib64 is essential for a "demi-plane", 
    // those files won't exist there, and the "summoning" of the shell 
    // will fail.
    const char* ley_lines[] = {"/bin", "/lib", "/lib64", "/usr"};
    for (int i = 0; i < 4; i++) {
        printf("Weaving ley line to %s...\n", ley_lines[i]);
        assert(mkdir(ley_lines[i], 0755) != -1);
        char src[PATH_MAX];
        snprintf(src, sizeof(src), "/void%s", ley_lines[i]);
        assert(mount(src, ley_lines[i], NULL, MS_BIND, NULL) != -1);
    }

    puts("Severing the connection to the True Reality...");
    assert(umount2("/void", MNT_DETACH) != -1);
    assert(rmdir("/void") != -1);

    assert(chdir("/") == 0);

    // Create the illusion
    int fd = open("/flag", O_WRONLY | O_CREAT, 0644);
    write(fd, "FLAG{illusion_of_power}", 23);
    close(fd);

    puts("\nSummoning a lesser spirit (shell) into the vessel. Good luck, apprentice.");
    execl("/bin/bash", "bash", "-p", NULL);
}