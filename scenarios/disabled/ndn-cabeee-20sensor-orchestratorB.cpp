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

#define PREFIX "/orchB"

namespace ns3 {

/**
*       /------\ Fapp x20 ----------------
*  node0|sensor|----------| Producer APP | Sensor 1-20
*       \------/          ----------------
*         ^ F1
*         |
*         |
*         v F2
*       /-------\ Fapp    ---------------------
*  node1| rtr-1 |---------|                   | No application, just a regular NDN forwarder
*       \-------/         ---------------------
*         ^ F3
*         |
*         .
*         .
*         .
*         |
*         v F4
*       /-------\ Fapp x20---------------------
*  node2| rtr-2 |---------| ServiceB APP      | Service 1-20
*       \-------/         ---------------------
*         ^ F5
*         |
*         .
*         .
*         |
*         v F6
*       /-------\ Fapp    ---------------------
*  node3| rtr-3 |-- ------| ServiceB APP      | Service 21
*       \-------/         ---------------------
*         ^ F7
*         |
*         |
*         v F8
*       /--------\ Fapp x2-----------------------------------
*  node4|  user  |--------| Consumer APP, OrchestratorB APP |
*       \--------/        -----------------------------------
* 
*     NS_LOG=CustomAppConsumer:CustomAppProducer:DagForwarderApp ./waf --run=ndn-cabeee-20sensor-orchestratorB
*/
int
main(int argc, char* argv[])
{
  // Read optional command-line parameters (e.g., enable visualizer with ./waf --run=<> --visualize
  CommandLine cmd;
  cmd.Parse(argc, argv);

  // Creating nodes
  AnnotatedTopologyReader topologyReader("", 1);
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
  //ndnHelper.setCsSize(0); // enable/disable content store by setting size
  //ndnHelper.Install(orchestrator);
  ndnHelper.setCsSize(0); // disable content store
  ndnHelper.Install(consumer);

  ndnHelper.setCsSize(100); // enable/disable content store by setting size
  ndnHelper.Install(router1);
  ndnHelper.Install(router2);
  ndnHelper.Install(router3);




  std::string Prefix(PREFIX);


  // Choosing forwarding strategy
  //ndn::StrategyChoiceHelper::InstallAll("/serviceOrchestration", "/localhost/nfd/strategy/best-route");
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
  sensorApp.Install(producer).Start(Seconds(0));
  sensorApp.SetPrefix(Prefix);
  sensorApp.SetAttribute("Service", StringValue("sensor2"));
  sensorApp.Install(producer).Start(Seconds(0));
  sensorApp.SetPrefix(Prefix);
  sensorApp.SetAttribute("Service", StringValue("sensor3"));
  sensorApp.Install(producer).Start(Seconds(0));
  sensorApp.SetPrefix(Prefix);
  sensorApp.SetAttribute("Service", StringValue("sensor4"));
  sensorApp.Install(producer).Start(Seconds(0));
  sensorApp.SetPrefix(Prefix);
  sensorApp.SetAttribute("Service", StringValue("sensor5"));
  sensorApp.Install(producer).Start(Seconds(0));
  sensorApp.SetPrefix(Prefix);
  sensorApp.SetAttribute("Service", StringValue("sensor6"));
  sensorApp.Install(producer).Start(Seconds(0));
  sensorApp.SetPrefix(Prefix);
  sensorApp.SetAttribute("Service", StringValue("sensor7"));
  sensorApp.Install(producer).Start(Seconds(0));
  sensorApp.SetPrefix(Prefix);
  sensorApp.SetAttribute("Service", StringValue("sensor8"));
  sensorApp.Install(producer).Start(Seconds(0));
  sensorApp.SetPrefix(Prefix);
  sensorApp.SetAttribute("Service", StringValue("sensor9"));
  sensorApp.Install(producer).Start(Seconds(0));
  sensorApp.SetPrefix(Prefix);
  sensorApp.SetAttribute("Service", StringValue("sensor10"));
  sensorApp.Install(producer).Start(Seconds(0));
  sensorApp.SetPrefix(Prefix);
  sensorApp.SetAttribute("Service", StringValue("sensor11"));
  sensorApp.Install(producer).Start(Seconds(0));
  sensorApp.SetPrefix(Prefix);
  sensorApp.SetAttribute("Service", StringValue("sensor12"));
  sensorApp.Install(producer).Start(Seconds(0));
  sensorApp.SetPrefix(Prefix);
  sensorApp.SetAttribute("Service", StringValue("sensor13"));
  sensorApp.Install(producer).Start(Seconds(0));
  sensorApp.SetPrefix(Prefix);
  sensorApp.SetAttribute("Service", StringValue("sensor14"));
  sensorApp.Install(producer).Start(Seconds(0));
  sensorApp.SetPrefix(Prefix);
  sensorApp.SetAttribute("Service", StringValue("sensor15"));
  sensorApp.Install(producer).Start(Seconds(0));
  sensorApp.SetPrefix(Prefix);
  sensorApp.SetAttribute("Service", StringValue("sensor16"));
  sensorApp.Install(producer).Start(Seconds(0));
  sensorApp.SetPrefix(Prefix);
  sensorApp.SetAttribute("Service", StringValue("sensor17"));
  sensorApp.Install(producer).Start(Seconds(0));
  sensorApp.SetPrefix(Prefix);
  sensorApp.SetAttribute("Service", StringValue("sensor18"));
  sensorApp.Install(producer).Start(Seconds(0));
  sensorApp.SetPrefix(Prefix);
  sensorApp.SetAttribute("Service", StringValue("sensor19"));
  sensorApp.Install(producer).Start(Seconds(0));
  sensorApp.SetPrefix(Prefix);
  sensorApp.SetAttribute("Service", StringValue("sensor20"));
  sensorApp.Install(producer).Start(Seconds(0));

  // Custom App for routers
  ndn::AppHelper routerApp("DagServiceB_App");
  routerApp.SetPrefix(Prefix);
  routerApp.SetAttribute("Service", StringValue("serviceP1"));
  routerApp.Install(router2).Start(Seconds(0));
  routerApp.SetPrefix(Prefix);
  routerApp.SetAttribute("Service", StringValue("serviceP2"));
  routerApp.Install(router2).Start(Seconds(0));
  routerApp.SetPrefix(Prefix);
  routerApp.SetAttribute("Service", StringValue("serviceP3"));
  routerApp.Install(router2).Start(Seconds(0));
  routerApp.SetPrefix(Prefix);
  routerApp.SetAttribute("Service", StringValue("serviceP4"));
  routerApp.Install(router2).Start(Seconds(0));
  routerApp.SetPrefix(Prefix);
  routerApp.SetAttribute("Service", StringValue("serviceP5"));
  routerApp.Install(router2).Start(Seconds(0));
  routerApp.SetPrefix(Prefix);
  routerApp.SetAttribute("Service", StringValue("serviceP6"));
  routerApp.Install(router2).Start(Seconds(0));
  routerApp.SetPrefix(Prefix);
  routerApp.SetAttribute("Service", StringValue("serviceP7"));
  routerApp.Install(router2).Start(Seconds(0));
  routerApp.SetPrefix(Prefix);
  routerApp.SetAttribute("Service", StringValue("serviceP8"));
  routerApp.Install(router2).Start(Seconds(0));
  routerApp.SetPrefix(Prefix);
  routerApp.SetAttribute("Service", StringValue("serviceP9"));
  routerApp.Install(router2).Start(Seconds(0));
  routerApp.SetPrefix(Prefix);
  routerApp.SetAttribute("Service", StringValue("serviceP10"));
  routerApp.Install(router2).Start(Seconds(0));
  routerApp.SetPrefix(Prefix);
  routerApp.SetAttribute("Service", StringValue("serviceP11"));
  routerApp.Install(router2).Start(Seconds(0));
  routerApp.SetPrefix(Prefix);
  routerApp.SetAttribute("Service", StringValue("serviceP12"));
  routerApp.Install(router2).Start(Seconds(0));
  routerApp.SetPrefix(Prefix);
  routerApp.SetAttribute("Service", StringValue("serviceP13"));
  routerApp.Install(router2).Start(Seconds(0));
  routerApp.SetPrefix(Prefix);
  routerApp.SetAttribute("Service", StringValue("serviceP14"));
  routerApp.Install(router2).Start(Seconds(0));
  routerApp.SetPrefix(Prefix);
  routerApp.SetAttribute("Service", StringValue("serviceP15"));
  routerApp.Install(router2).Start(Seconds(0));
  routerApp.SetPrefix(Prefix);
  routerApp.SetAttribute("Service", StringValue("serviceP16"));
  routerApp.Install(router2).Start(Seconds(0));
  routerApp.SetPrefix(Prefix);
  routerApp.SetAttribute("Service", StringValue("serviceP17"));
  routerApp.Install(router2).Start(Seconds(0));
  routerApp.SetPrefix(Prefix);
  routerApp.SetAttribute("Service", StringValue("serviceP18"));
  routerApp.Install(router2).Start(Seconds(0));
  routerApp.SetPrefix(Prefix);
  routerApp.SetAttribute("Service", StringValue("serviceP19"));
  routerApp.Install(router2).Start(Seconds(0));
  routerApp.SetPrefix(Prefix);
  routerApp.SetAttribute("Service", StringValue("serviceP20"));
  routerApp.Install(router2).Start(Seconds(0));
  routerApp.SetPrefix(Prefix);
  routerApp.SetAttribute("Service", StringValue("serviceP21"));
  routerApp.Install(router3).Start(Seconds(0)); 

  ndn::AppHelper orchestratorApp("DagOrchestratorB_App");
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
  userApp.SetAttribute("Orchestrate", UintegerValue(2)); // This enables the "orchestrator" by having the consumer set the head service to /serviceOrchestration/dag
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




  Simulator::Stop(Seconds(20.0));

  //ndn::L3RateTracer::InstallAll("rate-trace_cabeee-20sensor-orchestratorB.txt", Seconds(0.1));
  //ndn::CsTracer::InstallAll("cs-trace_cabeee-20sensor-orchestratorB.txt", Seconds(0.1));

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
