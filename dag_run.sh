#! /bin/bash


#----- THIS ONLY NEEDS TO BE RUN ONCE --------------------
#cd ~/ndnSIM/scenario
#export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig
#export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
#./waf clean
#CXXFLAGS="-std=c++17" ./waf configure --debug
#---------------------------------------------------------


clear
./waf clean

#NS_LOG=CustomAppConsumer:CustomAppProducer:DagForwarderApp:ndn.App ./waf --run=ndn-cabeee-3node-apps

NS_LOG=CustomAppConsumer:CustomAppConsumer2:CustomAppProducer:DagForwarderApp:ndn.App ./waf --run=ndn-cabeee-3node-apps_reuse


#NS_LOG=CustomAppConsumer:CustomAppProducer:DagOrchestratorB_App:DagServiceB_App:ndn.App ./waf --run=ndn-cabeee-3node-apps-orchestratorB
#NS_LOG=CustomAppConsumer:CustomAppProducer:DagOrchestratorA_App:DagServiceA_App:ndn.App ./waf --run=ndn-cabeee-3node-apps-orchestratorA


