# Config Guide For Ubuntu 20.04 

## 1.Config source.list 

```
cd /etc/apt
mv sources.list sources.list.bak
cat <<EOF> /etc/apt/sources.list
deb http://mirrors.163.com/ubuntu/ focal main restricted universe multiverse
deb http://mirrors.163.com/ubuntu/ focal-security main restricted universe multiverse
deb http://mirrors.163.com/ubuntu/ focal-updates main restricted universe multiverse
deb http://mirrors.163.com/ubuntu/ focal-backports main restricted universe multiverse
EOF
apt update 
```

## 2.Install Build Requirement

```
apt -y install gcc g++ git wget cmake libboost1.71-all-dev tcl-dev tk tk-dev flex bison build-essential libfontconfig1 mesa-common-dev libglu1-mesa-dev libbison-dev doxygen graphviz ghostscript lz4 liblz4-dev zstd libzstd-dev zip libzip-dev libreadline-dev libunwind-dev 
```

## 3.Build Gperftools

```
wget https://github.com/gperftools/gperftools/releases/download/gperftools-2.9.1/gperftools-2.9.1.tar.gz
tar xvf gperftools-2.9.1.tar.gz
cd gperftools-2.9.1/
./configure
make && make install
```

## 4.Install Qt5

You can install Qt5 in 2 ways:

4.1.Install Qt5 via apt

```
apt -y install qt5-default 
# For bash
export LD_LIBRARY_PATH=/usr/lib/x86_64-linux-gnu:${LD_LIBRARY_PATH}
# For csh
setenv LD_LIBRARY_PATH /usr/lib/x86_64-linux-gnu ${LD_LIBRARY_PATH}
```

4.2.Install the official installation package manually, this method requires an additional installation path to be specified

```
wget -O /tmp/qt.run https://download.qt.io/archive/qt/5.9/5.9.7/qt-opensource-linux-x64-5.9.7.run
chmod +x /tmp/qt.run
/tmp/qt.run
```

4.2.1 Add QT5 to PATH For Bash

```
export PATH=<QT5 install path, Must Point to gcc_64/bin>:$PATH
export LD_LIBRARY_PATH=<QT5 install path, Must Point to gcc_64/lib>:${LD_LIBRARY_PATH}
```

4.2.2  Add QT5 to PATH For CSH

```
set path=(. <QT5 install path, Must Point to gcc_64/lib> $path)
setenv LD_LIBRARY_PATH <QT5 install path, Must Point to gcc_64/lib> ${LD_LIBRARY_PATH}
```
