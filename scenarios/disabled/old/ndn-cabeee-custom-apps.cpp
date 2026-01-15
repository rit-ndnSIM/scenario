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

namespace ns3 {

/**
*                  /------\      -------
*                  |sensor|------| APP |
*                  \------/Fapp  -------
*                    ^ F1
*                    |
*                  /
*                /
*              /
*            |
*            v F2
*       /-------\ F3  F4 /-------\
*       | rtr-1 | <----> | rtr-2 |
*       \-------/        \-------/
*                           ^ F5                      
*                           |
*                          /
*                        /
*                      / 
*                     |      
*                     v F6
*               /--------\
*               |  rtr-3 |
*               \--------/
*                    ^ F7
*                    |
*                    v F8
*               /--------\Fapp  -------
*               |  user  |------| APP |
*               \--------/      -------
* 
 *     NS_LOG=CustomAppConsumer:CustomAppProducer ./waf --run=ndn-cabeee-custom-apps
 */
int
main(int argc, char* argv[])
{
  // Read optional command-line parameters (e.g., enable visualizer with ./waf --run=<> --visualize
  CommandLine cmd;
  cmd.Parse(argc, argv);

  // Creating nodes
  AnnotatedTopologyReader topologyReader("", 1);
  //topologyReader.SetFileName("topologies/topo-cabeee-3node.txt");
  topologyReader.SetFileName("topologies/topo-cabeee-3node-slow.txt");
  topologyReader.Read();




  // Install NDN stack on all nodes
  ndn::StackHelper ndnHelper;
  ndnHelper.setPolicy("nfd::cs::lru");
  //ndnHelper.setCsSize(100);
  //ndnHelper.setCsSize(1);
  ndnHelper.SetDefaultRoutes(true);

  //ndnHelper.InstallAll();

  // figure out how to setup the CS only in select nodes!!

  // Getting containers for the nodes
  Ptr<Node> producer = Names::Find<Node>("sensor");
  Ptr<Node> router1 = Names::Find<Node>("rtr-1");
  Ptr<Node> router2 = Names::Find<Node>("rtr-2");
  Ptr<Node> router3 = Names::Find<Node>("rtr-3");
  Ptr<Node> consumer = Names::Find<Node>("user");

  ndnHelper.setCsSize(0); // disable content store
  ndnHelper.Install(producer);

  ndnHelper.setCsSize(100); // enable content store
  ndnHelper.Install(router1);

  ndnHelper.setCsSize(100); // enable content store
  ndnHelper.Install(router2);

  ndnHelper.setCsSize(100); // enable content store
  ndnHelper.Install(router3);

  ndnHelper.setCsSize(0); // disable content store
  ndnHelper.Install(consumer);




  // Choosing forwarding strategy
  //ndn::StrategyChoiceHelper::InstallAll("/prefix", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll("/prefix", "/localhost/nfd/strategy/best-route");

  // Installing global routing interface on all nodes
  //ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
  //ndnGlobalRoutingHelper.InstallAll();

  // Getting containers for the consumer/producer
  //Ptr<Node> consumer = Names::Find<Node>("user");
  //Ptr<Node> producer = Names::Find<Node>("sensor");
  //Ptr<Node> producer = Names::Find<Node>("rtr-1");

 


  // Installing applications

  // Custom App1
  ndn::AppHelper app1("CustomAppConsumer");
  app1.SetPrefix("/cabeee-prefix");
  app1.Install(consumer).Start(Seconds(0));

  // Custom App2
  ndn::AppHelper app2("CustomAppProducer");
  app2.SetPrefix("/cabeee-prefix");
  app2.Install(producer).Start(Seconds(0));

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
  //ndn::FibHelper::AddRoute(consumer, "/prefix/sub", router3, 1);
  //ndn::FibHelper::AddRoute(router3, "/prefix/sub", router2, 1);
  //ndn::FibHelper::AddRoute(router2, "/prefix/sub", router1, 1);
  //ndn::FibHelper::AddRoute(router1, "/prefix/sub", producer, 1);

  // cabeee - it seems this still works without calculating/installing FIB and without manually configuring FIB routes




  Simulator::Stop(Seconds(20.0));

  ndn::L3RateTracer::InstallAll("rate-trace_cabeee-custom-apps.txt", Seconds(0.1));
  ndn::CsTracer::InstallAll("cs-trace_cabeee-custom-apps.txt", Seconds(0.1));

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
