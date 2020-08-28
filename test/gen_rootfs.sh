image_base_dir="/data/kucker"
test -d ${image_base_dir} || mkdir ${image_base_dir}
test -d ${image_base_dir}/aufs || mkdir  ${image_base_dir}/aufs
test -d ${image_base_dir}/containers || mkdir ${image_base_dir}/containers

image_id="1"

sudo mount -t aufs -o dirs=${image_base_dir}/aufs/${image_id}/diff=rw:/var=ro none ${image_base_dir}/aufs/${image_id}/mnt/var
exit 0

test -d ${image_base_dir}/containers/${image_id} || mkdir ${image_base_dir}/containers/${image_id}
test -d ${image_base_dir}/aufs/${image_id} || mkdir  ${image_base_dir}/aufs/${image_id}

test -d ${image_base_dir}/aufs/${image_id}/diff || mkdir  ${image_base_dir}/aufs/${image_id}/diff
test -d ${image_base_dir}/aufs/${image_id}/mnt || mkdir  ${image_base_dir}/aufs/${image_id}/mnt

cd ${image_base_dir}/aufs/${image_id}/mnt && mkdir -p  bin  dev/pts dev/shm etc  home  lib  lib64  mnt  opt  proc  root  run  sbin  sys  tmp  usr  var

sudo mount -t aufs -o dirs=${image_base_dir}/aufs/${image_id}/diff=rw:/bin=ro none ${image_base_dir}/aufs/${image_id}/mnt/bin
sudo mount -t aufs -o dirs=${image_base_dir}/aufs/${image_id}/diff=rw:/etc=ro none ${image_base_dir}/aufs/${image_id}/mnt/etc
sudo mount -t aufs -o dirs=${image_base_dir}/aufs/${image_id}/diff=rw:/lib=ro none ${image_base_dir}/aufs/${image_id}/mnt/lib
sudo mount -t aufs -o dirs=${image_base_dir}/aufs/${image_id}/diff=rw:/lib64=ro none ${image_base_dir}/aufs/${image_id}/mnt/lib64
sudo mount -t aufs -o dirs=${image_base_dir}/aufs/${image_id}/diff=rw:/sbin=ro none ${image_base_dir}/aufs/${image_id}/mnt/sbin
sudo mount -t aufs -o dirs=${image_base_dir}/aufs/${image_id}/diff=rw:/usr=ro none ${image_base_dir}/aufs/${image_id}/mnt/usr
sudo mount -t aufs -o dirs=${image_base_dir}/aufs/${image_id}/diff=rw:/var=ro none ${image_base_dir}/aufs/${image_id}/mnt/var
exit 0

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