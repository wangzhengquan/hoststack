sudo ./KUCKER-1.0.1-Linux.sh  --prefix=/usr/local

sudo cp etc/kuckerd.service /etc/systemd/system/backerd.service
sudo systemctl daemon-reload
sudo systemctl enable  backerd.service 
sudo  systemctl restart backerd.service 
systemctl list-unit-files --all | grep backerd
systemctl status backerd.service
