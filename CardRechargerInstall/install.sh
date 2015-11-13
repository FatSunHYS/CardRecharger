#! /bin/bash

LocalPath=/sdcard/CardRechargerInstall
InstallationPath=/home/AIROB
Log=${LocalPath}/InstallationLog

rm -rf ${Log}
echo "Start the installation of the CardRecharger Program..." | tee ${Log}

cd ${LocalPath}
cat ./RootPassword.txt | passwd root | tee -a ${Log}

cd /home
mkdir AIROB | tee -a ${Log}
echo "Build the directory for the program done!" | tee -a ${Log}

cd ${LocalPath}
cp -rv ./CURL/* /usr/local | tee -a ${Log}
cp -rv ./QREncode/* /usr/local | tee -a ${Log}
cp -rv ./NTP/* /usr | tee -a ${Log}
cp -v ./ntp.conf /etc | tee -a ${Log}
echo "Setup the libraries done!" | tee -a ${Log}


cd ${LocalPath}
cp -rv ./Program ${InstallationPath} | tee -a ${Log}
cd ${InstallationPath}
ln -sf ./Program/CardRecharger ./CardRecharger
ln -sf ./Program/DevInfo.ini ./DevInfo.ini
echo "Copy the program done!" | tee -a ${Log}


cd /etc/init.d
sed -i -e '
104s/qtopia /qt4/
105,106s/echo/#echo/' ./rcS
echo "Modify the /etc/init.d/rcS done!" | tee -a ${Log}

cd /bin
sed -i -e '
6c\
cd /home/AIROB
7c\
./CardRecharger -qws' ./qt4
echo "Modify the /bin/qt4 done!" | tee -a ${Log}












