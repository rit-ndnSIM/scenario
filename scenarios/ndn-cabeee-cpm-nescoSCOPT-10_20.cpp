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

int
main(int argc, char* argv[])
{
  // Read optional command-line parameters (e.g., enable visualizer with ./waf --run=<> --visualize
  CommandLine cmd;
  cmd.Parse(argc, argv);

  // Creating nodes
  AnnotatedTopologyReader topologyReader("", 1);
  topologyReader.SetFileName("topologies/topo-cabeee-cpm-x_20.txt");
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
  Ptr<Node> rtr001 = Names::Find<Node>("rtr-001");
  Ptr<Node> rtr002 = Names::Find<Node>("rtr-002");
  Ptr<Node> rtr003 = Names::Find<Node>("rtr-003");
  Ptr<Node> rtr004 = Names::Find<Node>("rtr-004");
  Ptr<Node> rtr005 = Names::Find<Node>("rtr-005");
  Ptr<Node> rtr006 = Names::Find<Node>("rtr-006");
  Ptr<Node> rtr007 = Names::Find<Node>("rtr-007");
  Ptr<Node> rtr008 = Names::Find<Node>("rtr-008");
  Ptr<Node> rtr009 = Names::Find<Node>("rtr-009");
  Ptr<Node> rtr010 = Names::Find<Node>("rtr-010");
  Ptr<Node> rtr011 = Names::Find<Node>("rtr-011");
  Ptr<Node> rtr012 = Names::Find<Node>("rtr-012");
  Ptr<Node> rtr013 = Names::Find<Node>("rtr-013");
  Ptr<Node> rtr014 = Names::Find<Node>("rtr-014");
  Ptr<Node> rtr015 = Names::Find<Node>("rtr-015");
  Ptr<Node> rtr016 = Names::Find<Node>("rtr-016");
  Ptr<Node> rtr017 = Names::Find<Node>("rtr-017");
  Ptr<Node> rtr018 = Names::Find<Node>("rtr-018");
  Ptr<Node> rtr019 = Names::Find<Node>("rtr-019");
  Ptr<Node> rtr020 = Names::Find<Node>("rtr-020");
  Ptr<Node> consumer = Names::Find<Node>("user");

  ndnHelper.setCsSize(0); // disable content store
  ndnHelper.Install(producer);
  ndnHelper.Install(rtr001);
  ndnHelper.Install(rtr002);
  ndnHelper.Install(rtr003);
  ndnHelper.Install(rtr004);
  ndnHelper.Install(rtr005);
  ndnHelper.Install(rtr006);
  ndnHelper.Install(rtr007);
  ndnHelper.Install(rtr008);
  ndnHelper.Install(rtr009);
  ndnHelper.Install(rtr010);
  ndnHelper.Install(rtr011);
  ndnHelper.Install(rtr012);
  ndnHelper.Install(rtr013);
  ndnHelper.Install(rtr014);
  ndnHelper.Install(rtr015);
  ndnHelper.Install(rtr016);
  ndnHelper.Install(rtr017);
  ndnHelper.Install(rtr018);
  ndnHelper.Install(rtr019);
  ndnHelper.Install(rtr020);
  ndnHelper.Install(consumer);



  std::string Prefix(PREFIX);

  // Choosing forwarding strategy
  //ndn::StrategyChoiceHelper::InstallAll("/prefix", "/localhost/nfd/strategy/best-route");
  //ndn::StrategyChoiceHelper::InstallAll("/prefix", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-001", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-002", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-003", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-004", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-005", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-006", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-007", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-008", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-009", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-010", "/localhost/nfd/strategy/multicast");
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
  routerApp.SetAttribute("Service", StringValue("serviceCPM-001"));
  routerApp.Install(rtr002).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-002"));
  routerApp.Install(rtr004).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-003"));
  routerApp.Install(rtr006).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-004"));
  routerApp.Install(rtr008).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-005"));
  routerApp.Install(rtr010).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-006"));
  routerApp.Install(rtr012).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-007"));
  routerApp.Install(rtr014).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-008"));
  routerApp.Install(rtr016).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-009"));
  routerApp.Install(rtr018).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-010"));
  routerApp.Install(rtr020).Start(Seconds(0));

  // Custom App for User(Consumer)
  ndn::AppHelper userApp("CustomAppConsumer");
  userApp.SetPrefix(Prefix);
  userApp.SetAttribute("Service", StringValue("consumer"));
  userApp.SetAttribute("Workflow", StringValue("workflows/cpm-10_x.json"));
  userApp.SetAttribute("Orchestrate", UintegerValue(0));
  userApp.Install(consumer).Start(Seconds(0));



  Simulator::Stop(Seconds(2.1));


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
