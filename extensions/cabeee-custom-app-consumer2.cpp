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

// cabeee-custom-app-consumer2.cpp

#include "cabeee-custom-app-consumer2.hpp"
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

#include "ns3/uinteger.h"

//#include "ns3/ndnSIM/ndn-cxx/ndn-cxx/encoding/encoder.hpp"
//#include "ns3/ndnSIM/ndn-cxx/ndn-cxx/encoding/block.hpp"
//#include "ns3/ndnSIM/ndn-cxx/encoding/tlv.hpp"
//#include "ns3/ndnSIM/ndn-cxx/encoding/block-helpers.hpp"


NS_LOG_COMPONENT_DEFINE("CustomAppConsumer2");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED(CustomAppConsumer2);

// register NS-3 type
TypeId
CustomAppConsumer2::GetTypeId()
{
  static TypeId tid = TypeId("CustomAppConsumer2")
    .SetParent<ndn::App>()
    .AddConstructor<CustomAppConsumer2>()
    .AddAttribute("Prefix", "Requested name", StringValue("/dumb-interest"),
                    ndn::MakeNameAccessor(&CustomAppConsumer2::m_name), ndn::MakeNameChecker())
    .AddAttribute("Orchestrate", "Requested orchestration", UintegerValue(0),
                    MakeUintegerAccessor(&CustomAppConsumer2::m_orchestrate), MakeUintegerChecker<uint16_t>());
  return tid;
}

CustomAppConsumer2::CustomAppConsumer2()
  : m_isRunning(false)
{
}

// Processing upon start of the application
void
CustomAppConsumer2::StartApplication()
{
  // initialize ndn::App
  ndn::App::StartApplication();
  m_isRunning = true;

  // Add entry to FIB for `/prefix/sub`
  //ndn::FibHelper::AddRoute(GetNode(), "/prefix/sub", m_face, 0); //cabeee took this out, let the global router figure it out.
  // cabeee - I think this means: if any interest exists for /prefix/sub, forward to this application's
  // face, and install the forwarding rule in the node where this application is running.
  // This is why we must remove the line, otherwise the interest is simply fowarded back into the app???
  // Although I don't see the log entry for receiving an interest.


  // Schedule send of first interest
  Simulator::Schedule(Seconds(1.0), &CustomAppConsumer2::SendInterest, this);
  //Simulator::Schedule(Seconds(2.0), &CustomAppConsumer2::SendInterest, this);
  //Simulator::Schedule(Seconds(3.0), &CustomAppConsumer2::SendInterest, this);
  //Simulator::Schedule(Seconds(4.0), &CustomAppConsumer2::SendInterest, this);
  //Simulator::Schedule(Seconds(5.0), &CustomAppConsumer2::SendInterest, this);
  //Simulator::Schedule(Seconds(6.0), &CustomAppConsumer2::SendInterest, this);
  //Simulator::Schedule(Seconds(7.0), &CustomAppConsumer2::SendInterest, this);
  //Simulator::Schedule(Seconds(8.0), &CustomAppConsumer2::SendInterest, this);
}

// Processing when application is stopped
void
CustomAppConsumer2::StopApplication()
{
  m_isRunning = false;
  // cleanup ndn::App
  ndn::App::StopApplication();
}

void
CustomAppConsumer2::SendInterest()
{
  if (!m_isRunning)
  {
    NS_LOG_DEBUG("Warning: trying to send interest while application is stopped!");
    return;
  }

  /////////////////////////////////////
  // Sending one Interest packet out //
  /////////////////////////////////////

  // Create and configure ndn::Interest
  //auto interest = std::make_shared<ndn::Interest>("/prefix/sub");
  auto interest = std::make_shared<ndn::Interest>(m_name); // cabeee - it is now set externally from the scenario
  Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();
  interest->setNonce(rand->GetValue(0, std::numeric_limits<uint32_t>::max()));
  interest->setInterestLifetime(ndn::time::seconds(5));
  //interest->setMustBeFresh(true);

  /*
  // this is my attempt to create the dag workflow using a struct, rather than using JSON. I aborted early on.
  std::list<ndn::cabeee::DAG_SERVICE> serviceList;
  ndn::cabeee::DAG_SERVICE service;

  service.thisService = "sensor";
  service.feeds.clear();
  service.feeds.push_back("service1");

  serviceList.push_back(service);

  service.thisService = "service1";
  service.feeds.clear();
  service.feeds.push_back("service2");
  service.feeds.push_back("service3");

  serviceList.push_back(service);

  service.thisService = "service2";
  service.feeds.clear();
  service.feeds.push_back("service4");

  serviceList.push_back(service);

  service.thisService = "service3";
  service.feeds.clear();
  service.feeds.push_back("service4");

  serviceList.push_back(service);

  //ndn::cabeee::DAG_INTEREST dagStringParameter = {.head = "service4", .dagWorkflow = serviceList, .hash = "0123456789ABCDEF"};
  */


  std::ifstream f("workflows/rpa-dag_reuse.json"); //TODO: take input from command line rather than hardcoding the dag file
  //std::ifstream f("workflows/rpa-dag_reuse2.json"); //TODO: take input from command line rather than hardcoding the dag file
  json dagObject = json::parse(f);

  // here we generate just the first interest(s) according to the workflow
  // to do this, we must discover which services in the DAG are "sink" services (services which feed the end consumer)

  std::string sinkService;
  for (auto& x : dagObject["dag"].items())
  {
    for (auto& y : dagObject["dag"][x.key()].items())
    {
      if (y.key() == "/consumer2")
      {
        sinkService = x.key();
      }
    }
  }


/*
  for (auto& x : dagObject["dag"].items())
  {
    m_listOfRootServices.push_back(x.key()); // for now, add ALL keys to the list, we'll remove non-root ones later
    for (auto& y : dagObject["dag"][x.key()].items())
    {
      m_listOfServicesWithInputs.push_back(y.key()); // add all values to the list
      if ((std::find(m_listOfSinkNodes.begin(), m_listOfSinkNodes.end(), y.key()) == m_listOfSinkNodes.end())) // if y.key() does not exist in m_listOfSinkNodes
      {
        m_listOfSinkNodes.push_back(y.key()); // for now, add ALL values to the list, we'll remove non-sinks later
      }
    }
  }

  // now remove services that feed into other services from the list of sink nodes
  for (auto& x : dagObject["dag"].items())
  {
    if (!(std::find(m_listOfSinkNodes.begin(), m_listOfSinkNodes.end(), x.key()) == m_listOfSinkNodes.end())) // if x.key() exists in m_listOfSinkNodes
    {
      m_listOfSinkNodes.remove(x.key());
    }
  }

  // now remove services that have other services as inputs from the list of root services
  for (auto serviceWithInputs : m_listOfServicesWithInputs)
  {
    if (!(std::find(m_listOfRootServices.begin(), m_listOfRootServices.end(), serviceWithInputs) == m_listOfRootServices.end())) // if serviceWithInputs exists in m_listOfRootServices
    {
      m_listOfRootServices.remove(serviceWithInputs);
    }
  }

  unsigned char numOfSinkServices = 0;
  for (auto sinkService : m_listOfSinkNodes)
  {
    numOfSinkServices++;

    if (m_orchestrate == 0) {
      dagObject["head"] = sinkService;
      interest->setName(sinkService);
    }
    else if (m_orchestrate == 1){ // orchestration method A
      dagObject["head"] = "/serviceOrchestration";
      interest->setName("/serviceOrchestration");
    }
    else if (m_orchestrate == 2){ // orchestration method B
      dagObject["head"] = "/serviceOrchestration/dag";
      interest->setName("/serviceOrchestration/dag");
    }
    else
    {
      NS_LOG_DEBUG("ERROR, this should not happen. m_orchestrate value set out of bounds!" << '\n');
      return;
    }
  }

  if (numOfSinkServices != 1)
  {
    NS_LOG_DEBUG("ERROR, this should not happen. Consumer2 found more than one (or none) sink services!" << '\n');
    return;
  }

*/



  if (m_orchestrate == 0) {
    //dagObject["head"] = "/service4"; //TODO: I'm doing this manually right now. I should look at the json input file, and see which service feeds "consumer2", and use that instead of hardcoding
    //interest->setName("/service4"); //TODO: I'm doing this manually right now. I should look at the json input file, and see which service feeds "consumer2", and use that instead of hardcoding
    dagObject["head"] = sinkService;
    interest->setName(sinkService);
  }
  else if (m_orchestrate == 1){ // orchestration method A
    dagObject["head"] = "/serviceOrchestration";
    interest->setName("/serviceOrchestration");
  }
  else if (m_orchestrate == 2){ // orchestration method B
    dagObject["head"] = "/serviceOrchestration/dag";
    interest->setName("/serviceOrchestration/dag");
  }
  else
  {
    NS_LOG_DEBUG("ERROR, this should not happen. m_orchestrate value set out of bounds!" << '\n');
    return;
  }



  std::string dagString = dagObject.dump();
  // in order to convert from std::string to a char[] datatype we do the following (https://stackoverflow.com/questions/7352099/stdstring-to-char):
  char *dagStringParameter = new char[dagString.length() + 1];
  strcpy(dagStringParameter, dagString.c_str());

  //std::string dagStringParameter = "head:service4,dagWorkflow:sensor>service1#service1>service2,service3#service2>service4#service3>service4,hash:0123456789ABCDEF";
  //char dagStringParameter[] = "head:service4,dagWorkflow:sensor>service1#service1>service2,service3#service2>service4#service3>service4,hash:0123456789ABCDEF";
  //std::string dagStringParameter = "abcdef";
  //const uint8_t * buffer = (const uint8_t*)dagStringParameter;

  //add dagStringParameter as custom parameter to interest packet
  //const uint8_t * buffer = (const uint8_t*)&dagStringParameter;
  //const uint8_t * buffer = NULL;
  //size_t length = dagParameter.length();
  size_t length = strlen(dagStringParameter);


  //uint32_t type = ndn::tlv::ApplicationParameters;
  //uint32_t type = 36;
  //ndn::ConstBufferPtr value = buffer;
  //ndn::Block appParamBlock = new ndn::Block(type, buffer);
  //ndn::Block appParamBlock = new ndn::Block(type, buffer);

  //ndn::Block appParamBlock = ndn::Encoder::BlockHelper::makeEmptyBlock(ndn::tlv::ApplicationParameters)
  //ndn::Block appParamBlock = ndn::encoding::makeEmptyBlock(ndn::tlv::ApplicationParameters)
  //ndn::Block appParamBlock = ndn::encoding::makeEmptyBlock(36);
  //ndn::Block appParamBlock = ndn::encoding::makeStringBlock(36, "fred");
  //ndn::Block appParamBlock = ndn::encoding::makeStringBlock(36, buffer);
  //ndn::Block appParamBlock;
  //auto appParamBlock = std::make_shared<ndn::Block>();


  // the parameters are encoded as a TLV-Value!!!
  //interest->setApplicationParameters(buffer, length);
  interest->setApplicationParameters((const uint8_t *)dagStringParameter, length);
  //interest->setApplicationParameters(appParamBlock);
  //extract custom parameter from interest packet
  //auto dagStringParameterFromInterest = interest.getApplicationParameters();


  /*
  // These interests are NOT signed. The SHA256 digest added by the application parameters is separate from a signature!
  auto sigPresent = interest->isSigned();
  NS_LOG_DEBUG("Consumer2: Interest is signed: " << sigPresent);
  auto dagSignatureInfo = interest->getSignatureInfo();
  //NS_LOG_DEBUG("Consumer2: Interest signature info after adding appParams: " << dagSignatureInfo);
  if (dagSignatureInfo) {
    auto dagSignatureType2 = dagSignatureInfo->getSignatureType();
    NS_LOG_DEBUG("Consumer2: Interest signature type after adding appParams: " << dagSignatureType2);
  }
  auto dagSignatureValue = interest->getSignatureValue();
  NS_LOG_DEBUG("Consumer2: Interest signature value after adding appParams: " << dagSignatureValue);
  */


  //NS_LOG_DEBUG("Consumer2: Interest parameters being sent: " << dagStringParameter);
  //auto dagParameterFromInterest = interest->getApplicationParameters();
  //NS_LOG_DEBUG("Consumer2: Interest parameters being sent: " << dagParameterFromInterest);


  NS_LOG_DEBUG("Sending Interest packet for " << *interest);

  // Call trace (for logging purposes)
  m_transmittedInterests(interest, this, m_face);

  m_appLink->onReceiveInterest(*interest);

}

// Callback that will be called when Interest arrives
void
CustomAppConsumer2::OnInterest(std::shared_ptr<const ndn::Interest> interest)
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
CustomAppConsumer2::OnData(std::shared_ptr<const ndn::Data> data)
{
  NS_LOG_DEBUG("Receiving Data packet for " << data->getName());

  std::cout << "\n\n      CONSUMER2: DATA received for name " << data->getName() << std::endl << "\n\n";

  ndn::Block myRxedBlock = data->getContent();
  //std::cout << "\nCONSUMER2: result = " << myRxedBlock << std::endl << "\n\n";

  uint8_t *pContent = (uint8_t *)(myRxedBlock.data()); // this points to the first byte, which is the TLV-TYPE (21 for data packet contet)
  pContent++;  // now this points to the second byte, containing 253 (0xFD), meaning size (1024) is expressed with 2 octets
  pContent++;  // now this points to the first size octet
  pContent++;  // now this points to the second size octet
  pContent++;  // now we are pointing at the first byte of the true content
  std::cout << "\n  The final answer is: " <<  (int)(*pContent) << std::endl << "\n\n";

}

} // namespace ns3
