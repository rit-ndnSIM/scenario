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

// cabeee-custom-app-consumerServiceDiscovery.cpp

#include "cabeee-custom-app-consumerServiceDiscovery.hpp"
//#include "cabeee-dag-forwarder-common.hpp"

#include "ns3/ptr.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "ns3/string.h"

#include "ns3/ndnSIM/helper/ndn-stack-helper.hpp"
#include "ns3/ndnSIM/helper/ndn-fib-helper.hpp"

#include "ns3/random-variable-stream.h"


#include <fstream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

//#include "ns3/integer.h"
#include "ns3/uinteger.h"

//#include "ns3/ndnSIM/ndn-cxx/ndn-cxx/encoding/encoder.hpp"
//#include "ns3/ndnSIM/ndn-cxx/ndn-cxx/encoding/block.hpp"
//#include "ns3/ndnSIM/ndn-cxx/encoding/tlv.hpp"
//#include "ns3/ndnSIM/ndn-cxx/encoding/block-helpers.hpp"


NS_LOG_COMPONENT_DEFINE("CustomAppConsumerServiceDiscovery");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED(CustomAppConsumerServiceDiscovery);

// register NS-3 type
TypeId
CustomAppConsumerServiceDiscovery::GetTypeId()
{
  static TypeId tid = TypeId("CustomAppConsumerServiceDiscovery")
    .SetParent<ndn::App>()
    .AddConstructor<CustomAppConsumerServiceDiscovery>()
    .AddAttribute("Prefix", "Requested prefix", StringValue("/dumb-interest"),
                    ndn::MakeNameAccessor(&CustomAppConsumerServiceDiscovery::m_prefix), ndn::MakeNameChecker())
    .AddAttribute("Service", "Requested service", StringValue("/dumb-interest"),
                    ndn::MakeNameAccessor(&CustomAppConsumerServiceDiscovery::m_service), ndn::MakeNameChecker())
    .AddAttribute("Workflow", "Requested workflow", StringValue("/workflows/dummy-workflow"),
                    MakeStringAccessor(&CustomAppConsumerServiceDiscovery::m_dagPath), MakeStringChecker())
    .AddAttribute("Orchestrate", "Requested orchestration", UintegerValue(0),
                    MakeUintegerAccessor(&CustomAppConsumerServiceDiscovery::m_orchestrate), MakeUintegerChecker<uint16_t>())
    .AddAttribute("FwdOpt", "Requested forwarding optimization", UintegerValue(0),
                    MakeUintegerAccessor(&CustomAppConsumerServiceDiscovery::m_fwdOpt), MakeUintegerChecker<uint16_t>())
    .AddAttribute("SDstartTime", "Requested forwarding optimization", TimeValue(Seconds(1)),
                    MakeTimeAccessor(&CustomAppConsumerServiceDiscovery::m_SDstartTime), MakeTimeChecker())
    .AddAttribute("WFstartTime", "Requested forwarding optimization", TimeValue(Seconds(2)),
                    MakeTimeAccessor(&CustomAppConsumerServiceDiscovery::m_WFstartTime), MakeTimeChecker());
  return tid;
}

CustomAppConsumerServiceDiscovery::CustomAppConsumerServiceDiscovery()
  : m_isRunning(false)
{
}

// Processing upon start of the application
void
CustomAppConsumerServiceDiscovery::StartApplication()
{
  // initialize ndn::App
  ndn::App::StartApplication();
  m_isRunning = true;
  m_SDrunning = false;

  // Add entry to FIB for `/prefix/sub`
  //ndn::FibHelper::AddRoute(GetNode(), "/prefix/sub", m_face, 0); //cabeee took this out, let the global router figure it out.
  // cabeee - I think this means: if any interest exists for /prefix/sub, forward to this application's
  // face, and install the forwarding rule in the node where this application is running.
  // This is why we must remove the line, otherwise the interest is simply fowarded back into the app???
  // Although I don't see the log entry for receiving an interest.


  // Schedule send of first interest
  if (m_fwdOpt == 0) // no service discovery
  {
    Simulator::Schedule(m_WFstartTime, &CustomAppConsumerServiceDiscovery::SendInterest, this);
  }
  if (m_fwdOpt == 1 || m_fwdOpt == 2) // run service discovery before running workflow (1: no CPU allocation, 2: use CPU allocation)
  {
    Simulator::Schedule(m_SDstartTime, &CustomAppConsumerServiceDiscovery::SendSDInterest, this);
    Simulator::Schedule(m_WFstartTime, &CustomAppConsumerServiceDiscovery::SendInterest, this);
  }
  //Simulator::Schedule(Seconds(2.0), &CustomAppConsumerServiceDiscovery::SendInterest, this);
  //Simulator::Schedule(Seconds(3.0), &CustomAppConsumerServiceDiscovery::SendInterest, this);
  //Simulator::Schedule(Seconds(4.0), &CustomAppConsumerServiceDiscovery::SendInterest, this);
  //Simulator::Schedule(Seconds(5.0), &CustomAppConsumerServiceDiscovery::SendInterest, this);
  //Simulator::Schedule(Seconds(6.0), &CustomAppConsumerServiceDiscovery::SendInterest, this);
  //Simulator::Schedule(Seconds(7.0), &CustomAppConsumerServiceDiscovery::SendInterest, this);
  //Simulator::Schedule(Seconds(8.0), &CustomAppConsumerServiceDiscovery::SendInterest, this);
}

// Processing when application is stopped
void
CustomAppConsumerServiceDiscovery::StopApplication()
{
  m_isRunning = false;
  // cleanup ndn::App
  ndn::App::StopApplication();
}

void
CustomAppConsumerServiceDiscovery::SendSDInterest()
{
  if (!m_isRunning)
  {
    NS_LOG_DEBUG("Warning: trying to send Service Discovery interest while application is stopped!");
    return;
  }

  m_SDrunning = true;

  /////////////////////////////////////
  // Sending one Interest packet out //
  /////////////////////////////////////

  // Create and configure ndn::Interest
  //auto interest = std::make_shared<ndn::Interest>("/prefix/sub");
  auto interest = std::make_shared<ndn::Interest>(m_prefix); // this name is just a placeholder
  Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();
  interest->setNonce(rand->GetValue(0, std::numeric_limits<uint32_t>::max()));
  interest->setInterestLifetime(ndn::time::seconds(540));
  //interest->setMustBeFresh(true);

  

  std::ifstream f(m_dagPath);
  json dagObject = json::parse(f);

  // here we generate just the first interest(s) according to the workflow
  // to do this, we must discover which services in the DAG are "sink" services (services which feed the end consumer)

  std::string sinkService;
  for (auto& x : dagObject["dag"].items())
  {
    for (auto& y : dagObject["dag"][x.key()].items())
    {
      if (y.key() == "/consumer")
      {
        sinkService = x.key();
      }
    }
  }

  //m_SDstartTime = Simulator::Now();
  // Convert to integer in milliseconds and then to string
  int64_t SDstartTimeNS = m_SDstartTime.ToInteger(ns3::Time::NS);
  //std::stringstream ssSD_ns;
  //ssSD_ns << SDstartTimeNS << " ns";
  //ssSD_ns << SDstartTimeNS;
  dagObject["serviceDiscoveryStartTimeNS"] = SDstartTimeNS;

  //m_WFstartTime = Seconds(2.0);
  // Convert to integer in milliseconds and then to string
  int64_t WFstartTimeNS = m_WFstartTime.ToInteger(ns3::Time::NS);
  //std::stringstream ssWF_ns;
  //ssWF_ns << WFstartTimeNS << " ns";
  //ssWF_ns << WFstartTimeNS;
  //std::string timeStringNS = ssWF_ns.str();
  //std::cout << "SD Start Time in milliseconds: " << timeStringNS << std::endl;
  dagObject["workflowStartTimeNS"] = WFstartTimeNS;
  dagObject["resourceAllocation"] = m_fwdOpt; // run service discovery before running workflow (1: no CPU allocation, 2: use CPU allocation)


  //std::cout << "Consumer: Full DAG as read: " << std::setw(2) << dagObject << '\n';
  //std::cout << "Consumer: setting head to sinkService: " << sinkService << '\n';

  if (m_orchestrate == 0) {
    dagObject["head"] = sinkService;
    //interest->setName(m_prefix.ndn::Name::toUri() + sinkService);
    interest->setName(m_prefix.ndn::Name::toUri() + "/serviceDiscovery" + sinkService);

    bool consumerFound = false;
    // now we remove the entry that has the sinkService feeding the consumer. It is not needed, and can't be in the dag if we want caching of intermediate results to work.
    for (auto& x : dagObject["dag"].items())
    {
      //std::cout << "Checking x.key: " << (std::string)x.key() << '\n';
      for (auto& y : dagObject["dag"][x.key()].items())
      {
        //std::cout << "Checking y.key: " << (std::string)y.key() << '\n';
        //if (y.key() == m_service.ndn::Name::toUri())
        if (y.key() == "/consumer")
        {
          dagObject["dag"].erase(x.key());
          consumerFound = true;
          break;
        }
      }
      if (consumerFound == true)
        break;
    }
  }
  else if (m_orchestrate == 1){ // orchestration method A
    dagObject["head"] = "/serviceOrchestration";
    interest->setName(m_prefix.ndn::Name::toUri() + "/serviceOrchestration");
  }
  else if (m_orchestrate == 2){ // orchestration method B
    dagObject["head"] = "/serviceOrchestration/dag";
    interest->setName(m_prefix.ndn::Name::toUri() + "/serviceOrchestration/dag");
  }
  else
  {
    NS_LOG_DEBUG("ERROR, this should not happen. m_orchestrate value set out of bounds!" << '\n');
    return;
  }


  //std::cout << "Consumer: DAG as trimmed for first interest: " << std::setw(2) << dagObject << '\n';

  std::string dagString = dagObject.dump();
  // in order to convert from std::string to a char[] datatype we do the following (https://stackoverflow.com/questions/7352099/stdstring-to-char):
  char *dagStringParameter = new char[dagString.length() + 1];
  strcpy(dagStringParameter, dagString.c_str());

  size_t length = strlen(dagStringParameter);


  interest->setApplicationParameters((const uint8_t *)dagStringParameter, length);


  //NS_LOG_DEBUG("Consumer: Interest parameters being sent: " << dagStringParameter);
  //auto dagParameterFromInterest = interest->getApplicationParameters();
  //NS_LOG_DEBUG("Consumer: Interest parameters being sent: " << dagParameterFromInterest);


  NS_LOG_DEBUG("Sending Interest packet for " << *interest);

  // Call trace (for logging purposes)
  m_transmittedInterests(interest, this, m_face);

  m_appLink->onReceiveInterest(*interest);

}

void
CustomAppConsumerServiceDiscovery::SendInterest()
{
  if (!m_isRunning)
  {
    NS_LOG_DEBUG("Warning: trying to send interest while application is stopped!");
    return;
  }

  // if we are trying to run the workflow before service discovery finishes, report an error and abort.
  if(m_SDrunning == true)
  {
    NS_LOG_DEBUG("\n\n  ERROR!!! Workflow started before Service Discovery process finished!" << "\n\n");
    NS_LOG_DEBUG("\n\n  TO FIX ERROR: change the worflow start time for this scenario to be a little later, to allow SD to finish." << "\n\n");
    return;
  }

  m_startTime = Simulator::Now();

  /////////////////////////////////////
  // Sending one Interest packet out //
  /////////////////////////////////////

  // Create and configure ndn::Interest
  //auto interest = std::make_shared<ndn::Interest>("/prefix/sub");
  auto interest = std::make_shared<ndn::Interest>(m_prefix); // this name is just a placeholder
  Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();
  interest->setNonce(rand->GetValue(0, std::numeric_limits<uint32_t>::max()));
  interest->setInterestLifetime(ndn::time::seconds(540));
  //interest->setMustBeFresh(true);


  std::ifstream f(m_dagPath);
  json dagObject = json::parse(f);

  // here we generate just the first interest(s) according to the workflow
  // to do this, we must discover which services in the DAG are "sink" services (services which feed the end consumer)

  std::string sinkService;
  for (auto& x : dagObject["dag"].items())
  {
    for (auto& y : dagObject["dag"][x.key()].items())
    {
      if (y.key() == "/consumer")
      {
        sinkService = x.key();
      }
    }
  }


  //std::cout << "Consumer: Full DAG as read: " << std::setw(2) << dagObject << '\n';
  //std::cout << "Consumer: setting head to sinkService: " << sinkService << '\n';

  if (m_orchestrate == 0) {
    dagObject["head"] = sinkService;
    interest->setName(m_prefix.ndn::Name::toUri() + sinkService);

    bool consumerFound = false;
    // now we remove the entry that has the sinkService feeding the consumer. It is not needed, and can't be in the dag if we want caching of intermediate results to work.
    for (auto& x : dagObject["dag"].items())
    {
      //std::cout << "Checking x.key: " << (std::string)x.key() << '\n';
      for (auto& y : dagObject["dag"][x.key()].items())
      {
        //std::cout << "Checking y.key: " << (std::string)y.key() << '\n';
        //if (y.key() == m_service.ndn::Name::toUri())
        if (y.key() == "/consumer")
        {
          dagObject["dag"].erase(x.key());
          consumerFound = true;
          break;
        }
      }
      if (consumerFound == true)
        break;
    }
  }
  else if (m_orchestrate == 1){ // orchestration method A
    dagObject["head"] = "/serviceOrchestration";
    interest->setName(m_prefix.ndn::Name::toUri() + "/serviceOrchestration");
  }
  else if (m_orchestrate == 2){ // orchestration method B
    dagObject["head"] = "/serviceOrchestration/dag";
    interest->setName(m_prefix.ndn::Name::toUri() + "/serviceOrchestration/dag");
  }
  else
  {
    NS_LOG_DEBUG("ERROR, this should not happen. m_orchestrate value set out of bounds!" << '\n');
    return;
  }


  //std::cout << "Consumer: DAG as trimmed for first interest: " << std::setw(2) << dagObject << '\n';

  std::string dagString = dagObject.dump();
  // in order to convert from std::string to a char[] datatype we do the following (https://stackoverflow.com/questions/7352099/stdstring-to-char):
  char *dagStringParameter = new char[dagString.length() + 1];
  strcpy(dagStringParameter, dagString.c_str());


  //add dagStringParameter as custom parameter to interest packet
  size_t length = strlen(dagStringParameter);



  interest->setApplicationParameters((const uint8_t *)dagStringParameter, length);



  //NS_LOG_DEBUG("Consumer: Interest parameters being sent: " << dagStringParameter);
  //auto dagParameterFromInterest = interest->getApplicationParameters();
  //NS_LOG_DEBUG("Consumer: Interest parameters being sent: " << dagParameterFromInterest);


  NS_LOG_DEBUG("Sending Interest packet for " << *interest);

  // Call trace (for logging purposes)
  m_transmittedInterests(interest, this, m_face);

  m_appLink->onReceiveInterest(*interest);

}

// Callback that will be called when Interest arrives
void
CustomAppConsumerServiceDiscovery::OnInterest(std::shared_ptr<const ndn::Interest> interest)
{
  ndn::App::OnInterest(interest);

  NS_LOG_DEBUG("Received Interest packet for " << interest->getName());

  // Note that Interests send out by the app will not be sent back to the app !

  auto data = std::make_shared<ndn::Data>(interest->getName());
  data->setFreshnessPeriod(ndn::time::milliseconds(3000));
  data->setContent(std::make_shared< ::ndn::Buffer>(1024));
  ndn::StackHelper::getKeyChain().sign(*data);

  NS_LOG_DEBUG("Sending Data packet for " << data->getName());

  // Call trace (for logging purposes)
  m_transmittedDatas(data, this, m_face);

  m_appLink->onReceiveData(*data);
}

// Callback that will be called when Data arrives
void
CustomAppConsumerServiceDiscovery::OnData(std::shared_ptr<const ndn::Data> data)
{
  NS_LOG_DEBUG("Receiving Data packet for " << data->getName());


  if (data->getName().ndn::Name::getPrefix(-1).getSubName(1,1).ndn::Name::toUri() == "/serviceDiscovery")
  {
    std::cout << "\n\n      CONSUMER: Service Discovery DATA received for name " << data->getName() << std::endl << "\n\n";
    std::string dataPacketString;
    dataPacketString = (const char *)data->getContent().value();
    json dataPacketContents = json::parse(dataPacketString);
    NS_LOG_DEBUG("\n\nData received - absolute EFT    (nanoseconds): " << dataPacketContents["EFT"] << "\n"
                     "              - workflow start: (nanoseconds): " << m_WFstartTime.ToInteger(ns3::Time::NS) << "\n\n");
    int64_t workflowLatency_ns = dataPacketContents["EFT"].get<int64_t>() - m_WFstartTime.ToInteger(ns3::Time::NS);
    NS_LOG_DEBUG("\n\n  Service Latency estimated by SD: " << workflowLatency_ns/1000 << " microseconds.\n\n");

    // IF this is an SD data packet, then begin the normal consumer workflow request (call CustomAppConsumerServiceDiscovery::SendInterest())
    //CustomAppConsumerServiceDiscovery::SendInterest();

    m_SDrunning = false;

    // if we have already started the workflow, report an error. (if current time is past the workflow start time)
    Time timeNow = Simulator::Now();
    if (timeNow > m_WFstartTime)
    {
      NS_LOG_DEBUG("\n\n  ERROR!!! Workflow started before Service Discovery process finished!" << "\n\n");
      NS_LOG_DEBUG("\n\n  TO FIX ERROR: change the worflow start time for this scenario to be a little later, to allow SD to finish." << "\n\n");
    }

  }
  else
  {
  // else, it's the final workflow data packet, so just report the result and stop the simulation timer

    std::cout << "\n\n      CONSUMER: DATA received for name " << data->getName() << std::endl << "\n\n";

    m_endTime = Simulator::Now();
    Time serviceLatency = m_endTime - m_startTime;
    std::cout << "\n  Service Latency: " <<  serviceLatency.GetMilliSeconds() << " milliseconds." << std::endl;
    std::cout << "\n  Service Latency: " <<  serviceLatency.GetMicroSeconds() << " microseconds." << std::endl;

    ndn::Block myRxedBlock = data->getContent();
    //std::cout << "\nCONSUMER: result = " << myRxedBlock << std::endl << "\n\n";
    uint8_t *pContent = (uint8_t *)(myRxedBlock.data()); // this points to the first byte, which is the TLV-TYPE (21 for data packet contet)
    pContent++;  // now this points to the second byte, containing 253 (0xFD), meaning size (1024) is expressed with 2 octets
    pContent++;  // now this points to the first size octet
    pContent++;  // now this points to the second size octet
    pContent++;  // now we are pointing at the first byte of the true content
    std::cout << "\n  The final answer is: " <<  (int)(*pContent) << std::endl << "\n\n";

  }

}

} // namespace ns3
