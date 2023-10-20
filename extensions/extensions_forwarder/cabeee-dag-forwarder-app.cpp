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

// cabeee-dag-forwarder-app.cpp

#include "cabeee-dag-forwarder-app.hpp"

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

NS_LOG_COMPONENT_DEFINE("DagForwarderApp");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED(DagForwarderApp);

// register NS-3 type
TypeId
DagForwarderApp::GetTypeId()
{
  static TypeId tid = TypeId("DagForwarderApp")
    .SetParent<ndn::App>()
    .AddConstructor<DagForwarderApp>()
    .AddAttribute("Prefix", "Requested name", StringValue("/dumb-interest"),
                    ndn::MakeNameAccessor(&DagForwarderApp::m_name), ndn::MakeNameChecker())
    .AddAttribute("Service", "Requested service", StringValue("dumb-service"),
                    ndn::MakeNameAccessor(&DagForwarderApp::m_service), ndn::MakeNameChecker());
  return tid;
}

DagForwarderApp::DagForwarderApp()
  : m_isRunning(false)
{
}

// Processing upon start of the application
void
DagForwarderApp::StartApplication()
{
  // initialize ndn::App
  ndn::App::StartApplication();
  m_isRunning = true;

  // Add entry to FIB for `/prefix/sub`
  //ndn::FibHelper::AddRoute(GetNode(), "/prefix/sub", m_face, 0); //cabeee took this out, let the global router figure it out.
  ndn::FibHelper::AddRoute(GetNode(), m_name, m_face, 0);


  m_inputTotal = 0;
  m_numRxedInputs = 0;

  //DagForwarderApp::SendInterest("/cabeee");
  // Schedule send of first interest
  //Simulator::Schedule(Seconds(1.0), &DagForwarderApp::SendInterest, this);
  //Simulator::Schedule(Seconds(2.0), &DagForwarderApp::SendInterest, this);
  //Simulator::Schedule(Seconds(3.0), &DagForwarderApp::SendInterest, this);
  //Simulator::Schedule(Seconds(4.0), &DagForwarderApp::SendInterest, this);
  //Simulator::Schedule(Seconds(5.0), &DagForwarderApp::SendInterest, this);
  //Simulator::Schedule(Seconds(6.0), &DagForwarderApp::SendInterest, this);
  //Simulator::Schedule(Seconds(7.0), &DagForwarderApp::SendInterest, this);
  //Simulator::Schedule(Seconds(8.0), &DagForwarderApp::SendInterest, this);
}

// Processing when application is stopped
void
DagForwarderApp::StopApplication()
{
  m_isRunning = false;
  // cleanup ndn::App
  ndn::App::StopApplication();
}

void
DagForwarderApp::SendInterest(const std::string& interestName, ndn::Block dagParameter)
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


  //TODO: we should PRUNE the DAG workflow to not include anything further downstream than this service,
  // so that when caching is implemented, we can reuse intermediate results for other DAG workflows which use portions of the current DAG.

  //add DAG workflow as a parameter to the new interest
  //interest->setApplicationParameters(dagParameter);
  //add modified DAG workflow as a parameter to the new interest
  interest->setApplicationParameters((const uint8_t *)dagStringParameter, length);



  NS_LOG_DEBUG("Forwarder: Sending Interest packet for " << *interest);

  // Call trace (for logging purposes)
  m_transmittedInterests(interest, this, m_face);

  m_appLink->onReceiveInterest(*interest);
}

// Callback that will be called when Interest arrives
void
DagForwarderApp::OnInterest(std::shared_ptr<const ndn::Interest> interest)
{
  ndn::App::OnInterest(interest);


  NS_LOG_DEBUG("Received Interest packet for " << interest->getName());
  //NS_LOG_DEBUG("Received on node hosting service " << m_service);

  /*
  //remove the FIRST portion of the name to create a new interest for the next service in the DAG workflow

  //std::string interestName = ndn::Name::toUri(interest->getName());
  //std::string last_element(interestName.substr(interestName.rfind("::") + 2));

  // it might be easier to simply use the ndn provided methods: getSubName() and getPrefix()

  //ndn::PartialName last_element = ndn::Name::getSubName(-1, 1); // isolate the last component of the name
  //ndn::Name last_element = interest->getName().ndn::Name::getSubName(-1, 1); // isolate the last component of the name
  //NS_LOG_DEBUG("Found last element: " << last_element);

  //ndn::Name without_last_element = interest->getName().ndn::Name::getPrefix(-1); // remove the last component
  //NS_LOG_DEBUG("Name without last element: " << without_last_element);
  //std::string string_without_last_element = without_last_element.ndn::Name::toUri();
  //NS_LOG_DEBUG("String without last element: " << string_without_last_element);

  //ndn::Name first_element = interest->getName().ndn::Name::getPrefix(1); // keep only first component
  //NS_LOG_DEBUG("Name first element: " << first_element);
  //std::string string_first_element = first_element.ndn::Name::toUri();
  //NS_LOG_DEBUG("String first element: " << string_first_element);

  ndn::Name without_first_element = interest->getName().ndn::Name::getSubName(1, ndn::Name::npos); // remove the first component
  NS_LOG_DEBUG("Name without first element: " << without_first_element);
  std::string string_without_first_element = without_first_element.ndn::Name::toUri();
  //NS_LOG_DEBUG("String without first element: " << string_without_first_element);

  // generate the new interest with the shorter name
  DagForwarderApp::SendInterest(string_without_first_element);
  */


  // decode the DAG string contained in the application parameters, so we can generate the new interest(s)
  //extract custom parameter from interest packet
  auto dagParameterFromInterest = interest->getApplicationParameters();
  //std::string dagString = ndn::encoding::readString(dagParameterFromInterest);
  std::string dagString = std::string(reinterpret_cast<const char*>(dagParameterFromInterest.value()), dagParameterFromInterest.value_size());
  //std::string dagString = ndn::lp::DecodeHelper();
  //auto dagBuffer = dagParameterFromInterest.ndn::Block::getBuffer();
  //auto dagValue = dagParameterFromInterest.ndn::Block::value();
  ////NS_LOG_DEBUG("Interest parameters received: " << dagParameterFromInterest);
  //NS_LOG_DEBUG("Interest parameters received as string: " << dagString);
  ////NS_LOG_DEBUG("Interest parameters received as value: " << dagValue);



  //bool dagParameterDigestValid = interest->isParametersDigestValid();
  //NS_LOG_DEBUG("Fwd: Interest parameter digest is valid: " << dagParameterDigestValid);

  //bool dagParameterPresent = interest->hasApplicationParameters();
  //NS_LOG_DEBUG("Fwd: Interest parameters presesnt: " << dagParameterPresent);



/*
  // for now, I'm just doing a linear workflow with the dag workflow attached to the interests for testing.
  ndn::Name without_first_element = interest->getName().ndn::Name::getSubName(1, ndn::Name::npos); // remove the first component
  NS_LOG_DEBUG("Name without first element: " << without_first_element);
  std::string string_without_first_element = without_first_element.ndn::Name::toUri();
  DagForwarderApp::SendInterest(string_without_first_element, dagParameterFromInterest);
*/



  // read the dag parameters and figure out which interests to generate next. Change the dagParameters accordingly (head will be different)
  auto dagObject = json::parse(dagString);
  //NS_LOG_DEBUG("Interest parameter head: " << dagObject["head"] << ", m_name attribute: " << m_name.ndn::Name::toUri());
  //if (dagObject["head"] != m_name.ndn::Name::toUri())
  //{
    //NS_LOG_DEBUG("Forwarder app ERROR: received interest with DAG head different than m_name attribute for this service.");
    //NS_LOG_DEBUG("Interest parameter head: " << dagObject["head"] << ", m_name attribute: " << m_name.ndn::Name::toUri());
  //}
  //NS_LOG_DEBUG("Interest parameter number of services: " << dagObject["dag"].size());
  //NS_LOG_DEBUG("Interest parameter sensor feeds " << (dagObject["dag"]["/sensor"].size()) << " services: " << dagObject["dag"]["/sensor"]);
  //NS_LOG_DEBUG("Interest parameter s1 feeds " << (dagObject["dag"]["/S1"].size()) << " services: " << dagObject["dag"]["/S1"]);



  for (auto& x : dagObject["dag"].items())
  {
    for (auto& y : dagObject["dag"][x.key()].items())
    {
      //NS_LOG_DEBUG("Looking at service x: " << x.key() << " feeding into service y: " << y.value());
      //NS_LOG_DEBUG("Comparing y.value(): " << y.value() << " with the name of this application's service: " << m_name.ndn::Name::toUri());
      //if (y.value() == dagObject["head"])
      if (y.value() == m_name.ndn::Name::toUri()) // if service x feeds into this service (y) then we need to generate an interest for x.
      {
        // generate new interest for service that feeds results to this application's service
        NS_LOG_DEBUG("Generating interest for: " << x.key());

        // We need to see if this interest has already been generated. If so, don't increment (seems duplicate interests are still generated and reach the "sensor")
        // if this is a new interest (if interest is not in our list of generated interests)
        if ((std::find(m_listOfGeneratedInterests.begin(), m_listOfGeneratedInterests.end(), x.key()) == m_listOfGeneratedInterests.end())) { // if we don't find it...
          // add this new interest to list of generated interests
          m_listOfGeneratedInterests.push_back(x.key());
          m_inputTotal++;
          NS_LOG_DEBUG(" THIS IS A NEW interest for X: " << x.key() << ", which feeds into service Y: " << y.value());

          m_mapOfRxedBlocks[y.value()].push_back(x.key());  // this is to keep track of the order of inputs for multi-input services.
                                                            // without this, we won't know which data packet goes with which generated interest.
          m_vectorOfServiceInputs.push_back(0);             // for now, just create vector entries for the inputs, so that if they arrive out of order, we can insert at any index location
        }
//TODO: is the following line supposed to be inside the if conditional above???
        DagForwarderApp::SendInterest(x.key(), dagParameterFromInterest);
      }
    }
  }
  //DagForwarderApp::SendInterest(string_without_first_element, dagParameterFromInterest);



  m_nameAndDigest = interest->getName();  // store the name with digest so that we can later generate the data packet with the same name/digest!
                                          // TODO: this has issues - we cannot use the same service in more than one location in the DAG workflow!
                                          // for this, we would need to be able to store and retrieve unique interests and their digest (perhaps using a fully hierarchical name?)
                                  // right now, this application only has one m_nameAndDigest private variable, and thus can only "store" one service instance.
                                  // we could turn that variable into a list of ndn::Name variables, and add to the list for each instance of the service?


  // Note that Interests send out by the app will not be sent back to the app !

  /*
  auto data = std::make_shared<ndn::Data>(interest->getName());
  data->setFreshnessPeriod(ndn::time::milliseconds(3000));
  data->setContent(std::make_shared< ::ndn::Buffer>(1024));
  ndn::StackHelper::getKeyChain().sign(*data);
  NS_LOG_DEBUG("Sending Data packet for " << data->getName());
  // Call trace (for logging purposes)
  m_transmittedDatas(data, this, m_face);
  m_appLink->onReceiveData(*data);
  */
}

// Callback that will be called when Data arrives
void
DagForwarderApp::OnData(std::shared_ptr<const ndn::Data> data)
{
  NS_LOG_DEBUG("Receiving Data packet for " << data->getName());

  //std::cout << "DATA received for name " << data->getName() << std::endl;

  //std::cout << "numRxedInputs = " << m_numRxedInputs << std::endl;
  //std::cout << "content = " << data->getContent() << std::endl;

  std::string rxedDataName = (data->getName()).getPrefix(-1).toUri(); // remove the last component of the name (the parameter digest) so we have just the raw name


  // TODO: this is a HACK. I need a better way to get to the first byte of the content. Right now, I'm just incrementing the pointer past the TLV type, and size.
  // and then getting to the first byte (which is all I'm using for data)
  unsigned char serviceOutput = 0;
  uint8_t *pServiceInput = 0;
  //pServiceInput = (uint8_t *)(m_mapOfRxedBlocks.back().data());
  pServiceInput = (uint8_t *)(data->getContent().data()); // this points to the first byte, which is the TLV-TYPE (21 for data packet content)
  pServiceInput++;  // now this points to the second byte, containing 253 (0xFD), meaning size (1024) is expressed with 2 octets
  pServiceInput++;  // now this points to the first size octet
  pServiceInput++;  // now this points to the second size octet
  pServiceInput++;  // now we are pointing at the first byte of the true content
  //m_mapOfServiceInputs[rxedDataName] = (*pServiceInput);

  // we keep track of which input is for which interest that was sent out. Data packets may arrive in different order than how interests were sent out.
  // index = look for rxedDataName in m_mapOfRxedBlocks and figure out what index it is using!
  unsigned char index = 0;
  for (auto i = (m_mapOfRxedBlocks[m_service.toUri()]).begin(); i != (m_mapOfRxedBlocks[m_service.toUri()]).end(); ++i) {
    if ((*i).compare(rxedDataName) == 0) {
      break;
    }
    index++;
  }
  m_vectorOfServiceInputs[index] = (*pServiceInput);

  m_numRxedInputs++;
  //TODO: this just checks for number of inputs received. I should check if all inputs have been received (this won't work if I need two inputs, but I receive the same input twice)

  // we have to check if we have received all necessary inputs for this instance of the hosted service!
  //      if so, run the service below and generate new data packet to go downstream.
  //      otherwise, just wait for the other inputs.
  if (m_numRxedInputs == m_inputTotal)
  {

    m_inputTotal = 0;     // reset for next time service is called? so that we start fresh with next interest?
    m_numRxedInputs = 0;  // reset for next time service is called? so that we wait for new inputs?

    //"RUN" the service, and create a new data packet to respond downstream
    //NS_LOG_DEBUG("Fake running service " << m_service);
    NS_LOG_DEBUG("Running service " << m_service);

    // run operation. First we need to figure out what service this is, so we know the operation. This screams to be a function pointer! For now just use if's

    // TODO: we should use function pointers here, and have each service be a function defined in a separate file. Figure out how to deal with potentially different num of inputs.

    if (m_service.ndn::Name::toUri() == "/service1"){
      serviceOutput = (m_vectorOfServiceInputs[0])*2;
    }
    if (m_service.ndn::Name::toUri() == "/service2"){
      serviceOutput = (m_vectorOfServiceInputs[0])+1;
    }
    if (m_service.ndn::Name::toUri() == "/service3"){
      serviceOutput = (m_vectorOfServiceInputs[0])+7;
    }
    if (m_service.ndn::Name::toUri() == "/service4"){
      serviceOutput = (m_vectorOfServiceInputs[0])*3 + (m_vectorOfServiceInputs[1])*4;
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
  else
  {
    NS_LOG_DEBUG("    Even though we received data packet, we are still waiting for more inputs! Rxed so far: " << m_numRxedInputs);
    NS_LOG_DEBUG("    This service needs this many inputs: " << m_inputTotal);
  }
  
}

} // namespace ns3
