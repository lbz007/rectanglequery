# OpenEDI

#### Description
An open-source infrastructure for VLSI design automation tool set.  

#### Software Architecture
Key components include:
- Database - OpenEDI database, core module.
- Parser - parse standard data format and import data to OpenEDI databade.
- CommandManager - manage commands.
- MessageManager - manage messages.
- IOManager - High-performance file IO.
- GUI - Graphic user interface

#### Building Dependency (Recommend)

- [GCC 6.5.0](http://ftp.tsukuba.wide.ad.jp/software/gcc/releases/gcc-6.5.0/gcc-6.5.0.tar.gz)
- [Cmake 3.12.4](https://cmake.org/files/v3.12/cmake-3.12.4-Linux-x86_64.tar.gz)
- [Qt 5.9.7](http://download.qt.io/archive/qt/5.9/5.9.7/qt-opensource-linux-x64-5.9.7.run)
- Python 3.6
- [Google test](https://gitee.com/mirrors/googletest)(Integrated as a submodule)
- [Pybind11](https://gitee.com/mirrors/pybind11)(Integrated as a submodule)
- Boost 1.55 +
- Tcl/Tk 8.6.9+
- Flex 2.5.37+
- Bison 3.0.0+
- [Gperftools 2.7.90](https://github.com/gperftools/gperftools/releases/download/gperftools-2.7.90/gperftools-2.7.90.tar.gz)
- [LZ4 1.9.2](https://github.com/lz4/lz4)

####  Deploy development environment

##### 1.Manual configuration guide

- [Ubuntu 20.04](config_guide_ubuntu20.04.md)
- [CentOS 7.9](config_guide_centos7.9.md)

##### 2.Dockerfile(Recommend)

- Download the dockerfile [Ubuntu 20.04](fast_deploy_ubuntu_20.04.dockerfile)   **or**  [CentOS 7.9](fast_deploy_centos_7.9.dockerfile), then build the docker image

```
docker build -t openedi -f fast_deploy_ubuntu_20.04.dockerfile .
or
docker build -t openedi -f fast_deploy_centos_7.9.dockerfile .

docker run --name openedi -it -p 2222:22 -d openedi
```

- Login to the container, check out and build the code.
```
docker exec -it openedi bash
or
ssh root@localhost -p 2222
```

- Default username:root, password:Passw0rd

##### 3.Virtual Machine Image

- Download and unzip CentOS7Dev.zip, then use the virtual machine software(Support VMware Workstations/VMware Player/Virtualbox) to import CentOS7Dev.ovf and start it.

```
Download url: https://pan.baidu.com/s/1JnxVadThOJL3K2jsscRaGA
Verification code: moqr
```

- Operating System Username:root, Password:toor.

- Before checkout openedi code, Please source /opt/devtoolset/devtoolset.bash first.

- The default configuration of the virtual machine is 4 cores and 8GB memory. Under default configuration, if multi-threaded compilation is enabled and the number of threads is greater than 8, insufficient resource may lead to build failure. Please manually modify the virtual machine hardware configuration of the virtual machine as you need.

#### Installation

1.  Clone the repository with submodules.
~~~~~~~~~~~~~~~~~~~~~
git clone --recursive https://gitee.com/niiceda/open-edi.git
~~~~~~~~~~~~~~~~~~~~~
Alternatively, you can clone the repository and the submodules in two steps.
~~~~~~~~~~~~~~~~~~~~~
git clone https://gitee.com/niiceda/open-edi.git
cd open-edi
git submodule update --init
~~~~~~~~~~~~~~~~~~~~~

2.  Build the program.
~~~~~~~~~~~~~~~~~~~~~
mkdir build   # assume you are inside the root directory of the repository
cd build
cmake ..      # you may add -DCMAKE_INSTALL_PREFIX=your/install/path to specify the build installation location, which by default is <repository>/
make          # you may add -j to turn on parallel building
make test     # optional step to run unittests
make docs     # optional step to generate HTML documentations
make install  # install 'openeda' binary and all related files in build installation location
~~~~~~~~~~~~~~~~~~~~~

#### Instructions

1.  Once installed (after make install), you can find the 'openeda' binary under 'your/install/path/bin/' directory. Run the binary <your/install/path/bin/>openeda in your design location, and try the openeda TCL commands:
~~~~~~~~~~~~~~~~~~~~~
OpenEDA> read_lef test.lef
OpenEDA> read_verilog test.v
OpenEDA> read_def test.def
OpenEDA> read_timing_library test.lib
~~~~~~~~~~~~~~~~~~~~~

2.  You can also find some test cases in demo directory, and run them with the binary just built.:
~~~~~~~~~~~~~~~~~~~~~
cd demo
tar xvf demo_case.tar.xz
cd demo_case
run.sh <your/install/path/bin/openeda>
~~~~~~~~~~~~~~~~~~~~~

#### Contribution

Welcome to contribute to our open source EDA ecosystems!
1.  Fork the repository.
2.  Create Feat_xxx branch.
3.  Commit your code.
4.  Create Pull Request to repository 'OpenEDI', branch 'dev-contrinutor'.


#### Features

1.  Open source parser to support lef, def, v, lib format design files
2.  Open source database API

#### Milestones

1.  2020/11/03 Open source EDI project went ONLINE on Gitee [https://gitee.com/niiceda/open-edi](https://gitee.com/niiceda/open-edi)
2.  2020/11/05 OpenEDI project became the recommended open source project.
3.  2020/12/01 OpenEDI project achieved the most valuable open source project [GVP](https://gitee.com/gvp)
