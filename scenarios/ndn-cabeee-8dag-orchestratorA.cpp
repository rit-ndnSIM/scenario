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

#define PREFIX "/orchA"

namespace ns3 {

/**
*
*       /------\ Fapp     ----------------
*  node0|sensor|----------| Producer APP | Sensor Data
*       \------/          ----------------
*         ^ F1
*         |
*         |
*         v F2
*       /-------\ Fapp    ---------------
*  node1| rtr-1 |---------| Service_A APP | Service 2 & 6
*       \-------/         ---------------
*         ^ F3
*         |
*         |
*         v F4    Fapp    -----------------
*       /-------\ --------| Service_A APP | Service 3 & 7
*  node2| rtr-2 | Fapp    |---------------|
*       \-------/ --------| Service_A APP | Service 4 & 8
*         ^ F5            -----------------
*         |
*         |
*         v F6
*       /-------\ Fapp    -----------------
*  node3| rtr-3 |---------| Service_A APP | Service 1 & 5
*       \-------/         -----------------
*         ^ F7
*         |
*         |
*         v F8
*       /--------\ Fapp   ----------------------
*  node4|  orch  |--------| Orchestrator_A APP | ServiceOrchestration
*       \--------/        ----------------------
*         ^ F9
*     0ms |
*         v FA
*       /--------\ Fapp   ----------------
*  node5|  user  |--------| Consumer APP |
*       \--------/        ----------------
*
*
* 
*     NS_LOG=CustomAppConsumer:CustomAppProducer:DagForwarderApp ./waf --run=ndn-cabeee-8dag-orchestratorA
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

  ndnHelper.setCsSize(0); // enable/disable content store by setting size
  ndnHelper.Install(producer);

  ndnHelper.setCsSize(0); // enable/disable content store by setting size
  ndnHelper.Install(router1);

  ndnHelper.setCsSize(0); // enable/disable content store by setting size
  ndnHelper.Install(router2);

  ndnHelper.setCsSize(0); // enable/disable content store by setting size
  ndnHelper.Install(router3);

  //ndnHelper.setCsSize(0); // enable/disable content store by setting size
  //ndnHelper.Install(orchestrator);

  ndnHelper.setCsSize(0); // disable content store
  ndnHelper.Install(consumer);



  std::string Prefix(PREFIX);

  // Choosing forwarding strategy
  //ndn::StrategyChoiceHelper::InstallAll("/prefix", "/localhost/nfd/strategy/best-route");
  //ndn::StrategyChoiceHelper::InstallAll("/prefix", "/localhost/nfd/strategy/multicast");
  //ndn::StrategyChoiceHelper::InstallAll(Prefix + "/service4", "/localhost/nfd/strategy/best-route");
  //ndn::StrategyChoiceHelper::InstallAll(Prefix + "/service3", "/localhost/nfd/strategy/best-route");
  //ndn::StrategyChoiceHelper::InstallAll(Prefix + "/service2", "/localhost/nfd/strategy/best-route");
  //ndn::StrategyChoiceHelper::InstallAll(Prefix + "/service1", "/localhost/nfd/strategy/best-route");
  //ndn::StrategyChoiceHelper::InstallAll(Prefix + "/sensor", "/localhost/nfd/strategy/best-route");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/service4", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/service3", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/service2", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/service1", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/service8", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/service7", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/service6", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/service5", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/sensor", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceOrchestration", "/localhost/nfd/strategy/multicast");

  // Installing global routing interface on all nodes
  //ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
  //ndnGlobalRoutingHelper.InstallAll();

  // Getting containers for the consumer/producer
  //Ptr<Node> consumer = Names::Find<Node>("user");
  //Ptr<Node> producer = Names::Find<Node>("sensor");
  //Ptr<Node> producer = Names::Find<Node>("rtr-1");

 


  // Installing applications

  // Custom App for Sensor(Producer)
  ndn::AppHelper sensorApp("CustomAppProducer");
  sensorApp.SetPrefix(Prefix);
  sensorApp.SetAttribute("Service", StringValue("sensor"));
  sensorApp.Install(producer).Start(Seconds(0));

  // Custom App for routers
  ndn::AppHelper serviceApp("DagServiceA_App");

  serviceApp.SetPrefix(Prefix);
  serviceApp.SetAttribute("Service", StringValue("service1"));
  auto s1 = serviceApp.Install(router3);
  s1.Start(Seconds(0));
  //s1.Stop(Seconds(4));

  serviceApp.SetPrefix(Prefix);
  serviceApp.SetAttribute("Service", StringValue("service5"));
  auto s5 = serviceApp.Install(router3);
  s5.Start(Seconds(0));

  serviceApp.SetPrefix(Prefix);
  serviceApp.SetAttribute("Service", StringValue("service2"));
  auto s2 = serviceApp.Install(router1);
  s2.Start(Seconds(0));
  //s2.Stop(Seconds(4));

  serviceApp.SetPrefix(Prefix);
  serviceApp.SetAttribute("Service", StringValue("service6"));
  auto s6 = serviceApp.Install(router1);
  s6.Start(Seconds(0));

  serviceApp.SetPrefix(Prefix);
  serviceApp.SetAttribute("Service", StringValue("service3"));
  auto s3 = serviceApp.Install(router2);
  s3.Start(Seconds(0));
  //s3.Stop(Seconds(4));

  serviceApp.SetPrefix(Prefix);
  serviceApp.SetAttribute("Service", StringValue("service7"));
  auto s7 = serviceApp.Install(router2);
  s7.Start(Seconds(0));

  serviceApp.SetPrefix(Prefix);
  serviceApp.SetAttribute("Service", StringValue("service4"));
  auto s4 = serviceApp.Install(router2);
  s4.Start(Seconds(0));
  //s4.Stop(Seconds(4));

  serviceApp.SetPrefix(Prefix);
  serviceApp.SetAttribute("Service", StringValue("service8"));
  auto s8 = serviceApp.Install(router2);
  s8.Start(Seconds(0));


  /* in the non-caching scenario, we don't need to run the 4dag workflow first.
  // FOR FIRST USER
  ndn::AppHelper orchestratorApp("DagOrchestratorA_App");
  orchestratorApp.SetPrefix("/serviceOrchestration");
  orchestratorApp.SetAttribute("Service", StringValue("serviceOrchestration"));
  auto orchApp = orchestratorApp.Install(orchestrator);
  orchApp.Start(Seconds(0));
  //orchApp.Stop(Seconds(4));

  // Custom App for User2(Consumer2)
  ndn::AppHelper userApp2("CustomAppConsumer2");
  userApp2.SetPrefix("/consumer2");
  userApp2.SetAttribute("Workflow", StringValue("workflows/4dag.json"));
  userApp2.SetAttribute("Orchestrate", UintegerValue(1)); // This enables the "orchestrator" by having the consumer set the head service to /serviceOrchestration
  userApp2.Install(consumer).Start(Seconds(0));
  */


  // FOR SECOND USER - we need a new Orchestrator app, because of the tracker data structures. OR we could simply reset the tracker once we generate the last data packet to the consumer.
  ndn::AppHelper orchestratorApp2("DagOrchestratorA_App");
  orchestratorApp2.SetPrefix(Prefix);
  orchestratorApp2.SetAttribute("Service", StringValue("serviceOrchestration"));
  //orchestratorApp2.Install(orchestrator).Start(Seconds(4));
  orchestratorApp2.Install(consumer).Start(Seconds(4));

  // Custom App for User1(Consumer1)
  ndn::AppHelper userApp("CustomAppConsumer");
  userApp.SetPrefix(Prefix);
  userApp.SetAttribute("Service", StringValue("consumer"));
  userApp.SetAttribute("Workflow", StringValue("workflows/8dag.json"));
  userApp.SetAttribute("Orchestrate", UintegerValue(1)); // This enables the "orchestrator" by having the consumer set the head service to /serviceOrchestration
  userApp.Install(consumer).Start(Seconds(4));




  // Calculate and install FIBs
  //ndn::GlobalRoutingHelper::CalculateRoutes();

  // Manually configure FIB routes
  //ndn::FibHelper::AddRoute(consumer, "/service4", router3, 1);
  //ndn::FibHelper::AddRoute(router3, "/service4", router2, 1);
  //ndn::FibHelper::AddRoute(router3, "/service2", router2, 1);
  //ndn::FibHelper::AddRoute(router2, "/service1", router1, 1);
  //ndn::FibHelper::AddRoute(router1, "/sensor", producer, 1);
  //ndn::FibHelper::AddRoute(router2, "/service3", router3, 1);

  // cabeee - it seems this still works without calculating/installing FIB and without manually configuring FIB routes
  // as long as interests always flow upstream and data downstream. If we want complex DAG workflows, we need to manually
  // set the routes, OR set the forwarding strategy to multi-cast with the appropriate prefixes




  Simulator::Stop(Seconds(8.0));

  ndn::L3RateTracer::InstallAll("rate-trace_cabeee-8dag-orchestratorA.txt", Seconds(0.01));
  ndn::CsTracer::InstallAll("cs-trace_cabeee-8dag-orchestratorA.txt", Seconds(0.01));

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
