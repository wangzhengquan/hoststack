kucker_repo="/data/kucker"
test -d ${kucker_repo} || mkdir -p ${kucker_repo}
test -d ${kucker_repo}/aufs || mkdir  ${kucker_repo}/aufs
test -d ${kucker_repo}/aufs/mnt || mkdir  ${kucker_repo}/aufs/mnt
test -d ${kucker_repo}/aufs/diff || mkdir  ${kucker_repo}/aufs/diff
test -d ${kucker_repo}/aufs/layers || mkdir  ${kucker_repo}/aufs/layers
test -d ${kucker_repo}/containers || mkdir ${kucker_repo}/containers

container_id="2"

exit 0

test -d ${kucker_repo}/containers/${container_id} || mkdir ${kucker_repo}/containers/${container_id}
test -d ${kucker_repo}/aufs/diff/${container_id} || mkdir  ${kucker_repo}/aufs/diff/${container_id}
test -d ${kucker_repo}/aufs/mnt/${container_id} || mkdir  ${kucker_repo}/aufs/mnt/${container_id}

cd ${kucker_repo}/aufs/mnt/${container_id} && mkdir -p  bin  dev/pts dev/shm etc  home  lib  lib64  mnt  opt  proc  root  run  sbin  sys  tmp  usr  var

sudo mount -t aufs -o dirs=${kucker_repo}/aufs/diff/${container_id}=rw:/bin=ro none ${kucker_repo}/aufs/mnt/${container_id}/bin
sudo mount -t aufs -o dirs=${kucker_repo}/aufs/diff/${container_id}=rw:/etc=ro none ${kucker_repo}/aufs/mnt/${container_id}/etc
sudo mount -t aufs -o dirs=${kucker_repo}/aufs/diff/${container_id}=rw:/lib=ro none ${kucker_repo}/aufs/mnt/${container_id}/lib
sudo mount -t aufs -o dirs=${kucker_repo}/aufs/diff/${container_id}=rw:/lib64=ro none ${kucker_repo}/aufs/mnt/${container_id}/lib64
sudo mount -t aufs -o dirs=${kucker_repo}/aufs/diff/${container_id}=rw:/sbin=ro none ${kucker_repo}/aufs/mnt/${container_id}/sbin
sudo mount -t aufs -o dirs=${kucker_repo}/aufs/diff/${container_id}=rw:/usr=ro none ${kucker_repo}/aufs/mnt/${container_id}/usr
sudo mount -t aufs -o dirs=${kucker_repo}/aufs/diff/${container_id}=rw:/var=ro none ${kucker_repo}/aufs/mnt/${container_id}/var
sudo mount -t aufs -o dirs=${kucker_repo}/aufs/diff/${container_id}=rw:/etc=ro none ${kucker_repo}/aufs/mnt/${container_id}/etc
sudo mount -t aufs -o dirs=${kucker_repo}/aufs/diff/${container_id}=rw:/run=ro none ${kucker_repo}/aufs/mnt/${container_id}/run
sudo mount -t aufs -o dirs=${kucker_repo}/aufs/diff/${container_id}=rw:/opt=ro none ${kucker_repo}/aufs/mnt/${container_id}/opt


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