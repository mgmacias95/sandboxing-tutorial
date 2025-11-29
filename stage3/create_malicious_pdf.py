from pwn import *

if __name__ == "__main__":
    context.arch = 'amd64'
    
    assembly_source = shellcraft.cat('/etc/passwd', 1)

    with open("demo.pdf", "wb") as f:
        f.write(asm(assembly_source))