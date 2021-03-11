[ -d package ] || mkdir package
cp  build/KUCKER-1.0.1-Linux.sh build/install.sh package
tar -cJpf ./backer_install.tar.xz ./package