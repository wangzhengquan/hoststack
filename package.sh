[ -d package ] || mkdir package
cp  build/HOSTSTACK-1.0.1-Linux.sh build/install.sh package
tar -cpf ./hoststack_install.tar ./package
