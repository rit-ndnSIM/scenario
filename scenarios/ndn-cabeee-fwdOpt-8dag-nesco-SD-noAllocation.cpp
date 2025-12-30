/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2011-2015  Regents of the University of California.
 *
 * This file is part of ndnSIM. See AUTHORS for complete list of ndnSIM authors and
 * contributors.
 *
 * ndnSIM is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * ndnSIM is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * ndnSIM, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 **/

// ndn-custom-apps.cpp

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ndnSIM-module.h"
#include "ns3/string.h"

#define PREFIX "/nesco"

namespace ns3 {

/**
*       /------\ Fapp     ----------------
*  node0|sensor|----------| Producer APP | Sensor Data
*       \------/          ----------------
*         ^ F1
*         |
*         |
*         v F2
*       /-------\ Fapp    ---------------------
*  node1| rtr-1 |---------| DAG Forwarder APP | Service 2 & 6
*       \-------/         ---------------------
*         ^ F3
*         |
*         |
*         v F4    Fapp    ---------------------
*       /-------\ --------| DAG Forwarder APP | Service 3 & 7
*  node2| rtr-2 | Fapp    |-------------------|
*       \-------/ --------| DAG Forwarder APP | Service 4 & 8
*         ^ F5            ---------------------
*         |
*         |
*         v F6
*       /-------\ Fapp    ---------------------
*  node3| rtr-3 |---------| DAG Forwarder APP | Service 1 & 5
*       \-------/         ---------------------
*         ^ F7
*         |
*         |
*         v F8
*       /--------\ 
*  node4|  orch  |
*       \--------/
*         ^ F9
*     0ms |
*         v FA
*       /--------\ Fapp   ----------------
*  node5|  user  |--------| Consumer APP |
*       \--------/        ----------------
* 
*     NS_LOG=CustomAppConsumer:CustomAppProducer:DagForwarderApp ./waf --run=ndn-cabeee-8dag
*/
int
main(int argc, char* argv[])
{
  // Read optional command-line parameters (e.g., enable visualizer with ./waf --run=<> --visualize
  CommandLine cmd;
  cmd.Parse(argc, argv);

  // Creating nodes
  AnnotatedTopologyReader topologyReader("", 1);
  //topologyReader.SetFileName("topologies/topo-cabeee-3node-slow.txt");
  topologyReader.SetFileName("topologies/topo-cabeee-3node.txt");
  topologyReader.Read();




  // Install NDN stack on all nodes
  ndn::StackHelper ndnHelper;
  ndnHelper.setPolicy("nfd::cs::lru");
  //ndnHelper.setCsSize(100);
  //ndnHelper.setCsSize(1);
  ndnHelper.SetDefaultRoutes(true);

  //ndnHelper.InstallAll();

  // Getting containers for the nodes
  Ptr<Node> producer = Names::Find<Node>("sensor");
  Ptr<Node> router1 = Names::Find<Node>("rtr-1");
  Ptr<Node> router2 = Names::Find<Node>("rtr-2");
  Ptr<Node> router3 = Names::Find<Node>("rtr-3");
  //Ptr<Node> orchestrator = Names::Find<Node>("orch");
  Ptr<Node> consumer = Names::Find<Node>("user");

  ndnHelper.setCsSize(0); // content store size (0 disables it)
  ndnHelper.Install(producer);

  ndnHelper.setCsSize(0); // content store size (0 disables it)
  ndnHelper.Install(router1);
  ndnHelper.Install(router2);
  ndnHelper.Install(router3);

  //ndnHelper.setCsSize(0); // content store size (0 disables it)
  //ndnHelper.Install(orchestrator);

  ndnHelper.setCsSize(0); // content store size (0 disables it)
  ndnHelper.Install(consumer);



  std::string Prefix(PREFIX);

  // Choosing forwarding strategy
  //ndn::StrategyChoiceHelper::InstallAll("/prefix", "/localhost/nfd/strategy/best-route");
  //ndn::StrategyChoiceHelper::InstallAll("/prefix", "/localhost/nfd/strategy/multicast");

  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/service4", "/localhost/nfd/strategy/best-route");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/service3", "/localhost/nfd/strategy/best-route");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/service2", "/localhost/nfd/strategy/best-route");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/service1", "/localhost/nfd/strategy/best-route");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/service8", "/localhost/nfd/strategy/best-route");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/service7", "/localhost/nfd/strategy/best-route");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/service6", "/localhost/nfd/strategy/best-route");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/service5", "/localhost/nfd/strategy/best-route");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/sensor", "/localhost/nfd/strategy/best-route");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceDiscovery", "/localhost/nfd/strategy/best-route");

  // Installing global routing interface on all nodes
  ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
  ndnGlobalRoutingHelper.InstallAll();


 


  // Installing applications

  // Custom App for Sensor(Producer)
  ndn::AppHelper sensorApp("CustomAppProducer");
  sensorApp.SetPrefix(Prefix);

  // Custom App for routers
  ndn::AppHelper routerApp("DagForwarderApp");
  routerApp.SetPrefix(Prefix);

  // Custom App for ServiceDiscovery
  ndn::AppHelper serviceDiscoveryApp("DagServiceDiscoveryApp");
  serviceDiscoveryApp.SetPrefix(Prefix);


  // Now install the service app AND the service discovery app (each service needs SD)
  // and register all service prefix names to their locations using the global routing helper
  sensorApp.SetAttribute("Service", StringValue("sensor"));
  serviceDiscoveryApp.SetAttribute("Service", StringValue("sensor"));

  sensorApp.Install(producer).Start(Seconds(0));
  serviceDiscoveryApp.Install(producer).Start(Seconds(0));
  ndnGlobalRoutingHelper.AddOrigins(Prefix + "/sensor", producer);
  ndnGlobalRoutingHelper.AddOrigins(Prefix + "/serviceDiscovery/sensor", producer);

  sensorApp.Install(router1).Start(Seconds(0));
  serviceDiscoveryApp.Install(router1).Start(Seconds(0));
  ndnGlobalRoutingHelper.AddOrigins(Prefix + "/sensor", router1);
  ndnGlobalRoutingHelper.AddOrigins(Prefix + "/serviceDiscovery/sensor", router1);

  sensorApp.Install(router3).Start(Seconds(0));
  serviceDiscoveryApp.Install(router3).Start(Seconds(0));
  ndnGlobalRoutingHelper.AddOrigins(Prefix + "/sensor", router3);
  ndnGlobalRoutingHelper.AddOrigins(Prefix + "/serviceDiscovery/sensor", router3);




  routerApp.SetAttribute("Service", StringValue("service1"));
  serviceDiscoveryApp.SetAttribute("Service", StringValue("service1"));

  routerApp.Install(producer).Start(Seconds(0));
  serviceDiscoveryApp.Install(producer).Start(Seconds(0));
  ndnGlobalRoutingHelper.AddOrigins(Prefix + "/service1", producer);
  ndnGlobalRoutingHelper.AddOrigins(Prefix + "/serviceDiscovery/service1", producer);

  routerApp.Install(router3).Start(Seconds(0));
  serviceDiscoveryApp.Install(router3).Start(Seconds(0));
  ndnGlobalRoutingHelper.AddOrigins(Prefix + "/service1", router3);
  ndnGlobalRoutingHelper.AddOrigins(Prefix + "/serviceDiscovery/service1", router3);




  routerApp.SetAttribute("Service", StringValue("service2"));
  serviceDiscoveryApp.SetAttribute("Service", StringValue("service2"));

  routerApp.Install(router1).Start(Seconds(0));
  serviceDiscoveryApp.Install(router1).Start(Seconds(0));
  ndnGlobalRoutingHelper.AddOrigins(Prefix + "/service2", router1);
  ndnGlobalRoutingHelper.AddOrigins(Prefix + "/serviceDiscovery/service2", router1);

  routerApp.Install(router3).Start(Seconds(0));
  serviceDiscoveryApp.Install(router3).Start(Seconds(0));
  ndnGlobalRoutingHelper.AddOrigins(Prefix + "/service2", router3);
  ndnGlobalRoutingHelper.AddOrigins(Prefix + "/serviceDiscovery/service2", router3);




  routerApp.SetAttribute("Service", StringValue("service3"));
  serviceDiscoveryApp.SetAttribute("Service", StringValue("service3"));

  routerApp.Install(router1).Start(Seconds(0));
  serviceDiscoveryApp.Install(router1).Start(Seconds(0));
  ndnGlobalRoutingHelper.AddOrigins(Prefix + "/service3", router1);
  ndnGlobalRoutingHelper.AddOrigins(Prefix + "/serviceDiscovery/service3", router1);

  routerApp.Install(router2).Start(Seconds(0));
  serviceDiscoveryApp.Install(router2).Start(Seconds(0));
  ndnGlobalRoutingHelper.AddOrigins(Prefix + "/service3", router2);
  ndnGlobalRoutingHelper.AddOrigins(Prefix + "/serviceDiscovery/service3", router2);



  
  routerApp.SetAttribute("Service", StringValue("service4"));
  serviceDiscoveryApp.SetAttribute("Service", StringValue("service4"));

  routerApp.Install(router2).Start(Seconds(0));
  serviceDiscoveryApp.Install(router2).Start(Seconds(0));
  ndnGlobalRoutingHelper.AddOrigins(Prefix + "/service4", router2);
  ndnGlobalRoutingHelper.AddOrigins(Prefix + "/serviceDiscovery/service4", router2);



  
  routerApp.SetAttribute("Service", StringValue("service5"));
  serviceDiscoveryApp.SetAttribute("Service", StringValue("service5"));

  routerApp.Install(router3).Start(Seconds(0));
  serviceDiscoveryApp.Install(router3).Start(Seconds(0));
  ndnGlobalRoutingHelper.AddOrigins(Prefix + "/service5", router3);
  ndnGlobalRoutingHelper.AddOrigins(Prefix + "/serviceDiscovery/service5", router3);




  routerApp.SetAttribute("Service", StringValue("service6"));
  serviceDiscoveryApp.SetAttribute("Service", StringValue("service6"));

  routerApp.Install(router1).Start(Seconds(0));
  serviceDiscoveryApp.Install(router1).Start(Seconds(0));
  ndnGlobalRoutingHelper.AddOrigins(Prefix + "/service6", router1);
  ndnGlobalRoutingHelper.AddOrigins(Prefix + "/serviceDiscovery/service6", router1);

  routerApp.Install(producer).Start(Seconds(0));
  serviceDiscoveryApp.Install(producer).Start(Seconds(0));
  ndnGlobalRoutingHelper.AddOrigins(Prefix + "/service6", producer);
  ndnGlobalRoutingHelper.AddOrigins(Prefix + "/serviceDiscovery/service6", producer);




  routerApp.SetAttribute("Service", StringValue("service7"));
  serviceDiscoveryApp.SetAttribute("Service", StringValue("service7"));

  routerApp.Install(router1).Start(Seconds(0));
  serviceDiscoveryApp.Install(router1).Start(Seconds(0));
  ndnGlobalRoutingHelper.AddOrigins(Prefix + "/service7", router1);
  ndnGlobalRoutingHelper.AddOrigins(Prefix + "/serviceDiscovery/service7", router1);




  routerApp.SetAttribute("Service", StringValue("service8"));
  serviceDiscoveryApp.SetAttribute("Service", StringValue("service8"));

  routerApp.Install(router1).Start(Seconds(0));
  serviceDiscoveryApp.Install(router1).Start(Seconds(0));
  ndnGlobalRoutingHelper.AddOrigins(Prefix + "/service8", router1);
  ndnGlobalRoutingHelper.AddOrigins(Prefix + "/serviceDiscovery/service8", router1);

  routerApp.Install(router2).Start(Seconds(0));
  serviceDiscoveryApp.Install(router2).Start(Seconds(0));
  ndnGlobalRoutingHelper.AddOrigins(Prefix + "/service8", router2);
  ndnGlobalRoutingHelper.AddOrigins(Prefix + "/serviceDiscovery/service8", router2);


 

  // Custom App for User(Consumer)
  ndn::AppHelper userApp("CustomAppConsumerServiceDiscovery");
  userApp.SetPrefix(Prefix); // this is only a placeholder. The app will read the JSON workflow, and figure out which service is "last"

  userApp.SetAttribute("Service", StringValue("consumer"));
  userApp.SetAttribute("Workflow", StringValue("workflows/8dag.json"));
  userApp.SetAttribute("Orchestrate", UintegerValue(0));
  userApp.SetAttribute("FwdOpt", UintegerValue(1));
  userApp.SetAttribute("SDstartTime", TimeValue(Seconds(1)));
  userApp.SetAttribute("WFstartTime", TimeValue(Seconds(2)));
  userApp.Install(consumer).Start(Seconds(0));




  // Calculate and install FIBs
  ndn::GlobalRoutingHelper::CalculateRoutes();




  Simulator::Stop(Seconds(20.1)); // pick a large value, the consumer will end the simulation as soon as the workflow data packet is received.

  //ndn::L3RateTracer::InstallAll("rate-trace_cabeee-8dag.txt", Seconds(0.1));
  //ndn::CsTracer::InstallAll("cs-trace_cabeee-8dag.txt", Seconds(0.1));

  Simulator::Run();
  Simulator::Destroy();

  return 0;
}

} // namespace ns3

int
main(int argc, char* argv[])
{
  return ns3::main(argc, argv);
}
