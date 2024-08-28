#! /bin/bash

INSTALL_DIR=~
#INSTALL_DIR=/opt/ndn/

cd $INSTALL_DIR
if [ ! -d "ndnSIM" ]; then
  mkdir ndnSIM
fi
cd ndnSIM

git clone git@github.com:rit-ndnSIM/ns-3-dev.git ns-3
git clone git@github.com:rit-ndnSIM/pybindgen.git pybindgen
git clone --recursive git@github.com:rit-ndnSIM/ndnSIM.git ns-3/src/ndnSIM

git clone git@github.com:rit-ndnSIM/scenario.git scenario

cd ns-3
#./waf configure -d debug --enable-examples --disable-python
./waf configure -d optimized --enable-examples --disable-python
./waf
sudo ./waf install

# now we can run scenarios
cd ../scenario
export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
CXXFLAGS="-std=c++17" ./waf configure --debug

#./dag_run.sh



