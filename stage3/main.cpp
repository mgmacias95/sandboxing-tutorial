#include <iostream>
#include <fstream>
#include <vector>
#include <cerrno>
#include <cstring>
#include <sys/mman.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <linux/seccomp.h>
#include <linux/filter.h>
#include <linux/audit.h>
#include <cstddef>
#include "grimoire_parser.cpp"

// --- ANSI COLOR DEFINITIONS ---
#define C_RESET   "\033[0m"
#define C_RED     "\033[31m"
#define C_GREEN   "\033[32m"
#define C_YELLOW  "\033[33m"
#define C_BLUE    "\033[34m"
#define C_MAGENTA "\033[35m"
#define C_CYAN    "\033[36m"
#define C_BOLD    "\033[1m"

// Helper macro for BPF generation to make the filter readable
#define SC_ALLOW(syscall_nr) \
    BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, (syscall_nr), 0, 1), \
    BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW)

void install_seccomp() {
    std::cout << C_BLUE "[*] Commencing ritual of containment" C_RESET << std::endl;
    std::cout << C_CYAN "[*] Verify Astral Alignment..." C_RESET << std::endl;
    std::cout << C_YELLOW "  [*] if (ARCH != ARCH_X86_64) -> KILL" C_RESET << std::endl;
    std::cout << C_CYAN "[*] Sanctioned Spells:" C_RESET << std::endl;
    std::cout << C_GREEN "  [*] exit (Allow peaceful departure to the void)" C_RESET << std::endl;
    std::cout << C_GREEN "  [*] read (Allow scrying)" C_RESET << std::endl;
    std::cout << C_GREEN "  [*] write (Allow inscribing)" C_RESET << std::endl;
    std::cout << C_GREEN "  [*] open (Allow unsealing tomes)" C_RESET << std::endl;
    std::cout << C_GREEN "  [*] lseek (Allow navigating)" C_RESET << std::endl;
    std::cout << C_GREEN "  [*] close (Allow resealing forbidding tomes)" C_RESET << std::endl;
    std::cout << C_GREEN "  [*] sendfile (Allow sending tomes to the void)" C_RESET << std::endl;
    std::cout << C_BOLD C_RED "[*] Obliterate any spirit attempting unsactioned magic" C_RESET << std::endl;

    struct sock_filter filter[] = {
        // 1. Validate Architecture (x86_64)
        // Load architecture from seccomp_data structure
        BPF_STMT(BPF_LD | BPF_W | BPF_ABS, (offsetof(struct seccomp_data, arch))),
        // If not x86_64, jump 1 instruction forward (to KILL)
        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, AUDIT_ARCH_X86_64, 1, 0),
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_KILL),

        // 2. Load Syscall Number
        BPF_STMT(BPF_LD | BPF_W | BPF_ABS, (offsetof(struct seccomp_data, nr))),

        // 3. Define Allowed Syscalls
        // Allow exit_group (standard process exit)
        SC_ALLOW(__NR_exit_group),
        // Allow exit
        SC_ALLOW(__NR_exit),
        // Allow read (e.g., from stdin)
        SC_ALLOW(__NR_read),
        // Allow write (e.g., to stdout/stderr)
        SC_ALLOW(__NR_write),

        // Grimoire Parser needs to open/seek/close files
        SC_ALLOW(__NR_open),
        SC_ALLOW(__NR_lseek),
        SC_ALLOW(__NR_close),
        SC_ALLOW(__NR_sendfile),

        // 4. Default Action: Kill process for any other syscall
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_KILL)
    };

    struct sock_fprog prog = {
        .len = (unsigned short)(sizeof(filter) / sizeof(filter[0])),
        .filter = filter,
    };

    // Ensure the process cannot gain new privileges.
    // REQUIRED before loading a seccomp filter without CAP_SYS_ADMIN.
    if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0)) {
        std::cerr << C_RED "[-] Ritual FAILED: Could not bind soul (NO_NEW_PRIVS): " 
                  << strerror(errno) << C_RESET << std::endl;
        exit(EXIT_FAILURE);
    }

    // Install the filter
    if (prctl(PR_SET_SECCOMP, SECCOMP_MODE_FILTER, &prog)) {
        std::cerr << C_RED "[-] Ritual FAILED: Warding circle shattered (SECCOMP): " 
                  << strerror(errno) << C_RESET << std::endl;
        exit(EXIT_FAILURE);
    }

    std::cout << C_GREEN "[+] Wards active. Spirit bound to basic cantrips (read/write/exit)." C_RESET << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << C_YELLOW "Usage: " << argv[0] << " <pdf_file>" C_RESET << std::endl;
        return EXIT_FAILURE;
    }

    // Initialize
    initialize_parser();

    // Prepare the parser
    pdf* file = open_grimoire(argv[1]);
    if (file == NULL){
        std::cerr << C_RED "[-] Failed to open PDF file" C_RESET << std::endl;
        exit(1);
    }

    // Install the sandbox
    install_seccomp();

    // Dangerous: parsing of a PDF
    parse_grimoire(file);
    
    return EXIT_SUCCESS;
}