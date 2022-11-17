#!bin/bash
git pull
make clean
make all
scp encoder root@10.10.7.1:~/project532
# scp decoder root@10.10.7.1:~/project532
read tmp
./client -s t=5 -f LittlePrince.txt --i 10.10.7.1
