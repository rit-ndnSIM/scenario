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
*  node1| rtr-1 |---------| DAG Forwarder APP | Service x
*       \-------/         ---------------------
*         ^ F3
*         |
*         .
*         .
*         .
*         |
*         v F4
*       /-------\ Fapp    ---------------------
*  node2| rtr-2 |---------| DAG Forwarder APP | Service x
*       \-------/         ---------------------
*         ^ F5
*         |
*         . Note: routers alternate per service as shown in the code below!
*         .
*         .
*         |
*         v F6
*       /-------\ Fapp    ---------------------
*  node3| rtr-3 |-- ------| DAG Forwarder APP | Service x
*       \-------/         ---------------------
*         ^ F7
*         |
*         |
*         v F8
*       /--------\ Fapp   ----------------
*  node4|  user  |--------| Consumer APP |
*       \--------/        ----------------
* 
*     NS_LOG=CustomAppConsumer:CustomAppProducer:DagForwarderApp ./waf --run=ndn-cabeee-20reuse
*/
int
main(int argc, char* argv[])
{
  // Read optional command-line parameters (e.g., enable visualizer with ./waf --run=<> --visualize
  CommandLine cmd;
  cmd.Parse(argc, argv);

  // Creating nodes
  AnnotatedTopologyReader topologyReader("", 1);
  topologyReader.SetFileName("topologies/topo-cabeee-Abilene.txt");
  topologyReader.Read();




  // Install NDN stack on all nodes
  ndn::StackHelper ndnHelper;
  ndnHelper.setPolicy("nfd::cs::lru");
  //ndnHelper.setCsSize(100);
  //ndnHelper.setCsSize(1);
  ndnHelper.SetDefaultRoutes(true);

  //ndnHelper.InstallAll();

  // Getting containers for the nodes
  Ptr<Node> rtrA    = Names::Find<Node>("rtr-A");
  Ptr<Node> rtrA1   = Names::Find<Node>("rtr-A1 ");
  Ptr<Node> rtrA1a  = Names::Find<Node>("rtr-A1a");
  Ptr<Node> rtrA2   = Names::Find<Node>("rtr-A2 ");
  Ptr<Node> rtrA2a  = Names::Find<Node>("rtr-A2a");
  Ptr<Node> rtrB    = Names::Find<Node>("rtr-B");
  Ptr<Node> rtrB1   = Names::Find<Node>("rtr-B1 ");
  Ptr<Node> rtrB1a  = Names::Find<Node>("rtr-B1a");
  Ptr<Node> rtrC    = Names::Find<Node>("rtr-C");
  Ptr<Node> rtrC1   = Names::Find<Node>("rtr-C1 ");
  Ptr<Node> rtrC1a  = Names::Find<Node>("rtr-C1a");
  Ptr<Node> rtrC1b  = Names::Find<Node>("rtr-C1b");
  Ptr<Node> rtrD    = Names::Find<Node>("rtr-D");
  Ptr<Node> rtrD1   = Names::Find<Node>("rtr-D1 ");
  Ptr<Node> rtrD2   = Names::Find<Node>("rtr-D2 ");
  Ptr<Node> rtrE    = Names::Find<Node>("rtr-E");
  Ptr<Node> rtrE1   = Names::Find<Node>("rtr-E1 ");
  Ptr<Node> rtrE1a  = Names::Find<Node>("rtr-E1a");
  Ptr<Node> rtrF    = Names::Find<Node>("rtr-F");
  Ptr<Node> rtrF1   = Names::Find<Node>("rtr-F1 ");
  Ptr<Node> rtrF2   = Names::Find<Node>("rtr-F2 ");
  Ptr<Node> rtrF2a  = Names::Find<Node>("rtr-F2a");
  Ptr<Node> rtrG    = Names::Find<Node>("rtr-G");
  Ptr<Node> rtrH    = Names::Find<Node>("rtr-H");
  Ptr<Node> rtrH1   = Names::Find<Node>("rtr-H1 ");
  Ptr<Node> rtrH1a  = Names::Find<Node>("rtr-H1a");
  Ptr<Node> rtrI    = Names::Find<Node>("rtr-I");
  Ptr<Node> rtrJ    = Names::Find<Node>("rtr-J");
  Ptr<Node> rtrK    = Names::Find<Node>("rtr-K");



  ndnHelper.setCsSize(1000); // enable/disable content store
  ndnHelper.Install(rtrA);
  ndnHelper.Install(rtrA1);
  //ndnHelper.Install(rtrA1a);
  ndnHelper.Install(rtrA2);
  //ndnHelper.Install(rtrA2a);
  ndnHelper.Install(rtrB);
  ndnHelper.Install(rtrB1);
  //ndnHelper.Install(rtrB1a);
  ndnHelper.Install(rtrC);
  ndnHelper.Install(rtrC1);
  //ndnHelper.Install(rtrC1a);
  //ndnHelper.Install(rtrC1b);
  ndnHelper.Install(rtrD);
  ndnHelper.Install(rtrD1);
  ndnHelper.Install(rtrD2);
  ndnHelper.Install(rtrE);
  ndnHelper.Install(rtrE1);
  //ndnHelper.Install(rtrE1a);
  ndnHelper.Install(rtrF);
  ndnHelper.Install(rtrF1);
  ndnHelper.Install(rtrF2);
  //ndnHelper.Install(rtrF2a);
  ndnHelper.Install(rtrG);
  ndnHelper.Install(rtrH);
  ndnHelper.Install(rtrH1);
  //ndnHelper.Install(rtrH1a);
  ndnHelper.Install(rtrI);
  ndnHelper.Install(rtrJ);
  ndnHelper.Install(rtrK);

  ndnHelper.setCsSize(0); // disable content store
  ndnHelper.Install(rtrA1a);
  ndnHelper.Install(rtrA2a);
  ndnHelper.Install(rtrB1a);
  ndnHelper.Install(rtrC1a);
  ndnHelper.Install(rtrC1b);
  ndnHelper.Install(rtrE1a);
  ndnHelper.Install(rtrF2a);
  ndnHelper.Install(rtrH1a);




  std::string Prefix(PREFIX);



  // Choosing forwarding strategy
  //ndn::StrategyChoiceHelper::InstallAll(Prefix + "/sensor", "/localhost/nfd/strategy/best-route");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/sensor1", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/sensor2", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/sensor3", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/sensor4", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/sensor5", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/sensor6", "/localhost/nfd/strategy/multicast");

  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceP1", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceP2", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceP3", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceP4", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceP5", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceP6", "/localhost/nfd/strategy/multicast");

  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceP22", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceP23", "/localhost/nfd/strategy/multicast");

  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/sensorL", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceL1", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceL2", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceL3", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceL4", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceL5", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceL6", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceL7", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceL8", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceL9", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceL10", "/localhost/nfd/strategy/multicast");

  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceR1", "/localhost/nfd/strategy/multicast");

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
  sensorApp.Install(rtrA1a).Start(Seconds(0));
  sensorApp.SetAttribute("Service", StringValue("sensor2"));
  sensorApp.Install(rtrA1a).Start(Seconds(0));
  sensorApp.SetAttribute("Service", StringValue("sensor3"));
  sensorApp.Install(rtrA1a).Start(Seconds(0));

  sensorApp.SetAttribute("Service", StringValue("sensor4"));
  sensorApp.Install(rtrA2a).Start(Seconds(0));
  sensorApp.SetAttribute("Service", StringValue("sensor5"));
  sensorApp.Install(rtrA2a).Start(Seconds(0));
  sensorApp.SetAttribute("Service", StringValue("sensor6"));
  sensorApp.Install(rtrA2a).Start(Seconds(0));

  sensorApp.SetAttribute("Service", StringValue("sensorL"));
  sensorApp.Install(rtrC1a).Start(Seconds(0));



  // Custom App for routers
  ndn::AppHelper routerApp("DagForwarderApp");
  routerApp.SetPrefix(Prefix);

  routerApp.SetAttribute("Service", StringValue("serviceP1"));
  routerApp.Install(rtrD1).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceP2"));
  routerApp.Install(rtrD1).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceP3"));
  routerApp.Install(rtrD1).Start(Seconds(0));

  routerApp.SetAttribute("Service", StringValue("serviceP4"));
  routerApp.Install(rtrD1).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceP5"));
  routerApp.Install(rtrD1).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceP6"));
  routerApp.Install(rtrD1).Start(Seconds(0));

  routerApp.SetAttribute("Service", StringValue("serviceP22"));
  routerApp.Install(rtrD2).Start(Seconds(0));

  routerApp.SetAttribute("Service", StringValue("serviceP23"));
  routerApp.Install(rtrB1).Start(Seconds(0));

  routerApp.SetAttribute("Service", StringValue("serviceL1"));
  routerApp.Install(rtrJ).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceL2"));
  routerApp.Install(rtrJ).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceL3"));
  routerApp.Install(rtrJ).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceL4"));
  routerApp.Install(rtrJ).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceL5"));
  routerApp.Install(rtrJ).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceL6"));
  routerApp.Install(rtrB).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceL7"));
  routerApp.Install(rtrB).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceL8"));
  routerApp.Install(rtrB).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceL9"));
  routerApp.Install(rtrB).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceL10"));
  routerApp.Install(rtrB).Start(Seconds(0));

  routerApp.SetAttribute("Service", StringValue("serviceR1"));
  routerApp.Install(rtrG).Start(Seconds(0));


  // Custom App for User(Consumer)
  ndn::AppHelper userApp("CustomAppConsumer");
  userApp.SetPrefix(Prefix);
  userApp.SetAttribute("Service", StringValue("consumer"));
  userApp.SetAttribute("Workflow", StringValue("workflows/20-reuse.json"));
  userApp.SetAttribute("Orchestrate", UintegerValue(0));
  userApp.Install(rtrF2a).Start(Seconds(0));

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




  Simulator::Stop(Seconds(10));

  //ndn::L3RateTracer::InstallAll("rate-trace_cabeee-20reuse.txt", Seconds(1.0));
  //ndn::CsTracer::InstallAll("cs-trace_cabeee-20reuse.txt", Seconds(1.0));

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
