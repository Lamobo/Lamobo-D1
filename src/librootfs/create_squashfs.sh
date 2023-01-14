#!/bin/sh

echo Creating SquashFS image...

# prepare rootfs/etc contains the essential files
cd rootfs
rm -rf _etc
mv etc _etc
mkdir -p etc/init.d
cp _etc/init.d/rcS etc/init.d/
cp _etc/fstab      etc/
cp _etc/inittab    etc/
cd ..

# packing
rm -f root.sqsh4
mksquashfs rootfs root.sqsh4 -noappend -comp xz -no-progress -e _etc

# recover rootfs/etc
cd rootfs
rm -rf etc
mv _etc etc
cd ..
