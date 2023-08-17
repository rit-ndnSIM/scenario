Prerequisites
=============

Custom version of NS-3 and specified version of ndnSIM needs to be installed.

The code should also work with the latest version of ndnSIM, but it is not guaranteed.

    mkdir ndnSIM
    cd ndnSIM

    git clone -b ndnSIM-ns-3.35 https://github.com/named-data-ndnSIM/ns-3-dev.git ns-3
        commit 1b3bab9b6dba5f1e616c0b286eb889e9ce3d5d59 (HEAD -> ndnSIM-ns-3.35, origin/ndnSIM-ns-3.35, origin/HEAD)

    git clone https://github.com/named-data-ndnSIM/pybindgen.git pybindgen
        commit a619e22a1492e586f42a688b015c551ea1facd7b (HEAD -> master, origin/master, origin/HEAD)
        
    git clone --recursive https://github.com/named-data-ndnSIM/ndnSIM ns-3/src/ndnSIM
        commit 90d50396654dabad54b6979f2dc8fa929ade544c (HEAD -> master, tag: ndnSIM-2.9-NFD-22.02, origin/master, origin/HEAD)

    git clone https://github.com/named-data-ndnSIM/scenario-template.git scenario
        commit 8c91e46d206229db9377ed48d2f56e47c8c46725 (HEAD -> master, origin/master, origin/HEAD)

    # Build and install NS-3 and ndnSIM
    cd ns-3
    ./waf configure -d optimized
    ./waf
    sudo ./waf install

    # When using Linux, run
    # sudo ldconfig

    # When using Freebsd, run
    # sudo ldconfig -a

After which you can proceed to compile and run the code

    cd ..
    git clone https://github.com/named-data-ndnSIM/scenario-template.git scenario
    cd scenario

    CXXFLAGS="-std=c++17" ./waf configure
    ./waf --run <scenario-name>


For more information how to install NS-3 and ndnSIM, please refer to http://ndnsim.net website.

Compiling
=========

To configure in optimized mode without logging **(default)**:

    ./waf configure

To configure in optimized mode with scenario logging enabled (logging in NS-3 and ndnSIM modules will still be disabled,
but you can see output from NS_LOG* calls from your scenarios and extensions):

    ./waf configure --logging

To configure in debug mode with all logging enabled

    ./waf configure --debug

If you have installed NS-3 in a non-standard location, you may need to set up ``PKG_CONFIG_PATH`` variable.

Running
=======

Normally, you can run scenarios either directly

    ./build/<scenario_name>

or using waf

    ./waf --run <scenario_name>

If NS-3 is installed in a non-standard location, on some platforms (e.g., Linux) you need to specify ``LD_LIBRARY_PATH`` variable:

    LD_LIBRARY_PATH=/usr/local/lib ./build/<scenario_name>

or

    LD_LIBRARY_PATH=/usr/local/lib ./waf --run <scenario_name>

To run scenario using debugger, use the following command:

    gdb --args ./build/<scenario_name>


Running with visualizer
-----------------------

There are several tricks to run scenarios in visualizer.  Before you can do it, you need to set up environment variables for python to find visualizer module.  The easiest way to do it using the following commands:

    cd ns-dev/ns-3
    ./waf shell

After these command, you will have complete environment to run the vizualizer.

The following will run scenario with visualizer:

    ./waf --run <scenario_name> --vis

or

    PKG_LIBRARY_PATH=/usr/local/lib ./waf --run <scenario_name> --vis

If you want to request automatic node placement, set up additional environment variable:

    NS_VIS_ASSIGN=1 ./waf --run <scenario_name> --vis

or

    PKG_LIBRARY_PATH=/usr/local/lib NS_VIS_ASSIGN=1 ./waf --run <scenario_name> --vis

Available simulations
=====================

<Scenario Name>
---------------

Description
