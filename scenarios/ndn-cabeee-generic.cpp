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

#define SCENARIO_FILE "4dag.json"

namespace ns3 {

int
main(int argc, char* argv[])
{
    // Read optional command-line parameters (e.g., enable visualizer with ./waf --run=<> --visualize
    CommandLine cmd;
    cmd.Parse(argc, argv);

    // eventually get SCENARIO_FILE from args
    std::ifstream f{ SCENARIO_FILE };
    const json scenario_json = json::parse(f);

    // for now, read topofile from scenario json
    // eventually, build topology manually from json
    // or maybe just generate topo file from json
    std::string topofile = scenario_json.at("topofile");

    // Creating nodes
    AnnotatedTopologyReader topologyReader("", 1);
    topologyReader.SetFileName(topofile);
    topologyReader.Read();

    // Install NDN stack on all nodes
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
        // default to 1000
        int cs_size = 1000;
        if (rtr.contains("cs-size")) {
            cs_size = rtr["cs-size"];
        }
        std::cout << "Now setting the CS in router " << (name) << " to this many packets: " << (cs_size) << std::endl;
        ndnHelper.setCsSize(cs_size);
        ndnHelper.Install(node);
    }

    std::string Prefix{ scenario_json.at("prefix") };

    for (const auto& srv : scenario_json.at("services")) {
        std::string strategy{ "/localhost/nfd/strategy/multicast" };
        std::string name = srv.at("name");
        srv_map[name] = &srv;
        if (srv.contains("strategy")) {
            strategy = srv["strategy"];
        }
        std::cout << "Now setting routing strategy for " << (name) << " to " << (strategy) << std::endl;
        ndn::StrategyChoiceHelper::InstallAll(Prefix + name, strategy);
    }

    // Installing global routing interface on all nodes
    //ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
    //ndnGlobalRoutingHelper.InstallAll();

    // Installing applications

    for (const auto& hosting : scenario_json.at("routerHosting")) {
        std::string rtr_name{ hosting.at("router") };
        std::string srv_name{ hosting.at("service") };
        Ptr<Node> rtr_node = Names::Find<Node>(rtr_name);
        hosting_map[rtr_name + srv_name] = &hosting;

        std::string type = srv_map.at(srv_name)->at("type");

        int start = 0;
        //int end = 0;

        if (hosting.contains("start")) {
            start = hosting["start"];
        }
        
        //if (hosting.contains("end")) {
        //    end = hosting["end"];
        //}


        std::cout << "Now installing " << (srv_name) << " in router " << (rtr_name) << ", starting at time: " << start << std::endl;

        if (type == "producer") {
            ndn::AppHelper appHelper("CustomAppProducer");
            appHelper.SetPrefix(Prefix);
            appHelper.SetAttribute("Service", StringValue(srv_name));
            appHelper.Install(rtr_node).Start(Seconds(start));
            //app.Start(Seconds(start));
            //app.Stop(Seconds(end));
        } else if (type == "service") {
            ndn::AppHelper appHelper("DagForwarderApp");
            appHelper.SetPrefix(Prefix);
            appHelper.SetAttribute("Service", StringValue(srv_name));
            appHelper.Install(rtr_node).Start(Seconds(start));
        } else if (type == "consumer") {
            ndn::AppHelper appHelper("CustomAppConsumer");
            appHelper.SetPrefix(Prefix);
            appHelper.SetAttribute("Service", StringValue(srv_name));
            appHelper.SetAttribute("Workflow", StringValue(SCENARIO_FILE));
            appHelper.SetAttribute("Orchestrate", UintegerValue(0));
            appHelper.Install(rtr_node).Start(Seconds(start));
        } 
    }

    Simulator::Stop(Seconds(3.0));

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
