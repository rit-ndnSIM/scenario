#! /bin/bash

#NS_LOG=CustomAppConsumer:CustomAppProducer:DagForwarderApp:ndn.App ./waf --run=ndn-cabeee-3node-apps

NS_LOG=CustomAppConsumer:CustomAppConsumer2:CustomAppProducer:DagForwarderApp:ndn.App ./waf --run=ndn-cabeee-3node-apps_reuse


#NS_LOG=CustomAppConsumer:CustomAppProducer:DagOrchestratorB_App:DagServiceB_App:ndn.App ./waf --run=ndn-cabeee-3node-apps-orchestratorB
#NS_LOG=CustomAppConsumer:CustomAppProducer:DagOrchestratorA_App:DagServiceA_App:ndn.App ./waf --run=ndn-cabeee-3node-apps-orchestratorA


