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


class PcapWriter {
public:
  PcapWriter(const std::string& file)
  {
    PcapHelper helper;
    m_pcap = helper.CreateFile(file, std::ios::out, PcapHelper::DLT_PPP);
  }

  void
  TracePacket(Ptr<const Packet> packet)
  {
    static PppHeader pppHeader;
    pppHeader.SetProtocol(0x0077);

    m_pcap->Write(Simulator::Now(), pppHeader, packet);
  }

private:
  Ptr<PcapFileWrapper> m_pcap;
};

int
main(int argc, char* argv[])
{
  // Read optional command-line parameters (e.g., enable visualizer with ./waf --run=<> --visualize
  CommandLine cmd;
  cmd.Parse(argc, argv);

  // Creating nodes
  AnnotatedTopologyReader topologyReader("", 1);
  topologyReader.SetFileName("topologies/topo-cabeee-3node-fwdOpt.txt");
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

  ndnHelper.setCsSize(0); // content store size (0 disables it)
  ndnHelper.Install(producer);

  ndnHelper.setCsSize(0); // content store size (0 disables it)
  ndnHelper.Install(router1);
  ndnHelper.Install(router2);
  ndnHelper.Install(router3);


  ndnHelper.setCsSize(0); // content store size (0 disables it)
  ndnHelper.Install(consumer);



  std::string Prefix(PREFIX);

  // Choosing forwarding strategy
  //ndn::StrategyChoiceHelper::InstallAll("/prefix", "/localhost/nfd/strategy/best-route");
  //ndn::StrategyChoiceHelper::InstallAll("/prefix", "/localhost/nfd/strategy/multicast");

  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/service4", "/localhost/nfd/strategy/best-route");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/service3", "/localhost/nfd/strategy/best-route");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/service2", "/localhost/nfd/strategy/best-route");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/service1", "/localhost/nfd/strategy/best-route");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/sensor",   "/localhost/nfd/strategy/best-route");

  //ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceDiscovery", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceDiscovery", "/localhost/nfd/strategy/best-route");

  //ndn::StrategyChoiceHelper::InstallAll(Prefix + "/service4/serviceDiscovery", "/localhost/nfd/strategy/multicast"); these are no longer in this order. Now we do /serviceDiscovery/serviceX
  //ndn::StrategyChoiceHelper::InstallAll(Prefix + "/service3/serviceDiscovery", "/localhost/nfd/strategy/multicast"); these are no longer in this order. Now we do /serviceDiscovery/serviceX
  //ndn::StrategyChoiceHelper::InstallAll(Prefix + "/service2/serviceDiscovery", "/localhost/nfd/strategy/multicast"); these are no longer in this order. Now we do /serviceDiscovery/serviceX
  //ndn::StrategyChoiceHelper::InstallAll(Prefix + "/service1/serviceDiscovery", "/localhost/nfd/strategy/multicast"); these are no longer in this order. Now we do /serviceDiscovery/serviceX
  //ndn::StrategyChoiceHelper::InstallAll(Prefix + "/sensor/serviceDiscovery", "/localhost/nfd/strategy/multicast"); these are no longer in this order. Now we do /serviceDiscovery/serviceX

  //ndn::StrategyChoiceHelper::InstallAll(Prefix + "/service4", "/localhost/nfd/strategy/multicast");
  //ndn::StrategyChoiceHelper::InstallAll(Prefix + "/service3", "/localhost/nfd/strategy/multicast");
  //ndn::StrategyChoiceHelper::InstallAll(Prefix + "/service2", "/localhost/nfd/strategy/multicast");
  //ndn::StrategyChoiceHelper::InstallAll(Prefix + "/service1", "/localhost/nfd/strategy/multicast");
  //ndn::StrategyChoiceHelper::InstallAll(Prefix + "/sensor", "/localhost/nfd/strategy/multicast");




  // Installing global routing interface on all nodes
  ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
  ndnGlobalRoutingHelper.InstallAll();


 


  // Installing applications

  // Custom App for Sensor(Producer)
  ndn::AppHelper sensorApp("CustomAppProducer");
  sensorApp.SetPrefix(Prefix);
  sensorApp.SetAttribute("Service", StringValue("sensor"));
  sensorApp.Install(producer).Start(Seconds(0));

  // Custom App for Services and Forwarding
  ndn::AppHelper routerApp("DagForwarderApp");
  routerApp.SetPrefix(Prefix);

  routerApp.SetAttribute("Service", StringValue("service1"));
  routerApp.Install(producer).Start(Seconds(0));
  routerApp.Install(router3).Start(Seconds(0));

  routerApp.SetAttribute("Service", StringValue("service2"));
  routerApp.Install(router1).Start(Seconds(0));

  routerApp.SetAttribute("Service", StringValue("service3"));
  routerApp.Install(router1).Start(Seconds(0));
  routerApp.Install(router2).Start(Seconds(0));

  routerApp.SetAttribute("Service", StringValue("service4"));
  routerApp.Install(router2).Start(Seconds(0));

  // Custom App for ServiceDiscovery
  ndn::AppHelper serviceDiscoveryApp("DagServiceDiscoveryApp");
  serviceDiscoveryApp.SetPrefix(Prefix);

  serviceDiscoveryApp.SetAttribute("Service", StringValue("sensor"));
  serviceDiscoveryApp.Install(producer).Start(Seconds(0));
  serviceDiscoveryApp.SetAttribute("Service", StringValue("service1"));
  serviceDiscoveryApp.Install(producer).Start(Seconds(0));
  serviceDiscoveryApp.Install(router3).Start(Seconds(0));
  serviceDiscoveryApp.SetAttribute("Service", StringValue("service2"));
  serviceDiscoveryApp.Install(router1).Start(Seconds(0));
  serviceDiscoveryApp.SetAttribute("Service", StringValue("service3"));
  serviceDiscoveryApp.Install(router1).Start(Seconds(0));
  serviceDiscoveryApp.Install(router2).Start(Seconds(0));
  serviceDiscoveryApp.SetAttribute("Service", StringValue("service4"));
  serviceDiscoveryApp.Install(router2).Start(Seconds(0));

  // Custom App for User(Consumer)
  //ndn::AppHelper userApp("CustomAppConsumer");
  ndn::AppHelper userApp("CustomAppConsumerServiceDiscovery");
  userApp.SetPrefix(Prefix);

  userApp.SetAttribute("Service", StringValue("consumer"));
  userApp.SetAttribute("Workflow", StringValue("workflows/4dag.json"));
  userApp.SetAttribute("Orchestrate", UintegerValue(0));
  userApp.SetAttribute("ServiceDiscovery", UintegerValue(1));
  userApp.SetAttribute("ResourceAllocation", UintegerValue(0));
  userApp.SetAttribute("AllocationReuse", UintegerValue(0));
  userApp.SetAttribute("ScheduleCompaction", UintegerValue(0));
  userApp.SetAttribute("SDstartTime", TimeValue(Seconds(1)));
  userApp.SetAttribute("WFstartTime", TimeValue(Seconds(2)));
  userApp.Install(consumer).Start(Seconds(0));


  // Register all service prefix names to their locations
  ndnGlobalRoutingHelper.AddOrigins(Prefix + "/sensor", producer);
  ndnGlobalRoutingHelper.AddOrigins(Prefix + "/service1", producer);
  ndnGlobalRoutingHelper.AddOrigins(Prefix + "/service1", router3);
  ndnGlobalRoutingHelper.AddOrigins(Prefix + "/service2", router1);
  ndnGlobalRoutingHelper.AddOrigins(Prefix + "/service3", router1);
  ndnGlobalRoutingHelper.AddOrigins(Prefix + "/service3", router2);
  ndnGlobalRoutingHelper.AddOrigins(Prefix + "/service4", router2);

  ndnGlobalRoutingHelper.AddOrigins(Prefix + "/serviceDiscovery/sensor", producer);
  ndnGlobalRoutingHelper.AddOrigins(Prefix + "/serviceDiscovery/service1", producer);
  ndnGlobalRoutingHelper.AddOrigins(Prefix + "/serviceDiscovery/service1", router3);
  ndnGlobalRoutingHelper.AddOrigins(Prefix + "/serviceDiscovery/service2", router1);
  ndnGlobalRoutingHelper.AddOrigins(Prefix + "/serviceDiscovery/service3", router1);
  ndnGlobalRoutingHelper.AddOrigins(Prefix + "/serviceDiscovery/service3", router2);
  ndnGlobalRoutingHelper.AddOrigins(Prefix + "/serviceDiscovery/service4", router2);



  // Calculate and install FIBs
  ndn::GlobalRoutingHelper::CalculateRoutes();

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


  PcapWriter trace("ndn-cabeee-4dag-nesco-trace.pcap");
  Config::ConnectWithoutContext("/NodeList/*/DeviceList/*/$ns3::PointToPointNetDevice/MacTx",
                                MakeCallback(&PcapWriter::TracePacket, &trace));

  Simulator::Stop(Seconds(20.1)); // pick a large value, the consumer will end the simulation as soon as the workflow data packet is received.

  ndn::L3RateTracer::InstallAll("rate-trace_cabeee-fwdOpt-4dag-nesco-SD-noAllocation.txt", Seconds(0.0005));
  ndn::CsTracer::InstallAll("cs-trace_cabeee-fwdOpt-4dag-nesco-SD-noAllocation.txt", Seconds(0.0005));

  Simulator::Run();
  Simulator::Destroy();

  //tshark -r ndn-cabeee-4dag-nesco-trace.pcap -z io,stat,0,"SUM(frame.len)frame.len" // this prints out total number of bytes in the pcap file


  return 0;
}

} // namespace ns3

int
main(int argc, char* argv[])
{
  return ns3::main(argc, argv);
}
