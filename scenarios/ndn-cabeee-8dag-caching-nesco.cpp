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
*     NS_LOG=CustomAppConsumer:CustomAppProducer:DagForwarderApp ./waf --run=ndn-cabeee-8dag-caching
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

  ndnHelper.setCsSize(0); // disable content store
  ndnHelper.Install(producer);

  ndnHelper.setCsSize(10); // enable/disable content store
  ndnHelper.Install(router1);

  ndnHelper.setCsSize(10); // enable/disable content store
  ndnHelper.Install(router2);

  ndnHelper.setCsSize(10); // enable/disable content store
  ndnHelper.Install(router3);

  //ndnHelper.setCsSize(0); // disable content store
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
  ndn::AppHelper routerApp("DagForwarderApp");

  routerApp.SetPrefix(Prefix);
  routerApp.SetAttribute("Service", StringValue("service1"));
  routerApp.Install(router3).Start(Seconds(0));
  routerApp.SetPrefix(Prefix);
  routerApp.SetAttribute("Service", StringValue("service5"));
  routerApp.Install(router3).Start(Seconds(0));

  routerApp.SetPrefix(Prefix);
  routerApp.SetAttribute("Service", StringValue("service2"));
  routerApp.Install(router1).Start(Seconds(0));
  routerApp.SetPrefix(Prefix);
  routerApp.SetAttribute("Service", StringValue("service6"));
  routerApp.Install(router1).Start(Seconds(0));

  routerApp.SetPrefix(Prefix);
  routerApp.SetAttribute("Service", StringValue("service3"));
  routerApp.Install(router2).Start(Seconds(0));
  routerApp.SetPrefix(Prefix);
  routerApp.SetAttribute("Service", StringValue("service7"));
  routerApp.Install(router2).Start(Seconds(0));

  routerApp.SetPrefix(Prefix);
  routerApp.SetAttribute("Service", StringValue("service4"));
  routerApp.Install(router2).Start(Seconds(0));
  routerApp.SetPrefix(Prefix);
  routerApp.SetAttribute("Service", StringValue("service8"));
  routerApp.Install(router2).Start(Seconds(0));

  // Custom App for User2(Consumer2) - Consumer 2 will not print results in the same way. We want consumer1 as the "final" one.
  ndn::AppHelper userApp2("CustomAppConsumer2");
  userApp2.SetPrefix(Prefix); // this is only a placeholder. The app will read the JSON workflow, and figure out which service is "last"
  userApp2.SetAttribute("Service", StringValue("consumer2"));
  userApp2.SetAttribute("Workflow", StringValue("workflows/4dag-cache.json"));
  userApp2.SetAttribute("Orchestrate", UintegerValue(0));
  userApp2.Install(consumer).Start(Seconds(0));

  // Custom App for User1(Consumer1)
  ndn::AppHelper userApp("CustomAppConsumer");
  userApp.SetPrefix(Prefix); // this is only a placeholder. The app will read the JSON workflow, and figure out which service is "last"
  userApp.SetAttribute("Service", StringValue("consumer"));
  userApp.SetAttribute("Workflow", StringValue("workflows/8dag.json"));
  //userApp.SetAttribute("Workflow", StringValue("workflows/rpa-dag_reuse2.json")); //TODO: test this other dag later
  userApp.SetAttribute("Orchestrate", UintegerValue(0));
  userApp.Install(consumer).Start(Seconds(4));

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

  ndn::L3RateTracer::InstallAll("rate-trace_cabeee-8dag-caching.txt", Seconds(0.1));
  ndn::CsTracer::InstallAll("cs-trace_cabeee-8dag-caching.txt", Seconds(0.1));

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
