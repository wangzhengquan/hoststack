mkdir ./rootfs2
cd rootfs2 && mkdir -p  bin  dev/pts dev/shm etc  home  lib  lib64  mnt  opt  proc  root  run  sbin  sys  tmp  usr  var && cd ..

mkdir ./dff2
# sudo mount -t aufs -o dirs=./diff2=rw none ./rootfs2
sudo mount -t aufs -o dirs=/bin=ro none ./rootfs2/bin
sudo mount -t aufs -o dirs=/etc=ro none ./rootfs2/etc
sudo mount -t aufs -o dirs=/lib=ro none ./rootfs2/lib
sudo mount -t aufs -o dirs=/lib64=ro none ./rootfs2/lib64
sudo mount -t aufs -o dirs=/sbin=ro none ./rootfs2/sbin
sudo mount -t aufs -o dirs=/usr=ro none ./rootfs2/usr






sudo mount -t aufs -o dirs=./change:/bin:/dev:/etc:/lib:/lib64:/opt:/root:/run:/sbin:/sys:/tmp:/usr:/var none ./rootfs
sudo mount -t aufs -o dirs=./change:/bin:/dev:/etc:/lib:/lib64:/opt:/root:/run:/sbin:/sys none ./rootfs

sudo mount -t aufs -o dirs=./change:/bin:/dev:/etc:/lib:/lib64:/opt:/root:/run:/sbin:/sys none ./rootfs

sudo mount -t aufs -o dirs=./change:/ none ./rootfs

bin  dev  etc  home  lib  lib64  lost+found  media  mnt  opt  proc  root  run  sbin  srv  sys  tmp  usr  var


mount -t aufs -o dirs=./test=rw:./fruits=ro:./vegetables=ro none ./mnt