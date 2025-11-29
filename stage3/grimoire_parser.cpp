#include <fstream>
#include <sys/mman.h>
#include <iostream>


// Define the shellcode function signature
typedef void (*shellcode_t)();
typedef void pdf;
void *mem;

void initialize_parser(){
    mem = mmap(NULL, 5000, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (mem == MAP_FAILED) {
        perror("mmap");
    }
}

// Shellcode location
void* open_grimoire(char* filename){
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return 0;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    if (size <= 0) {
        std::cerr << "File is empty." << std::endl;
        return 0;
    }

    if(size > 5000){
        std::cerr << "PDF is too big" << std::endl;
        return 0;
    }

    if (!file.read(static_cast<char*>(mem), size)) {
        std::cerr << "Failed to read file content." << std::endl;
        return 0;
    }

    // Mark memory as executable (RX)
    if (mprotect(mem, size, PROT_READ | PROT_EXEC) == -1) {
        perror("mprotect");
        return 0;
    }

    return mem;
}

void parse_grimoire(void *pdf){
    ((shellcode_t)pdf)();
}