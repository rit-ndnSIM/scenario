#! /bin/bash

cd ~
if [ ! -d "ndnSIM" ]; then
  mkdir ndnSIM
fi
cd ndnSIM

git clone git@github.com:rit-ndnSIM/ns-3-dev.git ns-3
git clone git@github.com:rit-ndnSIM/pybindgen.git pybindgen
git clone --recursive git@github.com:rit-ndnSIM/ndnSIM.git ns-3/src/ndnSIM

git clone git@github.com:rit-ndnSIM/scenario.git scenario

cd ns-3
./waf configure -d debug --enable-examples --disable-python
./waf
sudo ./waf install

# now we can run scenarios
cd ../scenario
#./dag_run.sh



