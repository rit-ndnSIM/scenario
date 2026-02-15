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

#include <nlohmann/json.hpp>

using json = nlohmann::json;


namespace ns3 {

int
main(int argc, char* argv[])
{
    std::string scenario_file = "";
    bool verbose = false;

    CommandLine cmd;
    cmd.AddValue("scenario", "json scenario file to use", scenario_file);
    cmd.AddValue("verbose", "increase verbosity", verbose);
    cmd.Parse(argc, argv);

    if (scenario_file == "") {
        std::cerr << "No scenario file specified (use '--scenario FILE')\n";
        std::exit(1);
    }

    const json scenario_json = json::parse(std::ifstream(scenario_file));

    //TODO: do we want to support having the topology described in the scenario_file? Right now we just support it being in a separate file (topofile)
    std::string topofile = scenario_json.at("topofile");

    // Creating nodes
    AnnotatedTopologyReader topologyReader("", 1);
    topologyReader.SetFileName(topofile);
    topologyReader.Read();

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
    int8_t resourceAllocationFlag = 0;
    int8_t allocationReuseFlag = 0;
    int8_t scheduleCompactionFlag = 0;
    if (scenario_json.contains("serviceDiscovery")) {
        serviceDiscoveryFlag = scenario_json.at("serviceDiscovery");
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

    float simulationEndTime = 20;
    if (scenario_json.contains("scheduleCompaction")) {
        simulationEndTime = scenario_json.at("simulationEndTime");
    }
    
    int8_t startTimeOffsetSD = 0;
    int8_t startTimeOffsetWF = 2;
    if (serviceDiscoveryFlag == 1) {
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
            appHelper = ndn::AppHelper("CustomAppProducer");
            appHelper.SetAttribute("Makespan", UintegerValue(makespanNS));
            if (serviceDiscoveryFlag == 1) {
                serviceDiscoveryApp = ndn::AppHelper("DagServiceDiscoveryApp");
                serviceDiscoveryApp.SetPrefix(Prefix);
                serviceDiscoveryApp.SetAttribute("Service", StringValue(srv_name));
                serviceDiscoveryApp.SetAttribute("Makespan", UintegerValue(makespanNS));
                auto sd_app = serviceDiscoveryApp.Install(rtr_node);
                sd_app.Start(Seconds(start));
                if (end > 0)
                    sd_app.Stop(Seconds(end));
            }
            ndnGlobalRoutingHelper.AddOrigins(Prefix + srv_name, rtr_name);
            if (serviceDiscoveryFlag == 1) {
                ndnGlobalRoutingHelper.AddOrigins(Prefix + "/serviceDiscovery" + srv_name, rtr_name);
            }
        } else if (type == "service") {
            if (Prefix == "nesco" || Prefix == "nescoSCOPT"){
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
            if (serviceDiscoveryFlag == 1) {
                serviceDiscoveryApp = ndn::AppHelper("DagServiceDiscoveryApp");
                serviceDiscoveryApp.SetPrefix(Prefix);
                serviceDiscoveryApp.SetAttribute("Service", StringValue(srv_name));
                serviceDiscoveryApp.SetAttribute("Makespan", UintegerValue(makespanNS));
                auto sd_app = serviceDiscoveryApp.Install(rtr_node);
                sd_app.Start(Seconds(start));
                if (end > 0)
                    sd_app.Stop(Seconds(end));
            }
            ndnGlobalRoutingHelper.AddOrigins(Prefix + srv_name, rtr_name);
            if (serviceDiscoveryFlag == 1) {
                ndnGlobalRoutingHelper.AddOrigins(Prefix + "/serviceDiscovery" + srv_name, rtr_name);
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
            if (Prefix == "nesco" || Prefix == "nescoSCOPT") {
                appHelper.SetAttribute("Orchestrate", UintegerValue(0));
                if (serviceDiscoveryFlag == 1) {
                    appHelper.SetAttribute("ServiceDiscovery", UintegerValue(serviceDiscoveryFlag));
                    appHelper.SetAttribute("ResourceAllocation", UintegerValue(resourceAllocationFlag));
                    appHelper.SetAttribute("AllocationReuse", UintegerValue(allocationReuseFlag));
                    appHelper.SetAttribute("ScheduleCompaction", UintegerValue(scheduleCompactionFlag));
                    appHelper.SetAttribute("SDstartTimeOffset", TimeValue(Seconds(startTimeOffsetSD)));
                    appHelper.SetAttribute("WFstartTimeOffset", TimeValue(Seconds(startTimeOffsetWF)));
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
            if (Prefix == "nesco" || Prefix == "nescoSCOPT") {
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
    }

    // Calculate and install FIBs
    ndn::GlobalRoutingHelper::CalculateRoutes();

    Simulator::Stop(Seconds(simulationEndTime));

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
