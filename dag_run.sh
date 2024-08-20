#! /bin/bash


#----- THIS ONLY NEEDS TO BE RUN ONCE --------------------
#cd ~/ndnSIM/scenario
#export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig
#export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
#./waf clean
#CXXFLAGS="-std=c++17" ./waf configure --debug
#---------------------------------------------------------

#rm logit.txt

clear
#./waf clean

LOGS=CustomAppConsumer
LOGS=${LOGS}:CustomAppConsumer2
LOGS=${LOGS}:CustomAppProducer
LOGS=${LOGS}:DagForwarderApp
LOGS=${LOGS}:ndn.App
LOGS=${LOGS}:DagOrchestratorA_App
LOGS=${LOGS}:DagServiceA_App
LOGS=${LOGS}:DagOrchestratorB_App
LOGS=${LOGS}:DagServiceB_App
LOGS=${LOGS}:ndn-cxx.nfd.Forwarder


# 4 DAG
#NS_LOG=${LOGS} ./waf --run=ndn-cabeee-4dag-orchestratorA |& tee log.txt
#NS_LOG=${LOGS} ./waf --run=ndn-cabeee-4dag-orchestratorB |& tee log.txt
NS_LOG=${LOGS} ./waf --run=ndn-cabeee-4dag |& tee log.txt

# 20 Linear
#NS_LOG=${LOGS} ./waf --run=ndn-cabeee-20node-linear-orchestratorA |& tee log.txt
#NS_LOG=${LOGS} ./waf --run=ndn-cabeee-20node-linear-orchestratorB |& tee log.txt
#NS_LOG=${LOGS} ./waf --run=ndn-cabeee-20node-linear |& tee log.txt


# 20 Parallel
#NS_LOG=${LOGS} ./waf --run=ndn-cabeee-20node-parallel-orchestratorA |& tee log.txt
#NS_LOG=${LOGS} ./waf --run=ndn-cabeee-20node-parallel-orchestratorB |& tee log.txt
#NS_LOG=${LOGS} ./waf --run=ndn-cabeee-20node-parallel |& tee log.txt


# 20 Sensor (Parallel)
#NS_LOG=${LOGS} ./waf --run=ndn-cabeee-20sensor-orchestratorA |& tee log.txt
#NS_LOG=${LOGS} ./waf --run=ndn-cabeee-20sensor-orchestratorB |& tee log.txt
#NS_LOG=${LOGS} ./waf --run=ndn-cabeee-20sensor |& tee log.txt


# 8 DAG
#NS_LOG=${LOGS} ./waf --run=ndn-cabeee-8dag-orchestratorA |& tee log.txt
#NS_LOG=${LOGS} ./waf --run=ndn-cabeee-8dag-orchestratorB |& tee log.txt
#NS_LOG=${LOGS} ./waf --run=ndn-cabeee-8dag |& tee log.txt


# 8 DAG w/ caching
#NS_LOG=${LOGS} ./waf --run=ndn-cabeee-8dag-caching-orchestratorA |& tee log.txt
#NS_LOG=${LOGS} ./waf --run=ndn-cabeee-8dag-caching-orchestratorB |& tee log.txt
#NS_LOG=${LOGS} ./waf --run=ndn-cabeee-8dag-caching |& tee log.txt




#NS_LOG=${LOGS} ./waf --run=ndn-cabeee-8dag-reuse |& tee log.txt


python process_nfd_logs.py
