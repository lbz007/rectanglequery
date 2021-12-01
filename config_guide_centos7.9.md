# Config Guide For CentOS 7.9

## 1.Config yum

```
yum -y install wget
cd /etc/yum.repos.d
mkdir bak && mv CentOS*.repo bak/
wget -O aliyun.repo http://mirrors.aliyun.com/repo/Centos-7.repo
yum makecache
yum -y install epel-release
yum -y install centos-release-scl centos-release-scl-rh scl-utils-build 
```

## 2.Install Build Requirement

```
yum-config-manager --enable rhel-server-rhscl-7-rpms
yum -y install devtoolset-9-gcc devtoolset-9-gcc-c++ devtoolset-9-gcc-gfortran \
    devtoolset-9-binutils devtoolset-9-binutils-devel \
    devtoolset-9-elfutils devtoolset-9-elfutils-devel \
    devtoolset-9-libstdc++-devel \
    devtoolset-9-strace devtoolset-9-gdb \
    devtoolset-9-valgrind devtoolset-9-valgrind-devel \
    devtoolset-9-make devtoolset-9-perftools
yum -y install cmake3 sciidoc docbook2X xmlto texinfo \
    perl-ExtUtils-MakeMaker sgml2xml autoconf openjade \
    expat-devel gettext-devel \
    curl curl-devel \
    bison bison-devel \
    flex flex-devel \
    openssl openssl-devel \
    zlib zlib-devel \
    bzip2 bzip2-devel \
    libicu libicu-devel \
    libzip libzip-devel \
    readline readline-devel \
    tcl tcl-devel \
    libX11 libX11-devel \
    zstd libzstd-devel \
    lz4 lz4-devel \
    qt5-default rh-python36* 

source /opt/rh/devtoolset-9/enable
export  CC=$(which gcc)
export CXX=$(which g++)
```

## 3.Build Boost

```
wget https://boostorg.jfrog.io/artifactory/main/release/1.72.0/source/boost_1_72_0.tar.bz2
tar xf boost_1_72_0.tar.bz2
cd boost_1_72_0/
./bootstrap.sh
./b2 -q --without-python
./b2 install -q --without-python
```

## 4.Build tcl/tk

```
wget https://downloads.sourceforge.net/tcl/tcl8.6.9-src.tar.gz
tar xvf tcl8.6.9-src.tar.gz
cd tcl8.6.9/unix/
./configure
make && make install
alternatives --install /usr/bin/tclsh tclsh /usr/local/bin/tclsh8.6 100

wget https://downloads.sourceforge.net/tcl/tk8.6.9-src.tar.gz
tar xvf tk8.6.9-src.tar.gz
cd tk8.6.9/unix/
./configure
make && make install
alternatives --install /usr/bin/wish wish /usr/local/bin/wish8.6 100
```

Before you launch openeda, You should setup system environment TCL_LIBRARY:
```
# For bash 
export TCL_LIBRARY=/usr/local/lib/tcl8.6
# For csh
setenv TCL_LIBRARY /usr/local/lib/tcl8.6
```

## 5.Build Gperftools

```
wget https://github.com/gperftools/gperftools/releases/download/gperftools-2.9.1/gperftools-2.9.1.tar.gz
tar xvf gperftools-2.9.1.tar.gz
cd gperftools-2.9.1/
./configure
make && make install
```

## 6.Install Qt5

You can install Qt5 in 2 ways:

### 6.1.Install Qt5 via apt

```
yum -y install qt5* 
# For bash
export LD_LIBRARY_PATH=/lib64:${LD_LIBRARY_PATH}
```

### 6.2.Install the official installation package manually

```
wget -O /tmp/qt.run https://download.qt.io/archive/qt/5.9/5.9.7/qt-opensource-linux-x64-5.9.7.run
chmod +x /tmp/qt.run
/tmp/qt.run
```

After install qt.run, you need to configure additional installation path to $PATH.

#### 6.2.1 Add QT5 to PATH For Bash

```
export PATH=<QT5 install path, Must Point to gcc_64/bin>:$PATH
export LD_LIBRARY_PATH=<QT5 install path, Must Point to gcc_64/lib>:${LD_LIBRARY_PATH}
```

## 7.Setup bash
```
cat <<EOF>> ~/.bashrc
source /opt/rh/devtoolset-9/enable
source /opt/rh/rh-python36/enable
export  CC=$(which gcc)
export CXX=$(which g++)
export TCL_LIBRARY=/usr/local/lib/tcl8.6
export LD_LIBRARY_PATH=/usr/local/lib:/lib64:\${LD_LIBRARY_PATH}
EOF
```
