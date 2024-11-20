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
*       /-------\        /--------\     /--------\ Fapp   ----------------
*  node0|sensor1|--------|sensor-n|-----|sensor20|--------| Producer APPs| Sensor Data
*       \-------/        \--------/     \--------/        ----------------
*         ^ F8             ^ F3            ^ F1
*         |                |               |
*         |                |               |
*         |                |               v F2
*         |                |             /-------\ Fapp   ---------------------
*         |                |       node20| rtr-20|--------| Service_A APP     | Service 20
*         |                |             \-------/        ---------------------
*         |                |               ^
*         |                v F6            |
*         |              /-------\ Fapp    |              ---------------------
*         |         noden| rtr-n |---------|--------------| Service_A APP     | Service n
*         |              \-------/         |              ---------------------
*         |                ^               |
*         v F6             |               | 
*       /-------\ Fapp     |               |              ---------------------
*  node1| rtr-1 |----------|---------------|--------------| Service_A APP     | Service 1
*       \-------/          |               |              ---------------------
*         ^ F7             |               |
*         |                |               |
*         |                |               |
*         v F8             v               v
*       /--------------------------------------\ Fapp     ---------------------
* node21|              rtr-21                  |----------| Service_A APP     | Service 21
*       \--------------------------------------/          ---------------------
*         ^ F7
*         |
*         v F6
*       /--------\ Fapp                                   ----------------------
* node22|  orch  |----------------------------------------| Orchestrator_A APP | ServiceOrchestration
*       \--------/                                        ----------------------
*         ^ F9
*     0ms |
*         v FA
*       /--------\ Fapp   ----------------
* node23|  user  |--------| Consumer APP |
*       \--------/        ----------------
* 
*     NS_LOG=CustomAppConsumer:CustomAppProducer:DagForwarderApp ./waf --run=ndn-cabeee-20sensor-orchestratorA
*/
int
main(int argc, char* argv[])
{
  // Read optional command-line parameters (e.g., enable visualizer with ./waf --run=<> --visualize
  CommandLine cmd;
  cmd.Parse(argc, argv);

  // Creating nodes
  AnnotatedTopologyReader topologyReader("", 1);
  topologyReader.SetFileName("topologies/topo-cabeee-20sensor.txt");
  topologyReader.Read();




  // Install NDN stack on all nodes
  ndn::StackHelper ndnHelper;
  ndnHelper.setPolicy("nfd::cs::lru");
  //ndnHelper.setCsSize(100);
  //ndnHelper.setCsSize(1);
  ndnHelper.SetDefaultRoutes(true);

  //ndnHelper.InstallAll();

  // Getting containers for the nodes
  Ptr<Node> producer1 = Names::Find<Node>("sensor-1");
  Ptr<Node> producer2 = Names::Find<Node>("sensor-2");
  Ptr<Node> producer3 = Names::Find<Node>("sensor-3");
  Ptr<Node> producer4 = Names::Find<Node>("sensor-4");
  Ptr<Node> producer5 = Names::Find<Node>("sensor-5");
  Ptr<Node> producer6 = Names::Find<Node>("sensor-6");
  Ptr<Node> producer7 = Names::Find<Node>("sensor-7");
  Ptr<Node> producer8 = Names::Find<Node>("sensor-8");
  Ptr<Node> producer9 = Names::Find<Node>("sensor-9");
  Ptr<Node> producer10 = Names::Find<Node>("sensor-10");
  Ptr<Node> producer11 = Names::Find<Node>("sensor-11");
  Ptr<Node> producer12 = Names::Find<Node>("sensor-12");
  Ptr<Node> producer13 = Names::Find<Node>("sensor-13");
  Ptr<Node> producer14 = Names::Find<Node>("sensor-14");
  Ptr<Node> producer15 = Names::Find<Node>("sensor-15");
  Ptr<Node> producer16 = Names::Find<Node>("sensor-16");
  Ptr<Node> producer17 = Names::Find<Node>("sensor-17");
  Ptr<Node> producer18 = Names::Find<Node>("sensor-18");
  Ptr<Node> producer19 = Names::Find<Node>("sensor-19");
  Ptr<Node> producer20 = Names::Find<Node>("sensor-20");
  Ptr<Node> router1 = Names::Find<Node>("rtr-1");
  Ptr<Node> router2 = Names::Find<Node>("rtr-2");
  Ptr<Node> router3 = Names::Find<Node>("rtr-3");
  Ptr<Node> router4 = Names::Find<Node>("rtr-4");
  Ptr<Node> router5 = Names::Find<Node>("rtr-5");
  Ptr<Node> router6 = Names::Find<Node>("rtr-6");
  Ptr<Node> router7 = Names::Find<Node>("rtr-7");
  Ptr<Node> router8 = Names::Find<Node>("rtr-8");
  Ptr<Node> router9 = Names::Find<Node>("rtr-9");
  Ptr<Node> router10 = Names::Find<Node>("rtr-10");
  Ptr<Node> router11 = Names::Find<Node>("rtr-11");
  Ptr<Node> router12 = Names::Find<Node>("rtr-12");
  Ptr<Node> router13 = Names::Find<Node>("rtr-13");
  Ptr<Node> router14 = Names::Find<Node>("rtr-14");
  Ptr<Node> router15 = Names::Find<Node>("rtr-15");
  Ptr<Node> router16 = Names::Find<Node>("rtr-16");
  Ptr<Node> router17 = Names::Find<Node>("rtr-17");
  Ptr<Node> router18 = Names::Find<Node>("rtr-18");
  Ptr<Node> router19 = Names::Find<Node>("rtr-19");
  Ptr<Node> router20 = Names::Find<Node>("rtr-20");
  Ptr<Node> router21 = Names::Find<Node>("rtr-21");
  //Ptr<Node> orchestrator = Names::Find<Node>("orch");
  Ptr<Node> consumer = Names::Find<Node>("user");

  ndnHelper.setCsSize(0); // enable/disable content store by setting size
  ndnHelper.Install(producer1);
  ndnHelper.Install(producer2);
  ndnHelper.Install(producer3);
  ndnHelper.Install(producer4);
  ndnHelper.Install(producer5);
  ndnHelper.Install(producer6);
  ndnHelper.Install(producer7);
  ndnHelper.Install(producer8);
  ndnHelper.Install(producer9);
  ndnHelper.Install(producer10);
  ndnHelper.Install(producer11);
  ndnHelper.Install(producer12);
  ndnHelper.Install(producer13);
  ndnHelper.Install(producer14);
  ndnHelper.Install(producer15);
  ndnHelper.Install(producer16);
  ndnHelper.Install(producer17);
  ndnHelper.Install(producer18);
  ndnHelper.Install(producer19);
  ndnHelper.Install(producer20);
  //ndnHelper.setCsSize(0); // enable/disable content store by setting size
  //ndnHelper.Install(orchestrator);
  ndnHelper.setCsSize(0); // disable content store
  ndnHelper.Install(consumer);

  ndnHelper.setCsSize(0); // enable/disable content store by setting size
  ndnHelper.Install(router1);
  ndnHelper.Install(router2);
  ndnHelper.Install(router3);
  ndnHelper.Install(router4);
  ndnHelper.Install(router5);
  ndnHelper.Install(router6);
  ndnHelper.Install(router7);
  ndnHelper.Install(router8);
  ndnHelper.Install(router9);
  ndnHelper.Install(router10);
  ndnHelper.Install(router11);
  ndnHelper.Install(router12);
  ndnHelper.Install(router13);
  ndnHelper.Install(router14);
  ndnHelper.Install(router15);
  ndnHelper.Install(router16);
  ndnHelper.Install(router17);
  ndnHelper.Install(router18);
  ndnHelper.Install(router19);
  ndnHelper.Install(router20);
  ndnHelper.Install(router21);




  std::string Prefix(PREFIX);


  // Choosing forwarding strategy
  //ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceOrchestration", "/localhost/nfd/strategy/best-route");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceOrchestration", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/sensor1", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/sensor2", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/sensor3", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/sensor4", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/sensor5", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/sensor6", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/sensor7", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/sensor8", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/sensor9", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/sensor10", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/sensor11", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/sensor12", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/sensor13", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/sensor14", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/sensor15", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/sensor16", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/sensor17", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/sensor18", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/sensor19", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/sensor20", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceP1", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceP2", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceP3", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceP4", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceP5", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceP6", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceP7", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceP8", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceP9", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceP10", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceP11", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceP12", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceP13", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceP14", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceP15", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceP16", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceP17", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceP18", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceP19", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceP20", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceP21", "/localhost/nfd/strategy/multicast");

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
  sensorApp.SetAttribute("Service", StringValue("sensor1"));
  sensorApp.Install(producer1).Start(Seconds(0));
  sensorApp.SetPrefix(Prefix);
  sensorApp.SetAttribute("Service", StringValue("sensor2"));
  sensorApp.Install(producer2).Start(Seconds(0));
  sensorApp.SetPrefix(Prefix);
  sensorApp.SetAttribute("Service", StringValue("sensor3"));
  sensorApp.Install(producer3).Start(Seconds(0));
  sensorApp.SetPrefix(Prefix);
  sensorApp.SetAttribute("Service", StringValue("sensor4"));
  sensorApp.Install(producer4).Start(Seconds(0));
  sensorApp.SetPrefix(Prefix);
  sensorApp.SetAttribute("Service", StringValue("sensor5"));
  sensorApp.Install(producer5).Start(Seconds(0));
  sensorApp.SetPrefix(Prefix);
  sensorApp.SetAttribute("Service", StringValue("sensor6"));
  sensorApp.Install(producer6).Start(Seconds(0));
  sensorApp.SetPrefix(Prefix);
  sensorApp.SetAttribute("Service", StringValue("sensor7"));
  sensorApp.Install(producer7).Start(Seconds(0));
  sensorApp.SetPrefix(Prefix);
  sensorApp.SetAttribute("Service", StringValue("sensor8"));
  sensorApp.Install(producer8).Start(Seconds(0));
  sensorApp.SetPrefix(Prefix);
  sensorApp.SetAttribute("Service", StringValue("sensor9"));
  sensorApp.Install(producer9).Start(Seconds(0));
  sensorApp.SetPrefix(Prefix);
  sensorApp.SetAttribute("Service", StringValue("sensor10"));
  sensorApp.Install(producer10).Start(Seconds(0));
  sensorApp.SetPrefix(Prefix);
  sensorApp.SetAttribute("Service", StringValue("sensor11"));
  sensorApp.Install(producer11).Start(Seconds(0));
  sensorApp.SetPrefix(Prefix);
  sensorApp.SetAttribute("Service", StringValue("sensor12"));
  sensorApp.Install(producer12).Start(Seconds(0));
  sensorApp.SetPrefix(Prefix);
  sensorApp.SetAttribute("Service", StringValue("sensor13"));
  sensorApp.Install(producer13).Start(Seconds(0));
  sensorApp.SetPrefix(Prefix);
  sensorApp.SetAttribute("Service", StringValue("sensor14"));
  sensorApp.Install(producer14).Start(Seconds(0));
  sensorApp.SetPrefix(Prefix);
  sensorApp.SetAttribute("Service", StringValue("sensor15"));
  sensorApp.Install(producer15).Start(Seconds(0));
  sensorApp.SetPrefix(Prefix);
  sensorApp.SetAttribute("Service", StringValue("sensor16"));
  sensorApp.Install(producer16).Start(Seconds(0));
  sensorApp.SetPrefix(Prefix);
  sensorApp.SetAttribute("Service", StringValue("sensor17"));
  sensorApp.Install(producer17).Start(Seconds(0));
  sensorApp.SetPrefix(Prefix);
  sensorApp.SetAttribute("Service", StringValue("sensor18"));
  sensorApp.Install(producer18).Start(Seconds(0));
  sensorApp.SetPrefix(Prefix);
  sensorApp.SetAttribute("Service", StringValue("sensor19"));
  sensorApp.Install(producer19).Start(Seconds(0));
  sensorApp.SetPrefix(Prefix);
  sensorApp.SetAttribute("Service", StringValue("sensor20"));
  sensorApp.Install(producer20).Start(Seconds(0));

  // Custom App for routers
  ndn::AppHelper serviceApp("DagServiceA_App");
  serviceApp.SetPrefix(Prefix);
  serviceApp.SetAttribute("Service", StringValue("serviceP1"));
  serviceApp.Install(router1).Start(Seconds(0));
  serviceApp.SetPrefix(Prefix);
  serviceApp.SetAttribute("Service", StringValue("serviceP2"));
  serviceApp.Install(router2).Start(Seconds(0));
  serviceApp.SetPrefix(Prefix);
  serviceApp.SetAttribute("Service", StringValue("serviceP3"));
  serviceApp.Install(router3).Start(Seconds(0));
  serviceApp.SetPrefix(Prefix);
  serviceApp.SetAttribute("Service", StringValue("serviceP4"));
  serviceApp.Install(router4).Start(Seconds(0));
  serviceApp.SetPrefix(Prefix);
  serviceApp.SetAttribute("Service", StringValue("serviceP5"));
  serviceApp.Install(router5).Start(Seconds(0));
  serviceApp.SetPrefix(Prefix);
  serviceApp.SetAttribute("Service", StringValue("serviceP6"));
  serviceApp.Install(router6).Start(Seconds(0));
  serviceApp.SetPrefix(Prefix);
  serviceApp.SetAttribute("Service", StringValue("serviceP7"));
  serviceApp.Install(router7).Start(Seconds(0));
  serviceApp.SetPrefix(Prefix);
  serviceApp.SetAttribute("Service", StringValue("serviceP8"));
  serviceApp.Install(router8).Start(Seconds(0));
  serviceApp.SetPrefix(Prefix);
  serviceApp.SetAttribute("Service", StringValue("serviceP9"));
  serviceApp.Install(router9).Start(Seconds(0));
  serviceApp.SetPrefix(Prefix);
  serviceApp.SetAttribute("Service", StringValue("serviceP10"));
  serviceApp.Install(router10).Start(Seconds(0));
  serviceApp.SetPrefix(Prefix);
  serviceApp.SetAttribute("Service", StringValue("serviceP11"));
  serviceApp.Install(router11).Start(Seconds(0));
  serviceApp.SetPrefix(Prefix);
  serviceApp.SetAttribute("Service", StringValue("serviceP12"));
  serviceApp.Install(router12).Start(Seconds(0));
  serviceApp.SetPrefix(Prefix);
  serviceApp.SetAttribute("Service", StringValue("serviceP13"));
  serviceApp.Install(router13).Start(Seconds(0));
  serviceApp.SetPrefix(Prefix);
  serviceApp.SetAttribute("Service", StringValue("serviceP14"));
  serviceApp.Install(router14).Start(Seconds(0));
  serviceApp.SetPrefix(Prefix);
  serviceApp.SetAttribute("Service", StringValue("serviceP15"));
  serviceApp.Install(router15).Start(Seconds(0));
  serviceApp.SetPrefix(Prefix);
  serviceApp.SetAttribute("Service", StringValue("serviceP16"));
  serviceApp.Install(router16).Start(Seconds(0));
  serviceApp.SetPrefix(Prefix);
  serviceApp.SetAttribute("Service", StringValue("serviceP17"));
  serviceApp.Install(router17).Start(Seconds(0));
  serviceApp.SetPrefix(Prefix);
  serviceApp.SetAttribute("Service", StringValue("serviceP18"));
  serviceApp.Install(router18).Start(Seconds(0));
  serviceApp.SetPrefix(Prefix);
  serviceApp.SetAttribute("Service", StringValue("serviceP19"));
  serviceApp.Install(router19).Start(Seconds(0));
  serviceApp.SetPrefix(Prefix);
  serviceApp.SetAttribute("Service", StringValue("serviceP20"));
  serviceApp.Install(router20).Start(Seconds(0));
  serviceApp.SetPrefix(Prefix);
  serviceApp.SetAttribute("Service", StringValue("serviceP21"));
  serviceApp.Install(router21).Start(Seconds(0));

  ndn::AppHelper orchestratorApp("DagOrchestratorA_App");
  orchestratorApp.SetPrefix(Prefix);
  orchestratorApp.SetAttribute("Service", StringValue("serviceOrchestration"));
  //orchestratorApp.Install(orchestrator).Start(Seconds(0));
  orchestratorApp.Install(consumer).Start(Seconds(0));

  // Custom App for User(Consumer)
  ndn::AppHelper userApp("CustomAppConsumer");
  //userApp.SetPrefix("/cabeee/sensor/service1/service2/service3");
  //userApp.SetPrefix("/service4/service3/service2/service1/sensor"); // only for linear workflows
  userApp.SetPrefix(Prefix);
  userApp.SetAttribute("Service", StringValue("consumer"));
  userApp.SetAttribute("Workflow", StringValue("workflows/20-sensor.json"));
  userApp.SetAttribute("Orchestrate", UintegerValue(1)); // This enables the "orchestrator" by having the consumer set the head service to /serviceOrchestration
  userApp.Install(consumer).Start(Seconds(0));

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




  Simulator::Stop(Seconds(3.0));

  //ndn::L3RateTracer::InstallAll("rate-trace_cabeee-20sensor-parallel-orchestratorA.txt", Seconds(0.1));
  //ndn::CsTracer::InstallAll("cs-trace_cabeee-20sensor-parallel-orchestratorA.txt", Seconds(0.1));

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
