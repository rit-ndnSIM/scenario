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
#include "ns3/point-to-point-module.h" // added for pCap generation


#define PREFIX "/nesco"

namespace ns3 {

/*
* 
*     NS_LOG=CustomAppConsumer:CustomAppProducer:DagForwarderApp ./waf --run=ndn-cabeee-cacheAdv
*/


int
main(int argc, char* argv[])
{
  // Read optional command-line parameters (e.g., enable visualizer with ./waf --run=<> --visualize
  CommandLine cmd;
  cmd.Parse(argc, argv);

  // Creating nodes
  AnnotatedTopologyReader topologyReader("", 1);
  topologyReader.SetFileName("topologies/topo-cabeee-3node-cacheAdv.txt");
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
  Ptr<Node> consumer = Names::Find<Node>("user");

  ndnHelper.setCsSize(0); // disable content store
  ndnHelper.Install(producer);

  ndnHelper.setCsSize(0); // enable/disable content store
  ndnHelper.Install(router1);
  ndnHelper.setCsSize(1000); // enable/disable content store
  ndnHelper.Install(router2);
  ndnHelper.setCsSize(0); // enable/disable content store
  ndnHelper.Install(router3);


  ndnHelper.setCsSize(0); // disable content store
  ndnHelper.Install(consumer);



  std::string Prefix(PREFIX);

  // Choosing forwarding strategy
  ndn::StrategyChoiceHelper::InstallAll(PREFIX, "/localhost/nfd/strategy/best-route");
  //ndn::StrategyChoiceHelper::InstallAll(PREFIX, "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/service1", "/localhost/nfd/strategy/best-route");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/sensor", "/localhost/nfd/strategy/best-route");
  //ndn::StrategyChoiceHelper::InstallAll(Prefix + "/csUpdate", "/localhost/nfd/strategy/best-route");
  //ndn::StrategyChoiceHelper::InstallAll(Prefix + "/service1", "/localhost/nfd/strategy/multicast");
  //ndn::StrategyChoiceHelper::InstallAll(Prefix + "/sensor", "/localhost/nfd/strategy/multicast");
  //ndn::StrategyChoiceHelper::InstallAll(Prefix + "/shortcutOPT", "/localhost/nfd/strategy/multicast");

  // Installing global routing interface on all nodes
  ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
  ndnGlobalRoutingHelper.InstallAll();

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
  routerApp.Install(router1).Start(Seconds(0));

  // Custom App for Content Store updaters
  ndn::AppHelper csUpdaterApp("CustomAppCsUpdater");
  csUpdaterApp.SetPrefix(Prefix);
  csUpdaterApp.SetAttribute("Service", StringValue("csUpdate"));
  csUpdaterApp.Install(router2).Start(Seconds(0));

  // Custom App for User(Consumer)
  ndn::AppHelper userApp("CustomAppConsumer");
  userApp.SetPrefix(Prefix);
  userApp.SetAttribute("Service", StringValue("consumer1"));
  userApp.SetAttribute("Workflow", StringValue("workflows/1dag.json"));
  userApp.SetAttribute("Orchestrate", UintegerValue(0));
  userApp.Install(router3).Start(Seconds(0));


  // Custom App for User2(Consumer2)
  ndn::AppHelper userApp2("CustomAppConsumer");
  userApp2.SetPrefix(Prefix);
  userApp2.SetAttribute("Service", StringValue("consumer2"));
  userApp2.SetAttribute("Workflow", StringValue("workflows/1dag.json"));
  userApp2.SetAttribute("Orchestrate", UintegerValue(0));
  userApp2.Install(consumer).Start(Seconds(3));


  ndnGlobalRoutingHelper.AddOrigins(Prefix + "/sensor", producer);
  ndnGlobalRoutingHelper.AddOrigins(Prefix + "/service1", router1);


  // Calculate and install FIBs
  ndn::GlobalRoutingHelper::CalculateRoutes();


  // Schedule the addition of cached item into RIB and recalculate routes
  //std::string originPrefix = (Prefix + "/service1/params-sha256=b11a48b8384e652ea726efb193902553c97041a52670bb25b5f2c19bb15a8af3");
  //Simulator::Schedule(Seconds(1.0), &(ndnGlobalRoutingHelper.AddOrigins), originPrefix, router2);
  //ndn::GlobalRoutingHelper * grhPtr = &ndnGlobalRoutingHelper;
  //Simulator::Schedule(Seconds(1.0), grhPtr->AddOrigins, originPrefix, router2);
  //Simulator::Schedule(Seconds(2.0), ndn::GlobalRoutingHelper::CalculateRoutes);


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



  Simulator::Stop(Seconds(5.0));

  ndn::L3RateTracer::InstallAll("rate-trace_cabeee-cacheAdv.txt", Seconds(0.0005));
  ndn::CsTracer::InstallAll("cs-trace_cabeee-cacheAdv.txt", Seconds(0.0005));

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
