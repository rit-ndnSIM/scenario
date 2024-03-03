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
*     NS_LOG=CustomAppConsumer:CustomAppProducer:DagForwarderApp ./waf --run=ndn-cabeee-8dag-orchestratorB
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
  Ptr<Node> orchestrator = Names::Find<Node>("orch");
  Ptr<Node> consumer = Names::Find<Node>("user");

  ndnHelper.setCsSize(0); // enable/disable content store by setting size
  ndnHelper.Install(producer);

  //ndnHelper.setCsSize(0); // enable/disable content store by setting size
  ndnHelper.setCsSize(0); // enable/disable content store by setting size
  ndnHelper.Install(router1);

  //ndnHelper.setCsSize(0); // enable/disable content store by setting size
  ndnHelper.setCsSize(0); // enable/disable content store by setting size
  ndnHelper.Install(router2);

  //ndnHelper.setCsSize(0); // enable/disable content store by setting size
  ndnHelper.setCsSize(0); // enable/disable content store by setting size
  ndnHelper.Install(router3);

  ndnHelper.setCsSize(0); // enable/disable content store by setting size
  ndnHelper.Install(orchestrator);

  ndnHelper.setCsSize(0); // disable content store
  ndnHelper.Install(consumer);




  // Choosing forwarding strategy
  //ndn::StrategyChoiceHelper::InstallAll("/prefix", "/localhost/nfd/strategy/best-route");
  //ndn::StrategyChoiceHelper::InstallAll("/prefix", "/localhost/nfd/strategy/multicast");
  //ndn::StrategyChoiceHelper::InstallAll("/service4", "/localhost/nfd/strategy/best-route");
  //ndn::StrategyChoiceHelper::InstallAll("/service3", "/localhost/nfd/strategy/best-route");
  //ndn::StrategyChoiceHelper::InstallAll("/service2", "/localhost/nfd/strategy/best-route");
  //ndn::StrategyChoiceHelper::InstallAll("/service1", "/localhost/nfd/strategy/best-route");
  //ndn::StrategyChoiceHelper::InstallAll("/sensor", "/localhost/nfd/strategy/best-route");
  ndn::StrategyChoiceHelper::InstallAll("/service4", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll("/service3", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll("/service2", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll("/service1", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll("/service8", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll("/service7", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll("/service6", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll("/service5", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll("/sensor", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll("/serviceOrchestration", "/localhost/nfd/strategy/multicast");

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
  sensorApp.SetPrefix("/sensor");
  sensorApp.Install(producer).Start(Seconds(0));

  // Custom App for routers
  ndn::AppHelper serviceApp("DagServiceB_App");

  serviceApp.SetPrefix("/service1");
  serviceApp.SetAttribute("Service", StringValue("service1"));
  auto s1 = serviceApp.Install(router3);
  auto s1b = serviceApp.Install(router3);
  s1.Start(Seconds(0));
  s1.Stop(Seconds(4));
  s1b.Start(Seconds(4));
  serviceApp.SetPrefix("/service5");
  serviceApp.SetAttribute("Service", StringValue("service5"));
  serviceApp.Install(router3).Start(Seconds(0));

  serviceApp.SetPrefix("/service2");
  serviceApp.SetAttribute("Service", StringValue("service2"));
  auto s2 = serviceApp.Install(router1);
  auto s2b = serviceApp.Install(router1);
  s2.Start(Seconds(0));
  s2.Stop(Seconds(4));
  s2b.Start(Seconds(4));
  serviceApp.SetPrefix("/service6");
  serviceApp.SetAttribute("Service", StringValue("service6"));
  serviceApp.Install(router1).Start(Seconds(0));

  serviceApp.SetPrefix("/service3");
  serviceApp.SetAttribute("Service", StringValue("service3"));
  auto s3 = serviceApp.Install(router2);
  auto s3b = serviceApp.Install(router2);
  s3.Start(Seconds(0));
  s3.Stop(Seconds(4));
  s3b.Start(Seconds(4));
  serviceApp.SetPrefix("/service7");
  serviceApp.SetAttribute("Service", StringValue("service7"));
  serviceApp.Install(router2).Start(Seconds(0));

  serviceApp.SetPrefix("/service4");
  serviceApp.SetAttribute("Service", StringValue("service4"));
  auto s4 = serviceApp.Install(router2);
  auto s4b = serviceApp.Install(router2);
  s4.Start(Seconds(0));
  s4.Stop(Seconds(4));
  s4b.Start(Seconds(4));
  serviceApp.SetPrefix("/service8");
  serviceApp.SetAttribute("Service", StringValue("service8"));
  serviceApp.Install(router2).Start(Seconds(0));


  // FOR FIRST USER
  ndn::AppHelper orchestratorApp("DagOrchestratorB_App");
  orchestratorApp.SetPrefix("/serviceOrchestration");
  orchestratorApp.SetAttribute("Service", StringValue("serviceOrchestration"));
  //orchestratorApp.SetAttribute("Results", StringValue("/sensor/service1/service2/service3/service4"));
  auto orchApp = orchestratorApp.Install(orchestrator);
  orchApp.Start(Seconds(0));
  orchApp.Stop(Seconds(4));
  
  // Custom App for User(Consumer)
  ndn::AppHelper userApp("CustomAppConsumer");
  userApp.SetPrefix("/consumer"); // this is only a placeholder. The app will read the JSON workflow, and figure out which service is "last"
  userApp.SetAttribute("Workflow", StringValue("workflows/4dag.json"));
  userApp.SetAttribute("Orchestrate", UintegerValue(2)); // This enables the "orchestrator" by having the consumer set the head service to /serviceOrchestration/dag
  userApp.Install(consumer).Start(Seconds(0));


  // FOR SECOND USER - we need a new Orchestrator app, because of the tracker data structures. OR we could simply reset the tracker once we generate the last data packet to the consumer.
  ndn::AppHelper orchestratorApp2("DagOrchestratorB_App");
  orchestratorApp2.SetPrefix("/serviceOrchestration");
  orchestratorApp2.SetAttribute("Service", StringValue("serviceOrchestration"));
  //orchestratorApp2.SetAttribute("Results", StringValue("/sensor/service1/service2/service3/service4"));
  orchestratorApp2.Install(orchestrator).Start(Seconds(4));

  // Custom App for User2(Consumer2)
  ndn::AppHelper userApp2("CustomAppConsumer2");
  userApp2.SetPrefix("/consumer2"); // this is only a placeholder. The app will read the JSON workflow, and figure out which service is "last"
  userApp2.SetAttribute("Workflow", StringValue("workflows/8dag.json"));
  userApp2.SetAttribute("Orchestrate", UintegerValue(2)); // This enables the "orchestrator" by having the consumer set the head service to /serviceOrchestration/dag
  userApp2.Install(consumer).Start(Seconds(4));


/*
  // default consumer app
  ndn::AppHelper consumerHelper("ns3::ndn::ConsumerCbr");
  // Consumer will request /prefix/0, /prefix/1, ...
  consumerHelper.SetPrefix("/prefix");
  consumerHelper.SetAttribute("Frequency", StringValue("1")); // 1 interests per second
  auto apps = consumerHelper.Install(consumer);
  apps.Start(Seconds(1.0));
  apps.Stop(Seconds(4.0)); // stop the consumer app at 3 seconds mark

  // cabeee - install another consumer app to see if items are retrieved from the content store.
  ndn::AppHelper consumerHelper2("ns3::ndn::ConsumerCbr");
  // Consumer will request /prefix/0, /prefix/1, ...
  consumerHelper2.SetPrefix("/prefix");
  //consumerHelper2.SetAttribute("Frequency", StringValue("0.5")); // 0.5 interests per second
  consumerHelper2.SetAttribute("Frequency", StringValue("1")); // 1 interests per second
  auto apps2 = consumerHelper2.Install(consumer);
  apps2.Start(Seconds(5.0));
  apps2.Stop(Seconds(7.0)); // stop the consumer app at 6 seconds mark

  // default producer app
  ndn::AppHelper producerHelper("ns3::ndn::Producer");
  // Producer will reply to all requests starting with /prefix
  producerHelper.SetPrefix("/prefix");
  producerHelper.SetAttribute("PayloadSize", StringValue("1024"));
  producerHelper.Install(producer);
*/







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

  ndn::L3RateTracer::InstallAll("rate-trace_cabeee-8dag-orchestratorB.txt", Seconds(0.1));
  ndn::CsTracer::InstallAll("cs-trace_cabeee-8dag-orchestratorB.txt", Seconds(0.1));

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
