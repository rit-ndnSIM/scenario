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

// cabeee-dag-serviceB-app.cpp

#include "cabeee-dag-serviceB-app.hpp"

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

NS_LOG_COMPONENT_DEFINE("DagServiceB_App");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED(DagServiceB_App);

// register NS-3 type
TypeId
DagServiceB_App::GetTypeId()
{
  static TypeId tid = TypeId("DagServiceB_App")
    .SetParent<ndn::App>()
    .AddConstructor<DagServiceB_App>()
    .AddAttribute("Prefix", "Requested prefix", StringValue("/dumb-interest"),
                    ndn::MakeNameAccessor(&DagServiceB_App::m_prefix), ndn::MakeNameChecker())
    .AddAttribute("Service", "Requested service", StringValue("dumb-service"),
                    ndn::MakeNameAccessor(&DagServiceB_App::m_service), ndn::MakeNameChecker());
  return tid;
}

DagServiceB_App::DagServiceB_App()
  : m_isRunning(false)
{
}

// Processing upon start of the application
void
DagServiceB_App::StartApplication()
{
  // initialize ndn::App
  ndn::App::StartApplication();
  m_isRunning = true;

  m_name = m_prefix.ndn::Name::toUri() + m_service.ndn::Name::toUri();

  // Add entry to FIB for `/prefix/sub`
  //ndn::FibHelper::AddRoute(GetNode(), "/prefix/sub", m_face, 0); //cabeee took this out, let the global router figure it out.
  ndn::FibHelper::AddRoute(GetNode(), m_name, m_face, 0);


  m_done = false;
  m_extracted = false;
  m_nameUri = m_service.ndn::Name::toUri();

}

// Processing when application is stopped
void
DagServiceB_App::StopApplication()
{
  m_isRunning = false;
  // cleanup ndn::App
  ndn::App::StopApplication();
}





void
DagServiceB_App::SendInterest(const std::string& interestName, std::string dagString)
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
  auto interest = std::make_shared<ndn::Interest>(m_prefix.ndn::Name::toUri() + interestName);
  Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();
  interest->setNonce(rand->GetValue(0, std::numeric_limits<uint32_t>::max()));
  interest->setInterestLifetime(ndn::time::seconds(5));
  //interest->setMustBeFresh(true);



  // overwrite the dag parameter "head" value and generate new application parameters (thus calculating new sha256 digest)
  //std::string dagString = std::string(reinterpret_cast<const char*>(dagParameter.value()), dagParameter.value_size());
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



  NS_LOG_DEBUG("Service: Sending Interest packet for " << *interest);

  // Call trace (for logging purposes)
  m_transmittedInterests(interest, this, m_face);

  m_appLink->onReceiveInterest(*interest);
}






// Callback that will be called when Interest arrives
void
DagServiceB_App::OnInterest(std::shared_ptr<const ndn::Interest> interest)
{
  ndn::App::OnInterest(interest);


  NS_LOG_DEBUG("Received Interest packet for " << interest->getName());
  //NS_LOG_DEBUG("Received on node hosting service " << m_service);


  if (m_extracted == false)
  {
    // extract the NAME of the inputs from the DAG in the interest parameter, and generate new interests for input data

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

    // read the dag parameter and figure out which interests to generate next.
    // if inputs come from other services, Ochestrator must have made sure the service results are already available (received "done" signal)!
    m_dagObject = json::parse(dagString);


    // create the tracking data structure using JSON
    json nullJson;
    //m_dagServTracker[m_nameUri].push_back( json::object_t::value_type("interestGenerated", 0 ) );
    m_dagServTracker[m_nameUri].push_back( json::object_t::value_type("inputsRxed", nullJson ) );
    for (auto& x : m_dagObject["dag"].items())
    {
      for (auto& y : m_dagObject["dag"][x.key()].items())
      {
        if (y.key() == m_nameUri)
        {
          m_dagServTracker[(std::string)y.key()]["inputsRxed"][(std::string)x.key()] = 0;
          //std::cout << "x.key is " << x.key() << ", and y.key is " << y.key() << '\n';

          m_vectorOfServiceInputs.push_back(0);             // for now, just create vector entries for the inputs, so that if they arrive out of order, we can insert at any index location
        }
      }
    }
    //std::cout << "ServiceB dagServTracker data structure: " << std::setw(2) << m_dagServTracker << '\n';

    NS_LOG_DEBUG("Creating data for name: " << m_nameAndDigest);  // m_name doesn't have the sha256 digest, so it doesn't match the original interest!
                                                                  // We use m_nameAndDigest to store the old name with the digest.
    m_extracted = true;
  }

  // IF we received a data request interest (ex: /service1/dataRequest/0/sensor), then generate the interest
  //else, continue below since it is a regular interest for the service itself
  ndn::Name nameAndDigest = interest->getName();  // store the name with digest
  //std::string nameUri = nameAndDigest.getSubName(1,1).toUri(); // extract 1 component starting from component 1, and then convert to Uri string
  std::string nameUri = nameAndDigest.getSubName(2,1).toUri(); // extract 1 component starting from component 2 (skip /interCACHE prefix), and then convert to Uri string
  NS_LOG_DEBUG("  NAME COMPONENT 1: " << nameUri);
  if (nameUri == "/dataRequest")
  {
    //generate interest for sensor
    //std::string requestNameUri = nameAndDigest.getSubName(2,2).toUri(); // extract 2 components starting from component 2, and then convert to Uri string
    std::string requestNameUri = nameAndDigest.getSubName(3,2).toUri(); // extract 2 components starting from component 3 (skip /interCACHE prefix), and then convert to Uri string
    requestNameUri = "/serviceOrchestration/data" + requestNameUri + m_nameUri; // ask for requestNameUri data, and let the orchestrator know who it's coming from (so it can me marked as txed when it responds)
    NS_LOG_DEBUG("Interest was for dataRequest, thus generating interest for " << requestNameUri);

    // generate the interest for this input
    auto dagParameterFromInterest = interest->getApplicationParameters();
    std::string dagString = std::string(reinterpret_cast<const char*>(dagParameterFromInterest.value()), dagParameterFromInterest.value_size());
    DagServiceB_App::SendInterest(requestNameUri, dagString);
  }

  else
  {

    m_nameAndDigest = interest->getName();  // store the name with digest so that we can later generate the data packet with the same name/digest!
                                            // TODO: this has issues - we cannot use the same service in more than one location in the DAG workflow!
                                            // for this, we would need to be able to store and retrieve unique interests and their digest (perhaps using a fully hierarchical name?)
                                    // right now, this application only has one m_nameAndDigest private variable, and thus can only "store" one service instance.
                                    // we could turn that variable into a list of ndn::Name variables, and add to the list for each instance of the service?

    // check to see if we have already ran this service with the included DAG. If so, just respond with the results.
    // OR we can let the content store caching deal with responding
    if (m_done == true)
    {
      NS_LOG_DEBUG("    ServiceB: We already ran this service before. Responding with internally stored result!");
      // send stored result
      //auto new_data = std::make_shared<ndn::Data>(new_name);
      auto new_data = std::make_shared<ndn::Data>(m_nameAndDigest);
      new_data->setFreshnessPeriod(ndn::time::milliseconds(9000));
      //new_data->setContent(std::make_shared< ::ndn::Buffer>(1024));
      unsigned char myBuffer[1024];
      // write to the buffer
      myBuffer[0] = m_serviceOutput;
      new_data->setContent(myBuffer, 1024);
      ndn::StackHelper::getKeyChain().sign(*new_data);

      // Call trace (for logging purposes)
      m_transmittedDatas(new_data, this, m_face);
      m_appLink->onReceiveData(*new_data);
      return;
    }



    // check which data inputs have not yet been received! Technically, the orchestrator should have already sent us all of them before genreating the main interest for service
    // generate all the interests for required inputs
    for (auto& serviceInput : m_dagServTracker[(std::string)m_nameUri]["inputsRxed"].items())
    {
      if (serviceInput.value() == 0)
      {
        NS_LOG_DEBUG("    ServiceB: ERROR! We should have already received this input, but somehow haven't: " << serviceInput.key());
        // generate the interest for this input
        std::string dagString = m_dagObject.dump();
        DagServiceB_App::SendInterest(serviceInput.key(), dagString);
      }
    }



    


   

    //TODO: if the service doesn't need any inputs, we should run the service and respond with the data packet right from here.
    // with the RPA DAG, this doesn't apply, but we could have a workflow with services that don't require any input (no other interests need to be generated here).
  }

}


// Callback that will be called when Data arrives
void
DagServiceB_App::OnData(std::shared_ptr<const ndn::Data> data)
{
  NS_LOG_DEBUG("Receiving Data packet for " << data->getName());

  //std::cout << "DATA received for name " << data->getName() << std::endl;

  //std::cout << "content = " << data->getContent() << std::endl;

  std::string rxedDataName;

  // we need to make sure we check the first two parts of the name on the incomming data packets from the orchestrator
  //        for example: /serviceOrchestration/data/0/sensor/service1
  //        if the name starts with serviceOrcehstration/data, then we must treat them differently than a regular data packet
  ndn::Name nameAndDigest = (data->getName()).getPrefix(-1); // remove the last component of the name (the parameter digest) so we have just the raw name
  //std::string firstTwo = nameAndDigest.getSubName(0,2).toUri(); // extract 2 components starting from component 0, and then convert to Uri string
  std::string firstTwo = nameAndDigest.getSubName(1,2).toUri(); // extract 2 components starting from component 1 (skip /interCACHE prefix), and then convert to Uri string

  std::string inputNumString;
  std::string requestedService;
  std::string requestorService;
  unsigned char inputNum;
  if (firstTwo == "/serviceOrchestration/data")
  {
    //inputNumString   = nameAndDigest.getSubName(2,1).toUri(); // extract 1 components starting from component 2, and then convert to Uri string
    //requestedService = nameAndDigest.getSubName(3,1).toUri(); // extract 1 components starting from component 3, and then convert to Uri string
    //requestorService = nameAndDigest.getSubName(4,1).toUri(); // extract 1 components starting from component 4, and then convert to Uri string
    inputNumString   = nameAndDigest.getSubName(3,1).toUri(); // extract 1 components starting from component 3 (skip /interCACHE prefix), and then convert to Uri string
    requestedService = nameAndDigest.getSubName(4,1).toUri(); // extract 1 components starting from component 4 (skip /interCACHE prefix), and then convert to Uri string
    requestorService = nameAndDigest.getSubName(5,1).toUri(); // extract 1 components starting from component 5 (skip /interCACHE prefix), and then convert to Uri string
    inputNumString = inputNumString.erase(0,1); // remove the "/" at the beginning of the Uri Name, to leave just the number (still as a string)
    inputNum = stoi(inputNumString);
    NS_LOG_DEBUG("Service received data for: " << firstTwo << ", requested service name: " << requestedService << ", stored at index " << inputNumString << ", by requestor: " << requestorService);
    rxedDataName = requestedService;
  }

  else
  {
    ndn::Name simpleName;
    simpleName = (data->getName()).getPrefix(-1); // remove the last component of the name (the parameter digest) so we have just the raw name, and convert to Uri string
    simpleName = simpleName.getSubName(1); // remove the first component of the name (/interCACHE)
    //rxedDataName = (data->getName()).getPrefix(-1).toUri(); // remove the last component of the name (the parameter digest) so we have just the raw name, and convert to Uri string
    rxedDataName = simpleName.toUri();
  }

  // TODO: this is a HACK. I need a better way to get to the first byte of the content. Right now, I'm just incrementing the pointer past the TLV type, and size.
  // and then getting to the first byte (which is all I'm using for data)
  uint8_t *pServiceInput = 0;
  //pServiceInput = (uint8_t *)(m_mapOfRxedBlocks.back().data());
  pServiceInput = (uint8_t *)(data->getContent().data()); // this points to the first byte, which is the TLV-TYPE (21 for data packet content)
  pServiceInput++;  // now this points to the second byte, containing 253 (0xFD), meaning size (1024) is expressed with 2 octets
  pServiceInput++;  // now this points to the first size octet
  pServiceInput++;  // now this points to the second size octet
  pServiceInput++;  // now we are pointing at the first byte of the true content
  //m_mapOfServiceInputs[rxedDataName] = (*pServiceInput);

  // we keep track of which input is for which interest that was sent out. Data packets may arrive in different order than how interests were sent out.
  // just read the index from the dagObject JSON structure
  char index = -1;
  for (auto& x : m_dagObject["dag"].items())
  {
    //std::cout << "  Evaluating source service " << (std::string)x.key() << '\n';
    if (x.key() == rxedDataName)
    {
      //std::cout << "  Found rxedDataName in dagObject! Looking for feed service now..." << '\n';
      for (auto& y : m_dagObject["dag"][x.key()].items())
      {
        //std::cout << "  Evaluating feed service " << (std::string)y.key() << '\n';
        if (y.key() == m_nameUri)
        {
          //std::cout << "  HIT, y.key(): " << y.key() << ", y.value(): " << y.value() << '\n';
          index = y.value().template get<char>();
        }
      }
    }
  }
  if (index != inputNum)
  {
    std::cout << "  ServiceB ERROR!! index inputs don't match! Something went wrong!!" << '\n';
    std::cout << "  Received index " << inputNumString << ", but dagOrchTracker has index " << std::to_string(index) << '\n';
  }
  if (index < 0)
  {
    std::cout << "  ServiceB ERROR!! index for m_vectorOfServiceInputs cannot be negative! Something went wrong!!" << '\n';
  }
  else
  {
    m_vectorOfServiceInputs[index] = (*pServiceInput);
  }

  // mark this input as having been received
  m_dagServTracker[m_nameUri]["inputsRxed"][rxedDataName] = 1;

  // we have to check if we have received all necessary inputs for this instance of the hosted service!
  //      if so, run the service below and generate new data packet to go downstream.
  //      otherwise, just wait for the other inputs.
  unsigned char allInputsReceived = 1;
  for (auto& serviceInput : m_dagServTracker[m_nameUri]["inputsRxed"].items())
  {
    if (serviceInput.value() == 0)
    {
      allInputsReceived = 0;
      break;
    }
  }
  if (allInputsReceived == 1)
  {
    //"RUN" the service, and create a new data packet to respond downstream
    NS_LOG_DEBUG("Running service " << m_service);

    // run operation. First we need to figure out what service this is, so we know the operation. This screams to be a function pointer! For now just use if's

    // TODO: we should use function pointers here, and have each service be a function defined in a separate file. Figure out how to deal with potentially different num of inputs.

    if (m_service.ndn::Name::toUri() == "/service1"){
      m_serviceOutput = (m_vectorOfServiceInputs[0])*2;
    }
    if (m_service.ndn::Name::toUri() == "/service2"){
      m_serviceOutput = (m_vectorOfServiceInputs[0])+1;
    }
    if (m_service.ndn::Name::toUri() == "/service3"){
      m_serviceOutput = (m_vectorOfServiceInputs[0])+7;
    }
    if (m_service.ndn::Name::toUri() == "/service4"){
      m_serviceOutput = (m_vectorOfServiceInputs[0])*3 + (m_vectorOfServiceInputs[1])*4;
    }
    if (m_service.ndn::Name::toUri() == "/service5"){
      m_serviceOutput = (m_vectorOfServiceInputs[0])*2;
    }
    if (m_service.ndn::Name::toUri() == "/service6"){
      m_serviceOutput = (m_vectorOfServiceInputs[0])+1;
    }
    if (m_service.ndn::Name::toUri() == "/service7"){
      m_serviceOutput = (m_vectorOfServiceInputs[0])+7;
    }
    if (m_service.ndn::Name::toUri() == "/service8"){
      m_serviceOutput = (m_vectorOfServiceInputs[0])*1 + (m_vectorOfServiceInputs[1])*1;
    }
    if ((m_service.ndn::Name::toUri() == "/serviceL1") ||
        (m_service.ndn::Name::toUri() == "/serviceL2") ||
        (m_service.ndn::Name::toUri() == "/serviceL3") ||
        (m_service.ndn::Name::toUri() == "/serviceL4") ||
        (m_service.ndn::Name::toUri() == "/serviceL5") ||
        (m_service.ndn::Name::toUri() == "/serviceL6") ||
        (m_service.ndn::Name::toUri() == "/serviceL7") ||
        (m_service.ndn::Name::toUri() == "/serviceL8") ||
        (m_service.ndn::Name::toUri() == "/serviceL9") ||
        (m_service.ndn::Name::toUri() == "/serviceL10") ||
        (m_service.ndn::Name::toUri() == "/serviceL11") ||
        (m_service.ndn::Name::toUri() == "/serviceL12") ||
        (m_service.ndn::Name::toUri() == "/serviceL13") ||
        (m_service.ndn::Name::toUri() == "/serviceL14") ||
        (m_service.ndn::Name::toUri() == "/serviceL15") ||
        (m_service.ndn::Name::toUri() == "/serviceL16") ||
        (m_service.ndn::Name::toUri() == "/serviceL17") ||
        (m_service.ndn::Name::toUri() == "/serviceL18") ||
        (m_service.ndn::Name::toUri() == "/serviceL19") ||
        (m_service.ndn::Name::toUri() == "/serviceL20")){
      m_serviceOutput = (m_vectorOfServiceInputs[0])+1;
    }
    if ((m_service.ndn::Name::toUri() == "/serviceP1") ||
        (m_service.ndn::Name::toUri() == "/serviceP2") ||
        (m_service.ndn::Name::toUri() == "/serviceP3") ||
        (m_service.ndn::Name::toUri() == "/serviceP4") ||
        (m_service.ndn::Name::toUri() == "/serviceP5") ||
        (m_service.ndn::Name::toUri() == "/serviceP6") ||
        (m_service.ndn::Name::toUri() == "/serviceP7") ||
        (m_service.ndn::Name::toUri() == "/serviceP8") ||
        (m_service.ndn::Name::toUri() == "/serviceP9") ||
        (m_service.ndn::Name::toUri() == "/serviceP10") ||
        (m_service.ndn::Name::toUri() == "/serviceP11") ||
        (m_service.ndn::Name::toUri() == "/serviceP12") ||
        (m_service.ndn::Name::toUri() == "/serviceP13") ||
        (m_service.ndn::Name::toUri() == "/serviceP14") ||
        (m_service.ndn::Name::toUri() == "/serviceP15") ||
        (m_service.ndn::Name::toUri() == "/serviceP16") ||
        (m_service.ndn::Name::toUri() == "/serviceP17") ||
        (m_service.ndn::Name::toUri() == "/serviceP18") ||
        (m_service.ndn::Name::toUri() == "/serviceP19") ||
        (m_service.ndn::Name::toUri() == "/serviceP20")){
      m_serviceOutput = (m_vectorOfServiceInputs[0])+1;
    }
    if (m_service.ndn::Name::toUri() == "/serviceP21"){
      m_serviceOutput =
            (m_vectorOfServiceInputs[0])+
            (m_vectorOfServiceInputs[1])+
            (m_vectorOfServiceInputs[2])+
            (m_vectorOfServiceInputs[3])+
            (m_vectorOfServiceInputs[4])+
            (m_vectorOfServiceInputs[5])+
            (m_vectorOfServiceInputs[6])+
            (m_vectorOfServiceInputs[7])+
            (m_vectorOfServiceInputs[8])+
            (m_vectorOfServiceInputs[9])+
            (m_vectorOfServiceInputs[10])+
            (m_vectorOfServiceInputs[11])+
            (m_vectorOfServiceInputs[12])+
            (m_vectorOfServiceInputs[13])+
            (m_vectorOfServiceInputs[14])+
            (m_vectorOfServiceInputs[15])+
            (m_vectorOfServiceInputs[16])+
            (m_vectorOfServiceInputs[17])+
            (m_vectorOfServiceInputs[18])+
            (m_vectorOfServiceInputs[19]);
    }
    
    // we stored the result so we can respond later when the main service interest comes in!!

    NS_LOG_DEBUG("Service " << m_service.ndn::Name::toUri() << " has output: " << (int)m_serviceOutput);
  
    m_done = true;

/*  we no longer respond here. We only respond when the main interest for the service comes in.
    NS_LOG_DEBUG("Creating data for name: " << m_nameAndDigest);  // m_name doesn't have the sha256 digest, so it doesn't match the original interest!
                                                                  // We use m_nameAndDigest to store the old name with the digest.




    //auto new_data = std::make_shared<ndn::Data>(new_name);
    auto new_data = std::make_shared<ndn::Data>(m_nameAndDigest);
    new_data->setFreshnessPeriod(ndn::time::milliseconds(9000));
    //new_data->setContent(std::make_shared< ::ndn::Buffer>(1024));
    unsigned char myBuffer[1024];
    // write to the buffer
    myBuffer[0] = m_serviceOutput;
    new_data->setContent(myBuffer, 1024);
    ndn::StackHelper::getKeyChain().sign(*new_data);

    // Call trace (for logging purposes)
    m_transmittedDatas(new_data, this, m_face);
    m_appLink->onReceiveData(*new_data);


    // in order to "host" the data locally, we must store it, and set a flag that is checked at "onInterest" above
    // This is a one time deal. Assumes data will be available and fresh from now on.
    // OR we can just let the content store deal with storing results

    // so now, we do the same for the copy that we store in m_data as we did for new_data above
    m_done = true;
    //m_data.ndn::setName(m_nameAndDigest);
    //m_data.setFreshnessPeriod(ndn::time::milliseconds(9000));
    //m_data.setContent(myBuffer, 1024);
    //ndn::StackHelper::getKeyChain().sign(*m_data);
*/
  }
  else
  {
    NS_LOG_DEBUG("    Even though we received data packet, we are still waiting for more inputs!");
  }
  
}


} // namespace ns3
