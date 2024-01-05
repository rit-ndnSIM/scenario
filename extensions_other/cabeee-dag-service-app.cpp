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

// cabeee-dag-service-app.cpp

#include "cabeee-dag-service-app.hpp"

#include "ns3/ptr.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "ns3/string.h"

#include "ns3/ndnSIM/helper/ndn-stack-helper.hpp"
#include "ns3/ndnSIM/helper/ndn-fib-helper.hpp"

#include "ns3/random-variable-stream.h"

//#include "ns3/ndnSIM/ndn-cxx/ndn-cxx/encoding/encoder.hpp"
//#include "ns3/ndnSIM/ndn-cxx/ndn-cxx/encoding/block.hpp"
//#include "ns3/ndnSIM/ndn-cxx/encoding/tlv.hpp"
//#include "ns3/ndnSIM/ndn-cxx/encoding/block-helpers.hpp"

#include <string.h>

#include <fstream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

NS_LOG_COMPONENT_DEFINE("DagServiceApp");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED(DagServiceApp);

// register NS-3 type
TypeId
DagServiceApp::GetTypeId()
{
  static TypeId tid = TypeId("DagServiceApp")
    .SetParent<ndn::App>()
    .AddConstructor<DagServiceApp>()
    .AddAttribute("Prefix", "Requested name", StringValue("/dumb-interest"),
                    ndn::MakeNameAccessor(&DagServiceApp::m_name), ndn::MakeNameChecker())
    .AddAttribute("Service", "Requested service", StringValue("dumb-service"),
                    ndn::MakeNameAccessor(&DagServiceApp::m_service), ndn::MakeNameChecker());
  return tid;
}

DagServiceApp::DagServiceApp()
  : m_isRunning(false)
{
}

// Processing upon start of the application
void
DagServiceApp::StartApplication()
{
  // initialize ndn::App
  ndn::App::StartApplication();
  m_isRunning = true;

  // Add entry to FIB for `/prefix/sub`
  //ndn::FibHelper::AddRoute(GetNode(), "/prefix/sub", m_face, 0); //cabeee took this out, let the global router figure it out.
  ndn::FibHelper::AddRoute(GetNode(), m_name, m_face, 0);


  m_inputTotal = 0;
  m_numRxedInputs = 0;

  //DagServiceApp::SendInterest("/cabeee");
  // Schedule send of first interest
  //Simulator::Schedule(Seconds(1.0), &DagServiceApp::SendInterest, this);
  //Simulator::Schedule(Seconds(2.0), &DagServiceApp::SendInterest, this);
  //Simulator::Schedule(Seconds(3.0), &DagServiceApp::SendInterest, this);
  //Simulator::Schedule(Seconds(4.0), &DagServiceApp::SendInterest, this);
  //Simulator::Schedule(Seconds(5.0), &DagServiceApp::SendInterest, this);
  //Simulator::Schedule(Seconds(6.0), &DagServiceApp::SendInterest, this);
  //Simulator::Schedule(Seconds(7.0), &DagServiceApp::SendInterest, this);
  //Simulator::Schedule(Seconds(8.0), &DagServiceApp::SendInterest, this);
}

// Processing when application is stopped
void
DagServiceApp::StopApplication()
{
  m_isRunning = false;
  // cleanup ndn::App
  ndn::App::StopApplication();
}






// TODO: go through this file and make changes so that:
// received interests will contain the name of the required inputs. This service must then generate said interest(s) (for inputs).
// Keep track of received interests. Once all interests have been received, run the service and respond with either 1) data or 2) data packet with complete signal?

// Figure out how to store the data locally? If an interest comes in for results, just respond with results? Caching will take care of this for us IF we have genrated data packets with results in the past.






/*
void
DagServiceApp::SendInterest(const std::string& interestName, ndn::Block dagParameter)
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
  //auto interest = std::make_shared<ndn::Interest>(m_name); // must take it in as an argument, not just use m_name!!
  auto interest = std::make_shared<ndn::Interest>(interestName);
  Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();
  interest->setNonce(rand->GetValue(0, std::numeric_limits<uint32_t>::max()));
  interest->setInterestLifetime(ndn::time::seconds(5));
  //interest->setMustBeFresh(true);



  // overwrite the dag parameter "head" value and generate new application parameters (thus calculating new sha256 digest)
  std::string dagString = std::string(reinterpret_cast<const char*>(dagParameter.value()), dagParameter.value_size());
  auto dagObject = json::parse(dagString);
  dagObject["head"] = interestName;
  std::string updatedDagString = dagObject.dump();
  // in order to convert from std::string to a char[] datatype we do the following (https://stackoverflow.com/questions/7352099/stdstring-to-char):
  char *dagStringParameter = new char[updatedDagString.length() + 1];
  strcpy(dagStringParameter, updatedDagString.c_str());
  size_t length = strlen(dagStringParameter);

  //add DAG workflow as a parameter to the new interest
  //interest->setApplicationParameters(dagParameter);

  //add modified DAG workflow as a parameter to the new interest
  interest->setApplicationParameters((const uint8_t *)dagStringParameter, length);



  NS_LOG_DEBUG("Service App: Sending Interest packet for " << *interest);

  // Call trace (for logging purposes)
  m_transmittedInterests(interest, this, m_face);

  m_appLink->onReceiveInterest(*interest);
}
*/

// Callback that will be called when Interest arrives
void
DagServiceApp::OnInterest(std::shared_ptr<const ndn::Interest> interest)
{
  ndn::App::OnInterest(interest);


  NS_LOG_DEBUG("Received Interest packet for " << interest->getName());
  //NS_LOG_DEBUG("Received on node hosting service " << m_service);




  // extract the inputs from the interest parameters
  // TODO: right now, I'm just extracting inputs into a string, but operands are raw bytes. This can be much cleaner!

  //extract custom parameter from interest packet
  auto appParameterFromInterest = interest->getApplicationParameters();
  //std::string appParamString = ndn::encoding::readString(appParameterFromInterest);
  std::string appParamString = std::string(reinterpret_cast<const char*>(appParameterFromInterest.value()), appParameterFromInterest.value_size());
  //std::string appParamString = ndn::lp::DecodeHelper();
  //auto dagBuffer = appParameterFromInterest.ndn::Block::getBuffer();
  //auto dagValue = appParameterFromInterest.ndn::Block::value();
  ////NS_LOG_DEBUG("Interest parameters received: " << appParameterFromInterest);
  //NS_LOG_DEBUG("Interest parameters received as string: " << appParamString);
  ////NS_LOG_DEBUG("Interest parameters received as value: " << dagValue);




  //"RUN" the service, and create a new data packet to respond downstream
  //NS_LOG_DEBUG("Fake running service " << m_service);
  NS_LOG_DEBUG("Running service " << m_service);

  // run operation. First we need to figure out what service this is, so we know the operation. This screams to be a function pointer! For now just use if's


  if (m_service.ndn::Name::toUri() == "/service1"){
    serviceOutput = (appParamString[0])*2;
  }
  if (m_service.ndn::Name::toUri() == "/service2"){
    serviceOutput = (appParamString[0])+1;
  }
  if (m_service.ndn::Name::toUri() == "/service3"){
    serviceOutput = (appParamString[0])+7;
  }
  if (m_service.ndn::Name::toUri() == "/service4"){
    serviceOutput = (appParamString[0])*3 + (appParamString[1])*4;
  }
    
  NS_LOG_DEBUG("Service " << m_service.ndn::Name::toUri() << " has output: " << (int)serviceOutput);
  
  // this following line is for linear workflows only!
  //now add the service name in front of the data name
  //std::string new_name = m_service.ndn::Name::toUri() + data->getName().ndn::Name::toUri();
  //NS_LOG_DEBUG("Creating data for new name: " << new_name);
  // for dag workflows, FOR NOW we just generate the data packet with the name of the service we ran. We don't support repeated services yet. For that we need higharchical names/results such as "/S2/S1/sensor" for example

  NS_LOG_DEBUG("Creating data for name: " << m_nameAndDigest);  // m_name doesn't have the sha256 digest, so it doesn't match the original interest!
                                                                  // We use m_nameAndDigest to store the old name with the digest.

  //auto new_data = std::make_shared<ndn::Data>(new_name);
  auto new_data = std::make_shared<ndn::Data>(m_nameAndDigest);
  new_data->setFreshnessPeriod(ndn::time::milliseconds(3000));

  //new_data->setContent(std::make_shared< ::ndn::Buffer>(1024));
  unsigned char myBuffer[1024];
  // write to the buffer
  myBuffer[0] = serviceOutput;
  new_data->setContent(myBuffer, 1024);

  ndn::StackHelper::getKeyChain().sign(*new_data);
  // Call trace (for logging purposes)
  m_transmittedDatas(new_data, this, m_face);
  m_appLink->onReceiveData(*new_data);




}

/*
// Callback that will be called when Data arrives
void
DagServiceApp::OnData(std::shared_ptr<const ndn::Data> data)
{
  NS_LOG_DEBUG("Receiving Data packet for " << data->getName());

  //std::cout << "DATA received for name " << data->getName() << std::endl;

  //std::cout << "content = " << data->getContent() << std::endl;

  
}
*/

} // namespace ns3
