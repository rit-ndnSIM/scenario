#!/bin/bash


#NS_LOG=CustomAppConsumer:CustomAppProducer:DagForwarderApp ./waf --run=ndn-cabeee-cacheAdv
#NS_LOG=CustomAppConsumer:CustomAppProducer:DagForwarderApp:CustomAppCsUpdater ./waf --run=ndn-cabeee-cacheAdv
#NS_LOG=CustomAppConsumer:CustomAppProducer:DagForwarderApp:CustomAppCsUpdater:ndn-cxx.nfd.Forwarder ./waf --run=ndn-cabeee-cacheAdv
#NS_LOG=CustomAppConsumer:CustomAppProducer:CustomAppCsUpdater ./waf --run=ndn-cabeee-cacheAdv
#NS_LOG=CustomAppConsumer:CustomAppProducer:DagForwarderApp:CustomAppCsUpdater ./waf --run=ndn-cabeee-cacheAdv
#NS_LOG=CustomAppConsumer:CustomAppProducer:DagForwarderApp:CustomAppCsUpdater:ndn-cxx.nfd.Forwarder ./waf --run=ndn-cabeee-cacheAdv
NS_LOG=CustomAppConsumer:CustomAppProducer:DagForwarderApp:CustomAppCsUpdater:ndn-cxx.nfd.Forwarder ./waf --run=ndn-cabeee-cacheAdv |& tee csUpdate.txt
