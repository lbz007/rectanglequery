FROM centos:7.9.2009

MAINTAINER zenghui@niiceda.com

ENV LOGIN_USR=root
ENV LOGIN_PWD=Passw0rd

RUN echo "Configure source.list" &&\
  yum -y install wget curl vim &&\
  cd /etc/yum.repos.d &&\
  mkdir bak && mv CentOS*.repo bak/ &&\
  wget -O aliyun.repo http://mirrors.aliyun.com/repo/Centos-7.repo &&\
  yum makecache &&\
  yum -y install epel-release &&\
  yum -y install centos-release-scl centos-release-scl-rh scl-utils-build

RUN echo " Install OpenSSH" &&\
  yum -y install openssh-clients openssh-server openssl &&\
  yum clean all && rm -rf /var/cache/yum/* &&\
  sed -i 's/#UseDNS yes/UseDNS no/g' /etc/ssh/sshd_config &&\
  echo "#!/bin/bash" > /entrypoint.sh &&\
  echo "/usr/sbin/sshd -D" >> /entrypoint.sh &&\
  echo "${LOGIN_USR}:${LOGIN_PWD}" | chpasswd &&\
  ssh-keygen -q -t rsa -b 2048 -f /etc/ssh/ssh_host_rsa_key -N '' &&\
  ssh-keygen -q -t ecdsa -f /etc/ssh/ssh_host_ecdsa_key -N '' &&\
  ssh-keygen -t dsa -f /etc/ssh/ssh_host_ed25519_key  -N ''
  
RUN echo "Install Compiler Environment" &&\
  yum-config-manager --enable rhel-server-rhscl-7-rpms &&\
  yum -y install devtoolset-9-gcc devtoolset-9-gcc-c++ devtoolset-9-gcc-gfortran \
    devtoolset-9-binutils devtoolset-9-binutils-devel \
    devtoolset-9-elfutils devtoolset-9-elfutils-devel \
    devtoolset-9-libstdc++-devel \
    devtoolset-9-strace devtoolset-9-gdb \
    devtoolset-9-valgrind devtoolset-9-valgrind-devel \
    devtoolset-9-make devtoolset-9-perftools &&\
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
    qt5* rh-python36* &&\
  yum clean all && rm -rf /var/cache/yum/*

RUN echo "Build boost" &&\
  cd /tmp &&\
  wget https://boostorg.jfrog.io/artifactory/main/release/1.72.0/source/boost_1_72_0.tar.gz &&\
  tar xvf boost_1_72_0.tar.gz  &&\
  cd boost_1_72_0/ &&\
  source /opt/rh/devtoolset-9/enable &&\
  export  CC=$(which gcc) &&\
  export CXX=$(which g++) &&\
  ./bootstrap.sh &&\
  ./b2 -q --without-python &&\
  ./b2 install -q --without-python &&\
  echo "Build tcl/tk" &&\
  cd /tmp && wget https://downloads.sourceforge.net/tcl/tcl8.6.9-src.tar.gz &&\
  tar xvf tcl8.6.9-src.tar.gz &&\
  cd tcl8.6.9/unix/ &&\
  ./configure &&\
  make && make install &&\
  alternatives --install /usr/bin/tclsh tclsh /usr/local/bin/tclsh8.6 100 &&\
  cd /tmp && wget https://downloads.sourceforge.net/tcl/tk8.6.9-src.tar.gz &&\
  tar xvf tk8.6.9-src.tar.gz &&\
  cd tk8.6.9/unix/ &&\
  ./configure &&\
  make && make install &&\
  alternatives --install /usr/bin/wish wish /usr/local/bin/wish8.6 100 &&\
  echo "Build gperftools" &&\
  cd /tmp &&\
  wget https://github.com/gperftools/gperftools/releases/download/gperftools-2.9.1/gperftools-2.9.1.tar.gz &&\
  tar xvf gperftools-2.9.1.tar.gz &&\
  cd gperftools-2.9.1/ &&\
  ./configure &&\
  make && make install &&\
  echo "source /opt/rh/devtoolset-9/enable" >> ~/.bashrc &&\
  echo "source /opt/rh/rh-python36/enable" >> ~/.bashrc &&\
  echo "export  CC=$(which gcc)" >> ~/.bashrc &&\
  echo "export CXX=$(which g++)" >> ~/.bashrc &&\
  echo "export TCL_LIBRARY=/usr/local/lib/tcl8.6" >> ~/.bashrc &&\
  echo "export LD_LIBRARY_PATH=/usr/local/lib:/lib64:\${LD_LIBRARY_PATH}" >> ~/.bashrc &&\
  rm -rf /tmp/*

# Inform which port could be opened
EXPOSE 22

# Exec configuration to container
CMD sh /entrypoint.sh