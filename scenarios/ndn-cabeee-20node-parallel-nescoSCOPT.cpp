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

#define PREFIX "/nescoSCOPT"

namespace ns3 {

/**
*       /--------------------------------------\ Fapp     ----------------
*  node0|              sensor                  |----------| Producer APP | Sensor Data
*       \--------------------------------------/          ----------------
*         ^ F8             ^ F3            ^ F1
*         |                |               |
*         |                |               |
*         |                |               v F2
*         |                |             /-------\ Fapp   ---------------------
*         |                |       node20| rtr-20|--------| DAG Forwarder APP | Service 20
*         |                |             \-------/        ---------------------
*         |                |               ^
*         |                v F6            |
*         |              /-------\ Fapp    |              ---------------------
*         |         noden| rtr-n |---------|--------------| DAG Forwarder APP | Service n
*         |              \-------/         |              ---------------------
*         |                ^               |
*         v F6             |               | 
*       /-------\ Fapp     |               |              ---------------------
*  node1| rtr-1 |----------|---------------|--------------| DAG Forwarder APP | Service 1
*       \-------/          |               |              ---------------------
*         ^ F7             |               |
*         |                |               |
*         |                |               |
*         v F8             v               v
*       /--------------------------------------\ Fapp     ---------------------
* node21|              rtr-21                  |----------| DAG Forwarder APP | Service 21
*       \--------------------------------------/          ---------------------
*         ^ F7
*         |
*         v F6
*       /--------\
* node22|  orch  |
*       \--------/
*         ^ F9
*     0ms |
*         v FA
*       /--------\ Fapp   ----------------
* node23|  user  |--------| Consumer APP |
*       \--------/        ----------------
* 
*     NS_LOG=CustomAppConsumer:CustomAppProducer:DagForwarderApp ./waf --run=ndn-cabeee-20node-parallel
*/
int
main(int argc, char* argv[])
{
  // Read optional command-line parameters (e.g., enable visualizer with ./waf --run=<> --visualize
  CommandLine cmd;
  cmd.Parse(argc, argv);

  // Creating nodes
  AnnotatedTopologyReader topologyReader("", 1);
  topologyReader.SetFileName("topologies/topo-cabeee-20node-parallel.txt");
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

  ndnHelper.setCsSize(0); // disable content store
  ndnHelper.Install(producer);
  //ndnHelper.setCsSize(0); // disable content store
  //ndnHelper.Install(orchestrator);
  ndnHelper.setCsSize(0); // disable content store
  ndnHelper.Install(consumer);

  ndnHelper.setCsSize(0); // enable/disable content store
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
  //ndn::StrategyChoiceHelper::InstallAll(Prefix + "/sensor", "/localhost/nfd/strategy/best-route");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/sensor", "/localhost/nfd/strategy/multicast");
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
  sensorApp.SetAttribute("Service", StringValue("sensor"));
  sensorApp.Install(producer).Start(Seconds(0));

  // Custom App for routers
  ndn::AppHelper routerApp("DagForwarderApp");
  routerApp.SetPrefix(Prefix);
  routerApp.SetAttribute("Service", StringValue("serviceP1"));
  routerApp.Install(router1).Start(Seconds(0));
  routerApp.SetPrefix(Prefix);
  routerApp.SetAttribute("Service", StringValue("serviceP2"));
  routerApp.Install(router2).Start(Seconds(0));
  routerApp.SetPrefix(Prefix);
  routerApp.SetAttribute("Service", StringValue("serviceP3"));
  routerApp.Install(router3).Start(Seconds(0));
  routerApp.SetPrefix(Prefix);
  routerApp.SetAttribute("Service", StringValue("serviceP4"));
  routerApp.Install(router4).Start(Seconds(0));
  routerApp.SetPrefix(Prefix);
  routerApp.SetAttribute("Service", StringValue("serviceP5"));
  routerApp.Install(router5).Start(Seconds(0));
  routerApp.SetPrefix(Prefix);
  routerApp.SetAttribute("Service", StringValue("serviceP6"));
  routerApp.Install(router6).Start(Seconds(0));
  routerApp.SetPrefix(Prefix);
  routerApp.SetAttribute("Service", StringValue("serviceP7"));
  routerApp.Install(router7).Start(Seconds(0));
  routerApp.SetPrefix(Prefix);
  routerApp.SetAttribute("Service", StringValue("serviceP8"));
  routerApp.Install(router8).Start(Seconds(0));
  routerApp.SetPrefix(Prefix);
  routerApp.SetAttribute("Service", StringValue("serviceP9"));
  routerApp.Install(router9).Start(Seconds(0));
  routerApp.SetPrefix(Prefix);
  routerApp.SetAttribute("Service", StringValue("serviceP10"));
  routerApp.Install(router10).Start(Seconds(0));
  routerApp.SetPrefix(Prefix);
  routerApp.SetAttribute("Service", StringValue("serviceP11"));
  routerApp.Install(router11).Start(Seconds(0));
  routerApp.SetPrefix(Prefix);
  routerApp.SetAttribute("Service", StringValue("serviceP12"));
  routerApp.Install(router12).Start(Seconds(0));
  routerApp.SetPrefix(Prefix);
  routerApp.SetAttribute("Service", StringValue("serviceP13"));
  routerApp.Install(router13).Start(Seconds(0));
  routerApp.SetPrefix(Prefix);
  routerApp.SetAttribute("Service", StringValue("serviceP14"));
  routerApp.Install(router14).Start(Seconds(0));
  routerApp.SetPrefix(Prefix);
  routerApp.SetAttribute("Service", StringValue("serviceP15"));
  routerApp.Install(router15).Start(Seconds(0));
  routerApp.SetPrefix(Prefix);
  routerApp.SetAttribute("Service", StringValue("serviceP16"));
  routerApp.Install(router16).Start(Seconds(0));
  routerApp.SetPrefix(Prefix);
  routerApp.SetAttribute("Service", StringValue("serviceP17"));
  routerApp.Install(router17).Start(Seconds(0));
  routerApp.SetPrefix(Prefix);
  routerApp.SetAttribute("Service", StringValue("serviceP18"));
  routerApp.Install(router18).Start(Seconds(0));
  routerApp.SetPrefix(Prefix);
  routerApp.SetAttribute("Service", StringValue("serviceP19"));
  routerApp.Install(router19).Start(Seconds(0));
  routerApp.SetPrefix(Prefix);
  routerApp.SetAttribute("Service", StringValue("serviceP20"));
  routerApp.Install(router20).Start(Seconds(0));
  routerApp.SetPrefix(Prefix);
  routerApp.SetAttribute("Service", StringValue("serviceP21"));
  routerApp.Install(router21).Start(Seconds(0));

  // Custom App for User(Consumer)
  ndn::AppHelper userApp("CustomAppConsumer");
  userApp.SetPrefix(Prefix); // this is only a placeholder. The app will read the JSON workflow, and figure out which service is "last"
  userApp.SetAttribute("Service", StringValue("consumer"));
  userApp.SetAttribute("Workflow", StringValue("workflows/20-parallel.json"));
  userApp.SetAttribute("Orchestrate", UintegerValue(0));
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




  Simulator::Stop(Seconds(3));

  //ndn::L3RateTracer::InstallAll("rate-trace_cabeee-20node-parallel.txt", Seconds(0.1));
  //ndn::CsTracer::InstallAll("cs-trace_cabeee-20node-parallel.txt", Seconds(0.1));

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
