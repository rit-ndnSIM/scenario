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

// ndn-cabeee-3node-tracers.cpp
// Scenario developed by Carlos Barrios (cabeee@rit.edu)
// built up from ndn-tree-tracers.cpp as a starting point
// June 2023

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ndnSIM-module.h"

namespace ns3 {

/**
*                  /------\
*                  |sensor|
*                  \------/
*                   ^
*                   |
*                  /
*                 /
*                /
*               |
*               v
*       /-------\        /-------\
*       | rtr-1 | <----> | rtr-2 |
*       \-------/        \-------/
*                          ^                      
*                          |
*                         /
*                        /
*                       / 
*                      |      
*                      v
*               /--------\
*               |  rtr-3 |
*               \--------/
*                    ^
*                    |
*                    v
*               /--------\
*               |  user  |
*               \--------/
*
*/

int
main(int argc, char* argv[])
{
  CommandLine cmd;
  cmd.Parse(argc, argv);

  AnnotatedTopologyReader topologyReader("", 1);
  topologyReader.SetFileName("topologies/topo-cabeee-3node.txt");
  topologyReader.Read();

  // Install NDN stack on all nodes
  ndn::StackHelper ndnHelper;
  ndnHelper.setPolicy("nfd::cs::lru");
  ndnHelper.setCsSize(100);
  ndnHelper.SetDefaultRoutes(true);
  ndnHelper.InstallAll();

  // Choosing forwarding strategy
  ndn::StrategyChoiceHelper::InstallAll("/prefix", "/localhost/nfd/strategy/best-route");
  //ndn::StrategyChoiceHelper::InstallAll("/prefix", "/localhost/nfd/strategy/multicast");

  // Installing global routing interface on all nodes
  ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
  ndnGlobalRoutingHelper.InstallAll();

  // Getting containers for the consumer/producer
  Ptr<Node> consumer = Names::Find<Node>("user");
  Ptr<Node> producer = Names::Find<Node>("sensor");

  ndn::AppHelper consumerHelper("ns3::ndn::ConsumerCbr");
  consumerHelper.SetAttribute("Frequency", StringValue("1")); // 1 interest per second

  // Each consumer will express unique interests /sensor/<leaf-name>/<seq-no>
  consumerHelper.SetPrefix("/sensor/" + Names::FindName(consumer));
  consumerHelper.Install(consumer);

  ndn::AppHelper producerHelper("ns3::ndn::Producer");
  producerHelper.SetAttribute("PayloadSize", StringValue("1024"));

  // Register /sensor prefix with global routing controller and
  // install producer that will satisfy Interests in /sensor namespace
  ndnGlobalRoutingHelper.AddOrigins("/sensor", producer);
  producerHelper.SetPrefix("/sensor");
  producerHelper.Install(producer);

  // Calculate and install FIBs
  ndn::GlobalRoutingHelper::CalculateRoutes();

  Simulator::Stop(Seconds(5.0));

  ndn::L3RateTracer::InstallAll("rate-trace_cabeee-3node.txt", Seconds(0.1));
  ndn::CsTracer::InstallAll("cs-trace_cabeee_simple.txt", Seconds(0.1));

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
