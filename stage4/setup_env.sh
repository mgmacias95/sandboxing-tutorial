sudo mv stage4 /etc/apparmor.d/stage4

sudo apparmor_parser -r /etc/apparmor.d/stage4

docker run --rm -it \
  --cap-add=SYS_ADMIN \
  --security-opt apparmor=stage4 \
  -v "$(pwd):/data" \
  ubuntu bash