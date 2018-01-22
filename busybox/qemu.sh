qemu-system-i386 \
	-kernel output/images/bzImage \
	-drive format=raw,file=$PWD/output/images/rootfs.ext2,index=0,media=disk \
	-append "root=/dev/sda rw" \
	-redir tcp:2222::22



