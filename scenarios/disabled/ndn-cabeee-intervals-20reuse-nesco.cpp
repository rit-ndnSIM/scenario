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

#include "cabeee-cs-tools.h"

#define PREFIX "/nesco"

namespace ns3 {

/**
*     Uses Abilene topology
* 
*     NS_LOG=CustomAppConsumer:CustomAppProducer:DagForwarderApp ./waf --run=ndn-cabeee-20reuse-nesco
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
  Ptr<Node> rtrA1   = Names::Find<Node>("rtr-A1");
  Ptr<Node> rtrA1a  = Names::Find<Node>("rtr-A1a");
  Ptr<Node> rtrA2   = Names::Find<Node>("rtr-A2");
  Ptr<Node> rtrA2a  = Names::Find<Node>("rtr-A2a");
  Ptr<Node> rtrB    = Names::Find<Node>("rtr-B");
  Ptr<Node> rtrB1   = Names::Find<Node>("rtr-B1");
  Ptr<Node> rtrB1a  = Names::Find<Node>("rtr-B1a");
  Ptr<Node> rtrC    = Names::Find<Node>("rtr-C");
  Ptr<Node> rtrC1   = Names::Find<Node>("rtr-C1");
  Ptr<Node> rtrC1a  = Names::Find<Node>("rtr-C1a");
  Ptr<Node> rtrC1b  = Names::Find<Node>("rtr-C1b");
  Ptr<Node> rtrD    = Names::Find<Node>("rtr-D");
  Ptr<Node> rtrD1   = Names::Find<Node>("rtr-D1");
  Ptr<Node> rtrD2   = Names::Find<Node>("rtr-D2");
  Ptr<Node> rtrE    = Names::Find<Node>("rtr-E");
  Ptr<Node> rtrE1   = Names::Find<Node>("rtr-E1");
  Ptr<Node> rtrE1a  = Names::Find<Node>("rtr-E1a");
  Ptr<Node> rtrF    = Names::Find<Node>("rtr-F");
  Ptr<Node> rtrF1   = Names::Find<Node>("rtr-F1");
  Ptr<Node> rtrF2   = Names::Find<Node>("rtr-F2");
  Ptr<Node> rtrF2a  = Names::Find<Node>("rtr-F2a");
  Ptr<Node> rtrG    = Names::Find<Node>("rtr-G");
  Ptr<Node> rtrH    = Names::Find<Node>("rtr-H");
  Ptr<Node> rtrH1   = Names::Find<Node>("rtr-H1");
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
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceL11", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceL12", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceL13", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceL14", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceL15", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceL16", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceL17", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceL18", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceL19", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceL20", "/localhost/nfd/strategy/multicast");

  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceP22", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceP23", "/localhost/nfd/strategy/multicast");
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
  //sensorApp.SetAttribute("UniformFreshness", UintegerValue(1));   // this will override the FreshnessPeriod_ms setting below, and use Uniform Distribution to pick value ONCE!
  //sensorApp.SetAttribute("UniformFreshness", UintegerValue(2));   // this will override the FreshnessPeriod_ms setting below, and use Uniform Distribution to pick new value EVERY TIME!
  //sensorApp.SetAttribute("minFreshness_ms", UintegerValue(100));
  //sensorApp.SetAttribute("maxFreshness_ms", UintegerValue(1000));
  sensorApp.SetAttribute("FreshnessPeriod_ms", UintegerValue(1000));
  sensorApp.Install(rtrA1a).Start(Seconds(0));
  sensorApp.SetAttribute("Service", StringValue("sensor2"));
  sensorApp.SetAttribute("FreshnessPeriod_ms", UintegerValue(750));
  sensorApp.Install(rtrA1a).Start(Seconds(0));
  sensorApp.SetAttribute("Service", StringValue("sensor3"));
  sensorApp.SetAttribute("FreshnessPeriod_ms", UintegerValue(500));
  sensorApp.Install(rtrA1a).Start(Seconds(0));
  sensorApp.SetAttribute("Service", StringValue("sensor4"));
  sensorApp.SetAttribute("FreshnessPeriod_ms", UintegerValue(2500));
  sensorApp.Install(rtrA2a).Start(Seconds(0));
  sensorApp.SetAttribute("Service", StringValue("sensor5"));
  sensorApp.SetAttribute("FreshnessPeriod_ms", UintegerValue(1500));
  sensorApp.Install(rtrA2a).Start(Seconds(0));
  sensorApp.SetAttribute("Service", StringValue("sensor6"));
  sensorApp.SetAttribute("FreshnessPeriod_ms", UintegerValue(1500));
  sensorApp.Install(rtrA2a).Start(Seconds(0));
  sensorApp.SetAttribute("Service", StringValue("sensor7"));
  sensorApp.SetAttribute("FreshnessPeriod_ms", UintegerValue(2500));
  sensorApp.Install(rtrC1b).Start(Seconds(0));
  sensorApp.SetAttribute("Service", StringValue("sensor8"));
  sensorApp.SetAttribute("FreshnessPeriod_ms", UintegerValue(2400));
  sensorApp.Install(rtrC1b).Start(Seconds(0));
  sensorApp.SetAttribute("Service", StringValue("sensor9"));
  sensorApp.SetAttribute("FreshnessPeriod_ms", UintegerValue(2300));
  sensorApp.Install(rtrC1b).Start(Seconds(0));
  sensorApp.SetAttribute("Service", StringValue("sensor10"));
  sensorApp.SetAttribute("FreshnessPeriod_ms", UintegerValue(2200));
  sensorApp.Install(rtrC1b).Start(Seconds(0));
  sensorApp.SetAttribute("Service", StringValue("sensor11"));
  sensorApp.SetAttribute("FreshnessPeriod_ms", UintegerValue(2100));
  sensorApp.Install(rtrC1b).Start(Seconds(0));
  sensorApp.SetAttribute("Service", StringValue("sensor12"));
  sensorApp.SetAttribute("FreshnessPeriod_ms", UintegerValue(2000));
  sensorApp.Install(rtrC1b).Start(Seconds(0));
  sensorApp.SetAttribute("Service", StringValue("sensor13"));
  sensorApp.SetAttribute("FreshnessPeriod_ms", UintegerValue(1800));
  sensorApp.Install(rtrC1b).Start(Seconds(0));
  sensorApp.SetAttribute("Service", StringValue("sensor14"));
  sensorApp.SetAttribute("FreshnessPeriod_ms", UintegerValue(1500));
  sensorApp.Install(rtrB1a).Start(Seconds(0));
  sensorApp.SetAttribute("Service", StringValue("sensor15"));
  sensorApp.SetAttribute("FreshnessPeriod_ms", UintegerValue(1000));
  sensorApp.Install(rtrB1a).Start(Seconds(0));
  sensorApp.SetAttribute("Service", StringValue("sensor16"));
  sensorApp.SetAttribute("FreshnessPeriod_ms", UintegerValue(1000));
  sensorApp.Install(rtrB1a).Start(Seconds(0));
  sensorApp.SetAttribute("Service", StringValue("sensor17"));
  sensorApp.SetAttribute("FreshnessPeriod_ms", UintegerValue(1000));
  sensorApp.Install(rtrB1a).Start(Seconds(0));
  sensorApp.SetAttribute("Service", StringValue("sensor18"));
  sensorApp.SetAttribute("FreshnessPeriod_ms", UintegerValue(1000));
  sensorApp.Install(rtrB1a).Start(Seconds(0));
  sensorApp.SetAttribute("Service", StringValue("sensor19"));
  sensorApp.SetAttribute("FreshnessPeriod_ms", UintegerValue(1000));
  sensorApp.Install(rtrB1a).Start(Seconds(0));
  sensorApp.SetAttribute("Service", StringValue("sensor20"));
  sensorApp.SetAttribute("FreshnessPeriod_ms", UintegerValue(1000));
  sensorApp.Install(rtrB1a).Start(Seconds(0));


  sensorApp.SetAttribute("Service", StringValue("sensorL"));
  sensorApp.SetAttribute("FreshnessPeriod_ms", UintegerValue(2000));
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
  routerApp.SetAttribute("Service", StringValue("serviceP7"));
  routerApp.Install(rtrD1).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceP8"));
  routerApp.Install(rtrD1).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceP9"));
  routerApp.Install(rtrD1).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceP10"));
  routerApp.Install(rtrD1).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceP11"));
  routerApp.Install(rtrE1).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceP12"));
  routerApp.Install(rtrE1).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceP13"));
  routerApp.Install(rtrE1).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceP14"));
  routerApp.Install(rtrE1).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceP15"));
  routerApp.Install(rtrE1).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceP16"));
  routerApp.Install(rtrE1).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceP17"));
  routerApp.Install(rtrE1).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceP18"));
  routerApp.Install(rtrE1).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceP19"));
  routerApp.Install(rtrE1).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceP20"));
  routerApp.Install(rtrE1).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceP21"));
  routerApp.Install(rtrF1).Start(Seconds(0));



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
  routerApp.SetAttribute("Service", StringValue("serviceL11"));
  routerApp.Install(rtrI).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceL12"));
  routerApp.Install(rtrI).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceL13"));
  routerApp.Install(rtrI).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceL14"));
  routerApp.Install(rtrI).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceL15"));
  routerApp.Install(rtrI).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceL16"));
  routerApp.Install(rtrI).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceL17"));
  routerApp.Install(rtrI).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceL18"));
  routerApp.Install(rtrI).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceL19"));
  routerApp.Install(rtrI).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceL20"));
  routerApp.Install(rtrI).Start(Seconds(0));

  routerApp.SetAttribute("Service", StringValue("serviceP22"));
  routerApp.Install(rtrD2).Start(Seconds(0));

  routerApp.SetAttribute("Service", StringValue("serviceP23"));
  routerApp.Install(rtrB1).Start(Seconds(0));

  routerApp.SetAttribute("Service", StringValue("serviceR1"));
  routerApp.Install(rtrG).Start(Seconds(0));



  // Custom App for User(Consumer)
  ndn::AppHelper userApp("CustomAppConsumerPoisson");
  userApp.SetPrefix(Prefix);

  userApp.SetAttribute("Service", StringValue("consumerP"));
  userApp.SetAttribute("Workflow", StringValue("workflows/20-sensor.json"));
  userApp.SetAttribute("Orchestrate", UintegerValue(0));
  userApp.SetAttribute("Frequency", DoubleValue(10));       // 10 interests per second on average (Poisson process)
  userApp.SetAttribute("NumInterests", UintegerValue(100)); // 100 total interests will be generated
  userApp.Install(rtrE1a).Start(Seconds(2));

  userApp.SetAttribute("Service", StringValue("consumerL"));
  userApp.SetAttribute("Workflow", StringValue("workflows/20-linear.json"));
  userApp.SetAttribute("Orchestrate", UintegerValue(0));
  userApp.SetAttribute("Frequency", DoubleValue(10));       // 10 interests per second on average (Poisson process)
  userApp.SetAttribute("NumInterests", UintegerValue(100)); // 100 total interests will be generated
  userApp.Install(rtrH1a).Start(Seconds(2));

  userApp.SetAttribute("Service", StringValue("consumerR"));
  userApp.SetAttribute("Workflow", StringValue("workflows/20-reuse.json"));
  userApp.SetAttribute("Orchestrate", UintegerValue(0));
  userApp.SetAttribute("Frequency", DoubleValue(10));       // 10 interests per second on average (Poisson process)
  userApp.SetAttribute("NumInterests", UintegerValue(100)); // 100 total interests will be generated
  userApp.Install(rtrF2a).Start(Seconds(2));







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




  Simulator::Stop(Seconds(120));

  //ndn::L3RateTracer::InstallAll("rate-trace_cabeee-20reuse.txt", Seconds(1.0));
  ndn::CsTracer::InstallAll("cs-trace_cabeee-20reuse-nesco.txt", Seconds(1.0));

  std::ofstream fout("cs-usage-20reuse-nesco.txt");
  Simulator::Schedule(Seconds(0), &ns3::printCsHeader, ref(fout));
  Simulator::Schedule(Seconds(0), &ns3::printCsUsage, ref(fout), Seconds(0.5), PREFIX);
  //Simulator::Schedule(Seconds(0), &ns3::printCsHeader, ref(std::cout));
  //Simulator::Schedule(Seconds(0), &ns3::printCsUsage, ref(std::cout), Seconds(0.5), PREFIX); // record CS usage every 0.5 seconds

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
