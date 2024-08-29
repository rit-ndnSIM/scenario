#!/bin/bash

# used to setup the dev environment

set -eou pipefail

INSTALL_DIR="$HOME/ndnSIM"

mkdir -p "$INSTALL_DIR"

# cloning through ssh, so make sure your user has read/write access
git clone git@github.com:rit-ndnSIM/ns-3-dev.git "$INSTALL_DIR/ns-3"
git clone git@github.com:rit-ndnSIM/pybindgen.git "$INSTALL_DIR/pybindgen"
git clone --recursive git@github.com:rit-ndnSIM/ndnSIM.git "$INSTALL_DIR/ns-3/src/ndnSIM"

git clone git@github.com:rit-ndnSIM/scenario.git "$INSTALL_DIR/scenario"

cd "$INSTALL_DIR/ns-3"
./waf configure -d debug --enable-examples --disable-python
./waf
sudo ./waf install

# now we can run scenarios, if we want
cd "$INSTALL_DIR/scenario"
export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig
export LD_LIBRARY_PATH="/usr/local/lib:$LD_LIBRARY_PATH"
CXXFLAGS="-std=c++17" ./waf configure --debug

#./dag_run.sh
