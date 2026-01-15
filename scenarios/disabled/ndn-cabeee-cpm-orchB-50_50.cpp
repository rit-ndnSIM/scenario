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

int
main(int argc, char* argv[])
{
  // Read optional command-line parameters (e.g., enable visualizer with ./waf --run=<> --visualize
  CommandLine cmd;
  cmd.Parse(argc, argv);

  // Creating nodes
  AnnotatedTopologyReader topologyReader("", 1);
  topologyReader.SetFileName("topologies/topo-cabeee-cpm-x_50.txt");
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
  Ptr<Node> rtr021 = Names::Find<Node>("rtr-021");
  Ptr<Node> rtr022 = Names::Find<Node>("rtr-022");
  Ptr<Node> rtr023 = Names::Find<Node>("rtr-023");
  Ptr<Node> rtr024 = Names::Find<Node>("rtr-024");
  Ptr<Node> rtr025 = Names::Find<Node>("rtr-025");
  Ptr<Node> rtr026 = Names::Find<Node>("rtr-026");
  Ptr<Node> rtr027 = Names::Find<Node>("rtr-027");
  Ptr<Node> rtr028 = Names::Find<Node>("rtr-028");
  Ptr<Node> rtr029 = Names::Find<Node>("rtr-029");
  Ptr<Node> rtr030 = Names::Find<Node>("rtr-030");
  Ptr<Node> rtr031 = Names::Find<Node>("rtr-031");
  Ptr<Node> rtr032 = Names::Find<Node>("rtr-032");
  Ptr<Node> rtr033 = Names::Find<Node>("rtr-033");
  Ptr<Node> rtr034 = Names::Find<Node>("rtr-034");
  Ptr<Node> rtr035 = Names::Find<Node>("rtr-035");
  Ptr<Node> rtr036 = Names::Find<Node>("rtr-036");
  Ptr<Node> rtr037 = Names::Find<Node>("rtr-037");
  Ptr<Node> rtr038 = Names::Find<Node>("rtr-038");
  Ptr<Node> rtr039 = Names::Find<Node>("rtr-039");
  Ptr<Node> rtr040 = Names::Find<Node>("rtr-040");
  Ptr<Node> rtr041 = Names::Find<Node>("rtr-041");
  Ptr<Node> rtr042 = Names::Find<Node>("rtr-042");
  Ptr<Node> rtr043 = Names::Find<Node>("rtr-043");
  Ptr<Node> rtr044 = Names::Find<Node>("rtr-044");
  Ptr<Node> rtr045 = Names::Find<Node>("rtr-045");
  Ptr<Node> rtr046 = Names::Find<Node>("rtr-046");
  Ptr<Node> rtr047 = Names::Find<Node>("rtr-047");
  Ptr<Node> rtr048 = Names::Find<Node>("rtr-048");
  Ptr<Node> rtr049 = Names::Find<Node>("rtr-049");
  Ptr<Node> rtr050 = Names::Find<Node>("rtr-050");
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
  ndnHelper.Install(rtr021);
  ndnHelper.Install(rtr022);
  ndnHelper.Install(rtr023);
  ndnHelper.Install(rtr024);
  ndnHelper.Install(rtr025);
  ndnHelper.Install(rtr026);
  ndnHelper.Install(rtr027);
  ndnHelper.Install(rtr028);
  ndnHelper.Install(rtr029);
  ndnHelper.Install(rtr030);
  ndnHelper.Install(rtr031);
  ndnHelper.Install(rtr032);
  ndnHelper.Install(rtr033);
  ndnHelper.Install(rtr034);
  ndnHelper.Install(rtr035);
  ndnHelper.Install(rtr036);
  ndnHelper.Install(rtr037);
  ndnHelper.Install(rtr038);
  ndnHelper.Install(rtr039);
  ndnHelper.Install(rtr040);
  ndnHelper.Install(rtr041);
  ndnHelper.Install(rtr042);
  ndnHelper.Install(rtr043);
  ndnHelper.Install(rtr044);
  ndnHelper.Install(rtr045);
  ndnHelper.Install(rtr046);
  ndnHelper.Install(rtr047);
  ndnHelper.Install(rtr048);
  ndnHelper.Install(rtr049);
  ndnHelper.Install(rtr050);
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
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-011", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-012", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-013", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-014", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-015", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-016", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-017", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-018", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-019", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-020", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-021", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-022", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-023", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-024", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-025", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-026", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-027", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-028", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-029", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-030", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-031", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-032", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-033", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-034", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-035", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-036", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-037", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-038", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-039", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-040", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-041", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-042", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-043", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-044", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-045", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-046", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-047", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-048", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-049", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-050", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/sensor", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceOrchestration", "/localhost/nfd/strategy/multicast");

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
  ndn::AppHelper routerApp("DagServiceB_App");
  routerApp.SetPrefix(Prefix);
  routerApp.SetAttribute("Service", StringValue("serviceCPM-001"));
  routerApp.Install(rtr001).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-002"));
  routerApp.Install(rtr002).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-003"));
  routerApp.Install(rtr003).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-004"));
  routerApp.Install(rtr004).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-005"));
  routerApp.Install(rtr005).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-006"));
  routerApp.Install(rtr006).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-007"));
  routerApp.Install(rtr007).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-008"));
  routerApp.Install(rtr008).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-009"));
  routerApp.Install(rtr009).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-010"));
  routerApp.Install(rtr010).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-011"));
  routerApp.Install(rtr011).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-012"));
  routerApp.Install(rtr012).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-013"));
  routerApp.Install(rtr013).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-014"));
  routerApp.Install(rtr014).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-015"));
  routerApp.Install(rtr015).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-016"));
  routerApp.Install(rtr016).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-017"));
  routerApp.Install(rtr017).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-018"));
  routerApp.Install(rtr018).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-019"));
  routerApp.Install(rtr019).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-020"));
  routerApp.Install(rtr020).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-021"));
  routerApp.Install(rtr021).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-022"));
  routerApp.Install(rtr022).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-023"));
  routerApp.Install(rtr023).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-024"));
  routerApp.Install(rtr024).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-025"));
  routerApp.Install(rtr025).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-026"));
  routerApp.Install(rtr026).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-027"));
  routerApp.Install(rtr027).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-028"));
  routerApp.Install(rtr028).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-029"));
  routerApp.Install(rtr029).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-030"));
  routerApp.Install(rtr030).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-031"));
  routerApp.Install(rtr031).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-032"));
  routerApp.Install(rtr032).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-033"));
  routerApp.Install(rtr033).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-034"));
  routerApp.Install(rtr034).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-035"));
  routerApp.Install(rtr035).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-036"));
  routerApp.Install(rtr036).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-037"));
  routerApp.Install(rtr037).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-038"));
  routerApp.Install(rtr038).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-039"));
  routerApp.Install(rtr039).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-040"));
  routerApp.Install(rtr040).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-041"));
  routerApp.Install(rtr041).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-042"));
  routerApp.Install(rtr042).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-043"));
  routerApp.Install(rtr043).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-044"));
  routerApp.Install(rtr044).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-045"));
  routerApp.Install(rtr045).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-046"));
  routerApp.Install(rtr046).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-047"));
  routerApp.Install(rtr047).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-048"));
  routerApp.Install(rtr048).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-049"));
  routerApp.Install(rtr049).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-050"));
  routerApp.Install(rtr050).Start(Seconds(0));
 
  ndn::AppHelper orchestratorApp("DagOrchestratorB_App");
  orchestratorApp.SetPrefix(Prefix);
  orchestratorApp.SetAttribute("Service", StringValue("serviceOrchestration"));
  //orchestratorApp.Install(orchestrator).Start(Seconds(0));
  orchestratorApp.Install(consumer).Start(Seconds(0));

  // Custom App for User(Consumer)
  ndn::AppHelper userApp("CustomAppConsumer");
  userApp.SetPrefix(Prefix);
  userApp.SetAttribute("Service", StringValue("consumer"));
  userApp.SetAttribute("Workflow", StringValue("workflows/cpm-50_x.json"));
  userApp.SetAttribute("Orchestrate", UintegerValue(2));
  userApp.Install(consumer).Start(Seconds(0));



  Simulator::Stop(Seconds(20.1));


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
