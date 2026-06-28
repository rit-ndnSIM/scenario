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
//#include "ns3/point-to-point-module.h" // added for pCap generation

#include <filesystem>
#include "cabeee-cs-tools.h"

#include <nlohmann/json.hpp>

using json = nlohmann::json;


namespace ns3 {

/*
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
*/


int
main(int argc, char* argv[])
{
    std::string scenario_file = "";
    std::string trace_dir = "";
    bool verbose = false;
    float overrideTrace = 0;
    uint64_t overrideMakespan = 0;
    uint64_t overrideFreshness = 0;

    CommandLine cmd;
    cmd.AddValue("scenario", "json scenario file to use", scenario_file);
    cmd.AddValue("traceDir", "trace directory where to store trace files", trace_dir);
    cmd.AddValue("verbose", "increase verbosity", verbose);
    cmd.AddValue("overrideTrace", "override rateTrace, csTrace and csUsage interval to this number of seconds", overrideTrace);
    cmd.AddValue("overrideMakespan", "override service makespan value to this number of nanoseconds", overrideMakespan);
    cmd.AddValue("overrideFreshness", "override data packet freshness value to this number of seconds", overrideFreshness);
    cmd.Parse(argc, argv);

    if (scenario_file == "") {
        std::cerr << "No scenario file specified (use '--scenario FILE')\n";
        std::exit(1);
    }

    if (trace_dir == "") {
        std::cerr << "No trace directory specified (use '--traceDir PATH')\n";
        std::exit(1);
    }

    if (verbose)
    {
        std::cout << "Trace directory is: " << trace_dir << std::endl;
        std::cout << "Trace override is: " << overrideTrace << std::endl;
        std::cout << "Makespan override is: " << overrideMakespan << std::endl;
        std::cout << "Freshness override is: " << overrideFreshness << std::endl;
    }

    const json scenario_json = json::parse(std::ifstream(scenario_file));

    //TODO: do we want to support having the topology described in the scenario_file? Right now we just support it being in a separate file (topofile)
    std::string topofile = scenario_json.at("topofile");

    // Creating nodes
    AnnotatedTopologyReader topologyReader("", 1);
    topologyReader.SetFileName(topofile);
    topologyReader.Read();

    if (scenario_json.contains("poissonConsumerNumInterests")) {
        if (scenario_json.at("poissonConsumerNumInterests") > 1) {
            std::cout << "Poisson consumer is being used. Number of interests = " << scenario_json.at("poissonConsumerNumInterests") << ", frequency = " << scenario_json.at("poissonConsumerFrequency") << " interests per second." << std::endl;
        }
    }

    // Install NDN stack on all nodes
    // TODO: customize policy per router?
    ndn::StackHelper ndnHelper;
    ndnHelper.setPolicy("nfd::cs::lru");
    ndnHelper.SetDefaultRoutes(true);

    map<std::string, const json*> srv_map{};
    map<std::string, const json*> rtr_map{};
    map<std::string, const json*> hosting_map{};

    for (const auto& rtr : scenario_json.at("router")) {
        std::string name{ rtr.at("node") };
        Ptr<Node> node = Names::Find<Node>(name);
        rtr_map[name] = &rtr;
        int cs_size = 0; // set default size (in case json doesn't specify size)
        if (rtr.contains("cs-size")) {
            cs_size = rtr["cs-size"];
        }
        if (verbose)
            std::cout << "Now setting the CS in router " << (name) << " to this many packets: " << (cs_size) << std::endl;
        ndnHelper.setCsSize(cs_size);
        ndnHelper.Install(node);
    }

    std::string Prefix = scenario_json.at("prefix");

    int8_t serviceDiscoveryFlag = 0;
    int8_t resourceUtilizationFlag = 0;
    int8_t resourceAllocationFlag = 0;
    int8_t allocationReuseFlag = 0;
    int8_t scheduleCompactionFlag = 0;
    int8_t producerFreshnessUniformDist = 0;
    uint64_t producerFreshnessMSmin = 0;
    uint64_t producerFreshnessMSmax = 0;
    uint64_t producerFreshnessMS = 60000;
    if (scenario_json.contains("serviceDiscovery")) {
        serviceDiscoveryFlag = scenario_json.at("serviceDiscovery");
    }
    if (scenario_json.contains("resourceUtilization")) {
        resourceUtilizationFlag = scenario_json.at("resourceUtilization");
    }
    if (scenario_json.contains("resourceAllocation")) {
        resourceAllocationFlag = scenario_json.at("resourceAllocation");
    }
    if (scenario_json.contains("allocationReuse")) {
        allocationReuseFlag = scenario_json.at("allocationReuse");
    }
    if (scenario_json.contains("scheduleCompaction")) {
        scheduleCompactionFlag = scenario_json.at("scheduleCompaction");
    }
    if (scenario_json.contains("producerFreshnessUniformDist")) {
        producerFreshnessUniformDist = scenario_json.at("producerFreshnessUniformDist");
    }
    if (scenario_json.contains("producerFreshnessMSmin")) {
        producerFreshnessMSmin = scenario_json.at("producerFreshnessMSmin");
    }
    if (scenario_json.contains("producerFreshnessMSmax")) {
        producerFreshnessMSmax = scenario_json.at("producerFreshnessMSmax");
    }
    if (scenario_json.contains("producerFreshnessMS")) {
        producerFreshnessMS = scenario_json.at("producerFreshnessMS");
    }

    float simulationEndTime = 1000; // set default end time (in case json doesn't specify)
    if (scenario_json.contains("scheduleCompaction")) {
        simulationEndTime = scenario_json.at("simulationEndTime");
    }
    
    uint64_t startTimeOffsetSD = 0;
    uint64_t startTimeOffsetWF = 2;
    if (serviceDiscoveryFlag > 0) {
        startTimeOffsetSD = scenario_json.at("startTimeOffsetSD");
        startTimeOffsetWF = scenario_json.at("startTimeOffsetWF");
    }

    for (const auto& srv : scenario_json.at("services")) {
        std::string strategy{ "/localhost/nfd/strategy/multicast" }; // set default strategy (in case json doesn't specify)
        std::string name = srv.at("name");
        srv_map[name] = &srv;
        if (srv.contains("strategy")) {
            strategy = srv["strategy"];
        }
        if (verbose)
            std::cout << "Now setting routing strategy for " << (Prefix + name) << " to " << (strategy) << std::endl;
        ndn::StrategyChoiceHelper::InstallAll(Prefix + name, strategy);
    }

    if (Prefix == "orchA" || Prefix == "orchB") {
        if (verbose)
            std::cout << "Now setting routing strategy for " << Prefix << "/serviceOrchestration to /localhost/nfd/strategy/multicast" << std::endl;
        ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceOrchestration", "/localhost/nfd/strategy/multicast");
    }

    // Installing global routing interface on all nodes
    ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
    ndnGlobalRoutingHelper.InstallAll();

    // Installing applications
    uint64_t makespanNS = 0;
    for (const auto& hosting : scenario_json.at("routerHosting")) {
        std::string rtr_name{ hosting.at("router") };
        std::string srv_name{ hosting.at("service") };
        if (hosting.contains("makespanNS")) {
            makespanNS = hosting["makespanNS"];
        }
        if (overrideMakespan > 0)
        {
            makespanNS = overrideMakespan;
        }
        Ptr<Node> rtr_node = Names::Find<Node>(rtr_name);
        hosting_map[rtr_name + srv_name] = &hosting;

        std::string type{};
        if (srv_map.count(srv_name)) {
            type = srv_map.at(srv_name)->at("type");
        } else {
            std::cerr << "Service " << srv_name << " not found\n";
            std::exit(EXIT_FAILURE);
        }

        double start = 0;
        double end = -1;

        if (hosting.contains("start")) {
            start = hosting["start"];
        }
        
        if (hosting.contains("end")) {
            end = hosting["end"];
        }

        if (verbose) {
            std::cout << "Now installing " << (srv_name) << " in router " << (rtr_name) << ", starting at time: " << start << std::endl;
            if (hosting.contains("makespanNS")) {
                std::cout << "Makespan is " << makespanNS << std::endl;
            }
            if (hosting.contains("workflowFile")) {
                std::cout << "workflowFile is: " << (hosting["workflowFile"]) << std::endl;
            }
        }

        ndn::AppHelper appHelper("DagForwarderApp");
        ndn::AppHelper serviceDiscoveryApp("DagServiceDiscoveryApp");

        if (type == "producer") {
            if (overrideFreshness > 0)
            {
                producerFreshnessMS = overrideFreshness;
                producerFreshnessUniformDist = 0;
                producerFreshnessMSmin = 0;
                producerFreshnessMSmax = 0;
            }
            appHelper = ndn::AppHelper("CustomAppProducer");
            appHelper.SetAttribute("Makespan", UintegerValue(makespanNS));
            appHelper.SetAttribute("FreshnessPeriod_ms", UintegerValue(producerFreshnessMS));
            appHelper.SetAttribute("UniformFreshness", UintegerValue(producerFreshnessUniformDist));
            appHelper.SetAttribute("minFreshness_ms", UintegerValue(producerFreshnessMSmin));
            appHelper.SetAttribute("maxFreshness_ms", UintegerValue(producerFreshnessMSmax));
            if (serviceDiscoveryFlag > 0) {
                serviceDiscoveryApp = ndn::AppHelper("DagServiceDiscoveryApp");
                serviceDiscoveryApp.SetPrefix(Prefix);
                if (serviceDiscoveryFlag == 1) {
                    serviceDiscoveryApp.SetAttribute("SDName", StringValue("/serviceDiscovery"));
                }
                if (serviceDiscoveryFlag == 2) {
                    serviceDiscoveryApp.SetAttribute("SDName", StringValue("/serviceDiscovery2"));
                }
                serviceDiscoveryApp.SetAttribute("Service", StringValue(srv_name));
                serviceDiscoveryApp.SetAttribute("Makespan", UintegerValue(makespanNS));
                if (verbose) {
                    std::cout << "Now installing ServiceDiscoveryApp in router " << (rtr_name) << ", starting at time: " << start << std::endl;
                }
                auto sd_app = serviceDiscoveryApp.Install(rtr_node);
                sd_app.Start(Seconds(start));
                if (end > 0)
                    sd_app.Stop(Seconds(end));
            }
            ndnGlobalRoutingHelper.AddOrigins(Prefix + srv_name, rtr_name);
            if (serviceDiscoveryFlag == 1) {
                ndnGlobalRoutingHelper.AddOrigins(Prefix + "/serviceDiscovery" + srv_name, rtr_name);
            }
            if (serviceDiscoveryFlag == 2) {
                ndnGlobalRoutingHelper.AddOrigins(Prefix + "/serviceDiscovery2" + srv_name, rtr_name);
            }
        } else if (type == "service") {
            if (Prefix == "nesco" || Prefix == "nescoSCOPT" ||
                Prefix == "icnfc" || Prefix == "ndnfcp" || Prefix == "or3"){
                appHelper = ndn::AppHelper("DagForwarderApp");
                serviceDiscoveryApp = ndn::AppHelper("DagServiceDiscoveryApp");
            }
            if (Prefix == "orchA") {
                appHelper = ndn::AppHelper("DagServiceA_App");
            }
            if (Prefix == "orchB") {
                appHelper = ndn::AppHelper("DagServiceB_App");
            }
            appHelper.SetAttribute("Makespan", UintegerValue(makespanNS));
            if (serviceDiscoveryFlag > 0) {
                serviceDiscoveryApp = ndn::AppHelper("DagServiceDiscoveryApp");
                serviceDiscoveryApp.SetPrefix(Prefix);
                if (serviceDiscoveryFlag == 1) {
                    serviceDiscoveryApp.SetAttribute("SDName", StringValue("/serviceDiscovery"));
                }
                if (serviceDiscoveryFlag == 2) {
                    serviceDiscoveryApp.SetAttribute("SDName", StringValue("/serviceDiscovery2"));
                }
                serviceDiscoveryApp.SetAttribute("Service", StringValue(srv_name));
                serviceDiscoveryApp.SetAttribute("Makespan", UintegerValue(makespanNS));
                if (verbose) {
                    std::cout << "Now installing ServiceDiscoveryApp in router " << (rtr_name) << ", starting at time: " << start << std::endl;
                }
                auto sd_app = serviceDiscoveryApp.Install(rtr_node);
                sd_app.Start(Seconds(start));
                if (end > 0)
                    sd_app.Stop(Seconds(end));
            }
            ndnGlobalRoutingHelper.AddOrigins(Prefix + srv_name, rtr_name);
            if (serviceDiscoveryFlag == 1) {
                ndnGlobalRoutingHelper.AddOrigins(Prefix + "/serviceDiscovery" + srv_name, rtr_name);
            }
            if (serviceDiscoveryFlag == 2) {
                ndnGlobalRoutingHelper.AddOrigins(Prefix + "/serviceDiscovery2" + srv_name, rtr_name);
            }
        } else if (type == "consumer") {
            //TODO: do we want to support having the DAG workflow in the scenario_file? Right now we just support it being in a separate file (workflowFile)
            std::string workflow_file = "";
            if (hosting.contains("workflowFile")) {
                workflow_file = hosting["workflowFile"];
            } else {
                std::cerr << "No workflow file specified for consumer " << (srv_name) << ". (make sure you use '--scenario FILE' and this file has workflowFile specified)\n";
                std::exit(1);
            }
            //appHelper = ndn::AppHelper("CustomAppConsumer");
            appHelper = ndn::AppHelper("CustomAppConsumerServiceDiscovery");
            appHelper.SetAttribute("Workflow", StringValue(workflow_file));
            if (Prefix == "nesco" || Prefix == "nescoSCOPT" ||
                Prefix == "icnfc" || Prefix == "ndnfcp" || Prefix == "or3"){
                appHelper.SetAttribute("Orchestrate", UintegerValue(0));
                if (serviceDiscoveryFlag > 0) {
                    if (serviceDiscoveryFlag == 1) {
                        appHelper.SetAttribute("SDName", StringValue("/serviceDiscovery"));
                        appHelper.SetAttribute("ResourceUtilization", UintegerValue(resourceUtilizationFlag));
                        appHelper.SetAttribute("ResourceAllocation", UintegerValue(resourceAllocationFlag));
                        appHelper.SetAttribute("AllocationReuse", UintegerValue(allocationReuseFlag));
                        appHelper.SetAttribute("ScheduleCompaction", UintegerValue(scheduleCompactionFlag));
                    }
                    if (serviceDiscoveryFlag == 2) {
                        appHelper.SetAttribute("SDName", StringValue("/serviceDiscovery2"));
                    }
                    appHelper.SetAttribute("SDstartTimeOffset", TimeValue(Seconds(startTimeOffsetSD)));
                    appHelper.SetAttribute("WFstartTimeOffset", TimeValue(Seconds(startTimeOffsetWF)));
                }
                appHelper.SetAttribute("ServiceDiscovery", UintegerValue(serviceDiscoveryFlag));
                if (scenario_json.contains("poissonConsumerFrequency")) {
                    if (verbose) {
                        std::cout << "Now setting poisson consumer frequency for " << (srv_name) << " to " << scenario_json.at("poissonConsumerFrequency") << std::endl;
                    }
                    appHelper.SetAttribute("Frequency", DoubleValue(scenario_json.at("poissonConsumerFrequency")));
                }
                if (scenario_json.contains("poissonConsumerNumInterests")) {
                    appHelper.SetAttribute("NumInterests", UintegerValue(scenario_json.at("poissonConsumerNumInterests")));
                }
            }
            if (Prefix == "orchA") {
                appHelper.SetAttribute("Orchestrate", UintegerValue(1));
                // now install the orchestrator
                if (verbose) {
                    std::cout << "Now installing orchestratorA in router " << (rtr_name) << ", starting at time: 0" << std::endl;
                }
                ndn::AppHelper orchestratorAppA("DagOrchestratorA_App");
                orchestratorAppA.SetPrefix(Prefix);
                orchestratorAppA.SetAttribute("Service", StringValue("serviceOrchestration"));
                orchestratorAppA.Install(rtr_node).Start(Seconds(0));
            }
            if (Prefix == "orchB") {
                appHelper.SetAttribute("Orchestrate", UintegerValue(2));
                // now install the orchestrator
                if (verbose) {
                    std::cout << "Now installing orchestratorB in router " << (rtr_name) << ", starting at time: 0" << std::endl;
                }
                ndn::AppHelper orchestratorAppB("DagOrchestratorB_App");
                orchestratorAppB.SetPrefix(Prefix);
                orchestratorAppB.SetAttribute("Service", StringValue("serviceOrchestration"));
                orchestratorAppB.Install(rtr_node).Start(Seconds(0));
            }
        } else if (type == "consumer2") {
            //TODO: do we want to support having the DAG workflow in the scenario_file? Right now we just support it being in a separate file (workflowFile)
            std::string workflow_file = "";
            if (hosting.contains("workflowFile")) {
                workflow_file = hosting["workflowFile"];
            } else {
                std::cerr << "No workflow file specified for consumer " << (srv_name) << ". (make sure you use '--scenario FILE' and this file has workflowFile specified)\n";
                std::exit(1);
            }
            appHelper = ndn::AppHelper("CustomAppConsumer2");
            appHelper.SetAttribute("Workflow", StringValue(workflow_file));
            if (Prefix == "nesco" || Prefix == "nescoSCOPT" ||
                Prefix == "icnfc" || Prefix == "ndnfcp" || Prefix == "or3"){
                appHelper.SetAttribute("Orchestrate", UintegerValue(0));
            }
            if (Prefix == "orchA") {
                appHelper.SetAttribute("Orchestrate", UintegerValue(1));
                // now install the orchestrator
                if (verbose) {
                    std::cout << "Now installing orchestratorA in router " << (rtr_name) << ", starting at time: 0" << std::endl;
                }
                ndn::AppHelper orchestratorAppA("DagOrchestratorA_App");
                orchestratorAppA.SetPrefix(Prefix);
                orchestratorAppA.SetAttribute("Service", StringValue("serviceOrchestration"));
                orchestratorAppA.Install(rtr_node).Start(Seconds(0));
            }
            if (Prefix == "orchB") {
                appHelper.SetAttribute("Orchestrate", UintegerValue(2));
                // now install the orchestrator
                if (verbose) {
                    std::cout << "Now installing orchestratorB in router " << (rtr_name) << ", starting at time: 0" << std::endl;
                }
                ndn::AppHelper orchestratorAppB("DagOrchestratorB_App");
                orchestratorAppB.SetPrefix(Prefix);
                orchestratorAppB.SetAttribute("Service", StringValue("serviceOrchestration"));
                orchestratorAppB.Install(rtr_node).Start(Seconds(0));
            }
        } else {
            std::cerr << "Unknown service type '" << type << "'\n";
            std::exit(EXIT_FAILURE);
        }

        appHelper.SetAttribute("Service", StringValue(srv_name));
        appHelper.SetPrefix(Prefix);
        auto srv_app = appHelper.Install(rtr_node);

        srv_app.Start(Seconds(start));
        if (end > 0)
            srv_app.Stop(Seconds(end));
        
    } // end for (const auto& hosting : scenario_json.at("routerHosting"))

    // Calculate and install FIBs
    ndn::GlobalRoutingHelper::CalculateRoutes();

    //PcapWriter trace("ndn-cabeee-4dag-nesco-trace.pcap");
    //Config::ConnectWithoutContext("/NodeList/*/DeviceList/*/$ns3::PointToPointNetDevice/MacTx", MakeCallback(&PcapWriter::TracePacket, &trace));

    Simulator::Stop(Seconds(simulationEndTime));



    float rateTraceInterval = 0;
    float csTraceInterval = 0;
    float csUsageInterval = 0;
    if (scenario_json.contains("rateTrace")) {
        rateTraceInterval = scenario_json.at("rateTrace");
    }
    if (scenario_json.contains("csTrace")) {
        csTraceInterval = scenario_json.at("csTrace");
    }
    if (scenario_json.contains("csUsage")) {
        csUsageInterval = scenario_json.at("csUsage");
    }
    if (overrideTrace > 0)
    {
        rateTraceInterval = overrideTrace;
        csTraceInterval = overrideTrace;
        csUsageInterval = overrideTrace;
    }
    std::string baseName = std::filesystem::path(scenario_file).stem().string();
    if (rateTraceInterval != 0) {
        std::string rTraceFileName = trace_dir + "/rate-trace_" + baseName + ".txt";
        ndn::L3RateTracer::InstallAll(rTraceFileName, Seconds(rateTraceInterval));
        std::cout << "Rate Trace Interval IS set. Filename is " << rTraceFileName << std::endl;
    }
    else{
        std::cout << "Rate Trace Interval NOT set" << std::endl;
    }
    if (csTraceInterval != 0) {
        std::string csTraceFileName = trace_dir + "/cs-trace_" + baseName + ".txt";
        ndn::CsTracer::InstallAll(csTraceFileName, Seconds(csTraceInterval));
        std::cout << "CS Trace Interval IS set. Filename is " << csTraceFileName << std::endl;
    }
    else{
        std::cout << "CS Trace Interval NOT set" << std::endl;
    }
    if (csUsageInterval != 0) {
        std::string csUsageFileName = trace_dir + "/cs-usage_" + baseName + ".txt";
        std::ofstream fout(csUsageFileName);
        fout << "testing" << std::endl;
        Simulator::Schedule(Seconds(0), &ns3::printCsHeader, ref(fout));
        Simulator::Schedule(Seconds(0), &ns3::printCsUsage, ref(fout), Seconds(csUsageInterval), Prefix); // record CS usage every 0.5 seconds
        //Simulator::Schedule(Seconds(0), &ns3::printCsHeader, ref(std::cout));
        //Simulator::Schedule(Seconds(0), &ns3::printCsUsage, ref(std::cout), Seconds(0.5), Prefix);
        std::cout << "CS Usage Interval IS set. Filename is " << csUsageFileName << std::endl;
    }
    else{
        std::cout << "CS Usage Interval NOT set" << std::endl;
    }

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
