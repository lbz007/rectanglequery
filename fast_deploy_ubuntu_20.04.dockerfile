FROM ubuntu:20.04

MAINTAINER zenghui@niiceda.com

ENV DEBIAN_FRONTEND=noninteractive
ENV LOGIN_USR=root
ENV LOGIN_PWD=Passw0rd

RUN echo "Configure source.list" &&\
  cd /etc/apt &&\
  mv sources.list sources.list.bak &&\
  echo '\
deb http://mirrors.163.com/ubuntu/ focal main restricted universe multiverse \n\
deb http://mirrors.163.com/ubuntu/ focal-security main restricted universe multiverse \n\
deb http://mirrors.163.com/ubuntu/ focal-updates main restricted universe multiverse \n\
deb http://mirrors.163.com/ubuntu/ focal-backports main restricted universe multiverse \n'\
> /etc/apt/sources.list

RUN echo "Configure SSH Service" &&\
  cat /etc/apt/sources.list &&\
  apt update &&\
  apt -y install openssh-server &&\
  mkdir /run/sshd &&\
  sed -i "s/^#PasswordAuth/PasswordAuth/g" /etc/ssh/sshd_config &&\
  sed -i "s/.*PermitRootLogin.*/PermitRootLogin yes/g" /etc/ssh/sshd_config &&\
  echo "${LOGIN_USR}:${LOGIN_PWD}" | chpasswd &&\
  echo "#!/bin/bash" > /entrypoint.sh &&\
  echo "[ ! -d /run/sshd ] && mkdir /var/run" >> /entrypoint.sh &&\
  echo "/usr/sbin/sshd -D" >> /entrypoint.sh
  
RUN echo "Install Compiler Environment" &&\
  apt -y install gcc g++ git wget cmake vim libboost1.71-all-dev tcl-dev tk tk-dev flex bison build-essential libfontconfig1 mesa-common-dev libglu1-mesa-dev libbison-dev doxygen graphviz ghostscript lz4 liblz4-dev zstd libzstd-dev zip libzip-dev libreadline-dev libunwind-dev qt5-default &&\
  cd /tmp &&\
  wget https://github.com/gperftools/gperftools/releases/download/gperftools-2.9.1/gperftools-2.9.1.tar.gz &&\
  tar xvf gperftools-2.9.1.tar.gz &&\
  cd gperftools-2.9.1/ &&\
  ./configure &&\
  make && make install &&\
  rm -rf /tmp/gperftools-2.9.1/ &&\
  echo "Configure Environment" &&\
  echo "export LD_LIBRARY_PATH=/usr/lib/x86_64-linux-gnu:/usr/local/lib:\${LD_LIBRARY_PATH}" >> ~/.bashrc

# Inform which port could be opened
EXPOSE 22

# Exec configuration to container
CMD sh /entrypoint.sh
