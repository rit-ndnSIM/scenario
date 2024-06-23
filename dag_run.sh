#! /bin/bash


#----- THIS ONLY NEEDS TO BE RUN ONCE --------------------
#cd ~/ndnSIM/scenario
#export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig
#export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
#./waf clean
#CXXFLAGS="-std=c++17" ./waf configure --debug
#---------------------------------------------------------


clear
#./waf clean

# 4 DAG
#NS_LOG=CustomAppConsumer:CustomAppProducer:DagOrchestratorA_App:DagServiceA_App:ndn.App ./waf --run=ndn-cabeee-4dag-orchestratorA
#NS_LOG=CustomAppConsumer:CustomAppProducer:DagOrchestratorB_App:DagServiceB_App:ndn.App ./waf --run=ndn-cabeee-4dag-orchestratorB
#NS_LOG=CustomAppConsumer:CustomAppProducer:DagForwarderApp:ndn.App ./waf --run=ndn-cabeee-4dag

# 20 Linear
#NS_LOG=CustomAppConsumer:CustomAppProducer:DagOrchestratorA_App:DagServiceA_App:ndn.App ./waf --run=ndn-cabeee-20node-linear-orchestratorA
#NS_LOG=CustomAppConsumer:CustomAppProducer:DagOrchestratorB_App:DagServiceB_App:ndn.App ./waf --run=ndn-cabeee-20node-linear-orchestratorB
#NS_LOG=CustomAppConsumer:CustomAppProducer:DagForwarderApp:ndn.App ./waf --run=ndn-cabeee-20node-linear


# 20 Parallel
#NS_LOG=CustomAppConsumer:CustomAppProducer:DagOrchestratorA_App:DagServiceA_App:ndn.App ./waf --run=ndn-cabeee-20node-parallel-orchestratorA
#NS_LOG=CustomAppConsumer:CustomAppProducer:DagOrchestratorB_App:DagServiceB_App:ndn.App ./waf --run=ndn-cabeee-20node-parallel-orchestratorB
#NS_LOG=CustomAppConsumer:CustomAppProducer:DagForwarderApp:ndn.App ./waf --run=ndn-cabeee-20node-parallel


# 20 Sensor (Parallel)
#NS_LOG=CustomAppConsumer:CustomAppProducer:DagOrchestratorA_App:DagServiceA_App:ndn.App ./waf --run=ndn-cabeee-20sensor-orchestratorA
#NS_LOG=CustomAppConsumer:CustomAppProducer:DagOrchestratorB_App:DagServiceB_App:ndn.App ./waf --run=ndn-cabeee-20sensor-orchestratorB
#NS_LOG=CustomAppConsumer:CustomAppProducer:DagForwarderApp:ndn.App ./waf --run=ndn-cabeee-20sensor


# 8 DAG
#NS_LOG=CustomAppConsumer:CustomAppConsumer2:CustomAppProducer:DagOrchestratorA_App:DagServiceA_App:ndn.App ./waf --run=ndn-cabeee-8dag-orchestratorA
#NS_LOG=CustomAppConsumer:CustomAppConsumer2:CustomAppProducer:DagOrchestratorB_App:DagServiceB_App:ndn.App ./waf --run=ndn-cabeee-8dag-orchestratorB
#NS_LOG=CustomAppConsumer:CustomAppConsumer2:CustomAppProducer:DagForwarderApp:ndn.App ./waf --run=ndn-cabeee-8dag


# 8 DAG w/ caching
#NS_LOG=CustomAppConsumer:CustomAppConsumer2:CustomAppProducer:DagOrchestratorA_App:DagServiceA_App:ndn.App ./waf --run=ndn-cabeee-8dag-caching-orchestratorA
#NS_LOG=CustomAppConsumer:CustomAppConsumer2:CustomAppProducer:DagOrchestratorB_App:DagServiceB_App:ndn.App ./waf --run=ndn-cabeee-8dag-caching-orchestratorB
NS_LOG=CustomAppConsumer:CustomAppConsumer2:CustomAppProducer:DagForwarderApp:ndn.App ./waf --run=ndn-cabeee-8dag-caching






#NS_LOG=CustomAppConsumer:CustomAppConsumer2:CustomAppProducer:DagForwarderApp:ndn.App ./waf --run=ndn-cabeee-8dag-reuse

