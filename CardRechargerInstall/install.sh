#! /bin/bash

LocalPath=/sdcard/CardRechargerInstall
InstallationPath=/home/AIROB
Log=${LocalPath}/InstallationLog

rm -rf ${Log}
echo "Start the installation of the CardRecharger Program..." | tee ${Log}

cd ${LocalPath}
cat ./conf/RootPassword | passwd root | tee -a ${Log}

mkdir -p ${InstallationPath} | tee -a ${Log}
mkdir -p ${InstallationPath}/Advertisement | tee -a ${Log}
mkdir -p ${InstallationPath}/LogFile | tee -a ${Log}
echo "Build the directory for the program done!" | tee -a ${Log}

cd ${LocalPath}/Library
cp -rv ./CURL/* /usr/local | tee -a ${Log}
cp -rv ./QREncode/* /usr/local | tee -a ${Log}
cp -rv ./NTP/* /usr | tee -a ${Log}
echo "Setup the libraries done!" | tee -a ${Log}

cd ${LocalPath}/conf
cp -v ./ntp.conf /etc | tee -a ${Log}
mkdir -p /etc/crontabs | tee -a ${Log}
cp -v ./root.crontabs /etc/crontabs | tee -a ${Log}
echo "Setup service configuration files done!" | tee -a ${Log}

cd ${LocalPath}/script
mkdir -p ${InstallationPath}/script | tee -a ${Log}
cp -v ./reboot.sh ${InstallationPath}/script | tee -a ${Log}
cp -v ./NTPrecalibrate.sh ${InstallationPath}/script | tee -a ${Log}
cp -v ./test.sh ${InstallationPath}/script | tee -a ${Log}
echo "Setup scripts done!" | tee -a ${Log}

cd ${LocalPath}
cp -rv ./Program ${InstallationPath} | tee -a ${Log}
cd ${InstallationPath}
ln -sf ./Program/CardRecharger ./CardRecharger
ln -sf ./Program/DevInfo.ini ./DevInfo.ini
echo "Copy the program done!" | tee -a ${Log}

cd /etc/init.d
sed -i -e '
104s/qtopia /qt4/
105,106s/echo/#echo/
107a\
mkdir -p /var/spool/cron/crontabs
107a\
cp -p /etc/crontabs/root.crontabs /var/spool/cron/crontabs/root
107a\
chmod 777 /var/spool/cron/crontabs/root
107a\
crond
103a\
rm -rf /home/AIROB/Advertisement/*' ./rcS
echo "Modify the /etc/init.d/rcS done!" | tee -a ${Log}

cd /bin
sed -i -e '
6c\
cd /home/AIROB
7c\
./CardRecharger -qws' ./qt4
echo "Modify the /bin/qt4 done!" | tee -a ${Log}

cp -v ${Log} ${InstallationPath}/LogFile/

reboot











