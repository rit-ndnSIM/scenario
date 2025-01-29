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

// The orchestrator should receive the interest from the consumer (with JSON DAG as a parameter), read the DAG workflow, and use it to
// coordinate interest generation.
// Start with an interest for the root service(s) - the most upstream service(s)!!
// Keep track of received data packets (results) for each service.
// Once all results from upstream services are received, we can then generate the interest for the downstream service they feed.
// Eventually, respond to the consumer with the final data result.


// cabeee-dag-orchestratorB-app.cpp

#include "cabeee-dag-orchestratorB-app.hpp"

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

NS_LOG_COMPONENT_DEFINE("DagOrchestratorB_App");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED(DagOrchestratorB_App);

// register NS-3 type
TypeId
DagOrchestratorB_App::GetTypeId()
{
  static TypeId tid = TypeId("DagOrchestratorB_App")
    .SetParent<ndn::App>()
    .AddConstructor<DagOrchestratorB_App>()
    .AddAttribute("Prefix", "Requested prefix", StringValue("/dumb-interest"),
                    ndn::MakeNameAccessor(&DagOrchestratorB_App::m_prefix), ndn::MakeNameChecker())
    .AddAttribute("Service", "Requested service", StringValue("dumb-service"),
                    ndn::MakeNameAccessor(&DagOrchestratorB_App::m_service), ndn::MakeNameChecker());
  return tid;
}

DagOrchestratorB_App::DagOrchestratorB_App()
  : m_isRunning(false)
{
}

// Processing upon start of the application
void
DagOrchestratorB_App::StartApplication()
{
  // initialize ndn::App
  ndn::App::StartApplication();
  m_isRunning = true;

  m_name = m_prefix.ndn::Name::toUri() + m_service.ndn::Name::toUri();

  // Add entry to FIB for `/prefix/sub`
  //ndn::FibHelper::AddRoute(GetNode(), "/prefix/sub", m_face, 0); //cabeee took this out, let the global router figure it out.
  ndn::FibHelper::AddRoute(GetNode(), m_name, m_face, 0);

  m_serviceInputIndex = 0;
  m_lowestFreshness = ndn::time::milliseconds(100000); // set to a high value (I know no producer freshness value is higher than 100 seconds)

}

// Processing when application is stopped
void
DagOrchestratorB_App::StopApplication()
{
  m_isRunning = false;
  // cleanup ndn::App
  ndn::App::StopApplication();
}







void
DagOrchestratorB_App::SendInterest(const std::string& interestName, std::string dagString)
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
  interest->setInterestLifetime(ndn::time::seconds(10));
  interest->setMustBeFresh(false);



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



  NS_LOG_DEBUG("Orchestrator: Sending Interest packet for " << *interest);

  // Call trace (for logging purposes)
  m_transmittedInterests(interest, this, m_face);

  m_appLink->onReceiveInterest(*interest);
}





// Callback that will be called when Interest arrives
void
DagOrchestratorB_App::OnInterest(std::shared_ptr<const ndn::Interest> interest)
{
  ndn::App::OnInterest(interest);

  ndn::Name nameAndDigest = interest->getName();
  //m_nameUri = nameAndDigest.ndn::Name::toUri();
  //ndn::Name tempName = nameAndDigest.getPrefix(-1); // remove the last component of the name (the parameter digest) so we have just the raw name
  //tempName = tempName.getPrefix(2); // keep only the first two components of the name
  //m_nameUri = tempName.toUri(); // convert to Uri string
  //std::string nameUri = nameAndDigest.getSubName(0,2).toUri(); // extract 2 components starting from component 0, and then convert to Uri string
  std::string nameUri = nameAndDigest.getSubName(1,2).toUri(); // extract 2 components starting from component 1 (skip /interCACHE prefix), and then convert to Uri string

  NS_LOG_DEBUG("Received Interest packet for " << nameUri << ", full name with digest: " << nameAndDigest);
  //NS_LOG_DEBUG("Received on node hosting service " << m_service);

  //NS_LOG_DEBUG("Interest packet URI: " << nameUri);

  if (nameUri == "/serviceOrchestration/dag") // if interest is for orchestration (from consumer)
  {

    m_nameAndDigest = interest->getName();  // store the name with digest so that we can later generate the final result data packet with the same name/digest!

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






    // read the dag parameters and figure out which interest(s) to generate (only for those that don't have inputs coming from other services - "root" services).
    m_dagObject = json::parse(dagString);


    // here we generate just the first interest(s) according to the workflow (not backwards as done originally in our proposed forwarder-based methodology).
    // to do this, we must discover which services in the DAG are "root" services (services which don't have inputs coming from other services, for example: sensors).


    m_listOfServicesWithInputs.clear();
    m_listOfRootServices.clear();
    m_listOfSinkNodes.clear();
    for (auto& x : m_dagObject["dag"].items())
    {
      m_listOfRootServices.push_back(x.key()); // for now, add ALL keys to the list, we'll remove non-root ones later
      for (auto& y : m_dagObject["dag"][x.key()].items())
      {
        m_listOfServicesWithInputs.push_back(y.key()); // add all values to the list
        if ((std::find(m_listOfSinkNodes.begin(), m_listOfSinkNodes.end(), y.key()) == m_listOfSinkNodes.end())) // if y.key() does not exist in m_listOfSinkNodes
        {
          m_listOfSinkNodes.push_back(y.key()); // for now, add ALL values to the list, we'll remove non-sinks later
        }
      }
    }

    // now remove services that feed into other services from the list of sink nodes
    for (auto& x : m_dagObject["dag"].items())
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




    // create the tracking data structure using JSON
    json nullJson;
    for (auto& x : m_dagObject["dag"].items()) // first pass just gets the source services (don't worry what they feed)
    {
      unsigned char isRoot = 0;
      if (!(std::find(m_listOfRootServices.begin(), m_listOfRootServices.end(), x.key()) == m_listOfRootServices.end())) // if x.key() exists in m_listOfRootServices
        isRoot = 1;
      m_dagOrchTracker.push_back( json::object_t::value_type(x.key(), {{"root", isRoot}} ) );
      m_dagOrchTracker[x.key()].push_back( json::object_t::value_type("interestGenerated", 0 ) );
      m_dagOrchTracker[x.key()].push_back( json::object_t::value_type("inputsRxed", nullJson ) );
      m_dagOrchTracker[x.key()].push_back( json::object_t::value_type("inputsTxed", nullJson ) );
    }
    //std::cout << std::setw(2) << m_dagOrchTracker << '\n';
    for (auto& x : m_dagObject["dag"].items())
    {
      for (auto& y : m_dagObject["dag"][x.key()].items())
      {
        if ((std::find(m_listOfSinkNodes.begin(), m_listOfSinkNodes.end(), y.key()) == m_listOfSinkNodes.end())) // if y.key() does not exist in m_listOfSinkNodes
        {
          m_dagOrchTracker[(std::string)y.key()]["inputsRxed"][(std::string)x.key()] = -1;
          m_dagOrchTracker[(std::string)y.key()]["inputsTxed"][(std::string)x.key()] = 0;
          //std::cout << "x.key is " << x.key() << ", and y.key is " << y.key() << '\n';
        }
      }
    }
    //std::cout << "OrchestratorB dagOrchTracker data structure: " << std::setw(2) << m_dagOrchTracker << '\n';


    for (auto rootService : m_listOfRootServices)
    {
      // generate new interest for root services if one has not yet been generated
      NS_LOG_DEBUG("Generating interest for: " << rootService);

      // We need to see if this interest has already been generated. If so, don't increment
      // if this is a new interest (if interest is not in our list of generated interests)
      //if ((std::find(m_listOfGeneratedInterests.begin(), m_listOfGeneratedInterests.end(), rootService) == m_listOfGeneratedInterests.end())) { // if we don't find it...
        // add this new interest to list of generated interests
        //m_listOfGeneratedInterests.push_back(rootService);
        DagOrchestratorB_App::SendInterest(rootService, dagString);
        m_dagOrchTracker[rootService]["interestGenerated"] = 1;
      //}
    }

  }

  else if (nameUri == "/serviceOrchestration/data") // if interest is for data input to service (from a service)
  {

    // respond with data that was requested

    // look at 2nd and 3rd components of the name, for example: /serviceOrchestration/data/sensor/service1 + digest which contains DAG
    // requestedService will be for example /sensor
    // requestorService will be for example /service1
    //std::string inputNumString   = nameAndDigest.getSubName(2,1).toUri(); // extract 1 components starting from component 2, and then convert to Uri string
    //std::string requestedService = nameAndDigest.getSubName(3,1).toUri(); // extract 1 components starting from component 3, and then convert to Uri string
    //std::string requestorService = nameAndDigest.getSubName(4,1).toUri(); // extract 1 components starting from component 4, and then convert to Uri string
    std::string inputNumString   = nameAndDigest.getSubName(3,1).toUri(); // extract 1 components starting from component 3 (skip /interCACHE prefix), and then convert to Uri string
    std::string requestedService = nameAndDigest.getSubName(4,1).toUri(); // extract 1 components starting from component 4 (skip /interCACHE prefix), and then convert to Uri string
    std::string requestorService = nameAndDigest.getSubName(5,1).toUri(); // extract 1 components starting from component 5 (skip /interCACHE prefix), and then convert to Uri string
    inputNumString = inputNumString.erase(0,1); // remove the "/" at the beginning of the Uri Name, to leave just the number (still as a string)
    unsigned char inputNum = stoi(inputNumString);
    NS_LOG_DEBUG("Orchestrator received interest for: " << nameUri << ", service name: " << requestedService << ", stored at index " << inputNumString << ", by requestor: " << requestorService);

    // look at data structure and figure out which index is used for the stored result for requestedService
    for (auto& service : m_dagOrchTracker.items())  // for each service in the tracker
    {
      if (service.key() == requestorService)
      {
        NS_LOG_DEBUG("Found requestor service!");
        for (auto& serviceInput : m_dagOrchTracker[(std::string)service.key()]["inputsRxed"].items())
        {
          if (serviceInput.key() == requestedService) // there might be more than one service that uses this input, but we already matched to the requestor above.
          {
            NS_LOG_DEBUG("Found requested service result!");
            unsigned char inputStorageIndex = serviceInput.value();

            // send that data at that index as a response for this interest
            NS_LOG_DEBUG("Generating data packet for: " << nameAndDigest.ndn::Name::toUri());
            auto new_data = std::make_shared<ndn::Data>(nameAndDigest);
            //new_data->setFreshnessPeriod(ndn::time::milliseconds(0)); // we set the freshness to zero, so that it is not cached. We can't cache it because future requests need to be served from the orchestrator to keep the dagOrchTracker updated.
            new_data->setFreshnessPeriod(ndn::time::milliseconds(m_lowestFreshness));
            //new_data->setContent(std::make_shared< ::ndn::Buffer>(1024));
            unsigned char myBuffer[1024];
            // write to the buffer
            myBuffer[0] = m_vectorOfServiceInputs[inputStorageIndex];
            new_data->setContent(myBuffer, 1024);
            ndn::StackHelper::getKeyChain().sign(*new_data);
            // Call trace (for logging purposes)
            m_transmittedDatas(new_data, this, m_face);
            m_appLink->onReceiveData(*new_data);

            // update tracker data structure to mark data as having been sent (set inputsTxed value to 1)
            m_dagOrchTracker[(std::string)service.key()]["inputsTxed"][(std::string)serviceInput.key()] = 1;
            //std::cout << "Updated dagOrchTracker data structure: " << std::setw(2) << m_dagOrchTracker << '\n';
          }
        }
      }
    }




    // if all data inputs for the downstream service have been sent out, then generate an interest for the service to run
    // we check to see which services that we have not generated interests for yet have all their inputs satisfied (Txed to the service), and generate new interest if satisfied.
    // note there might be more than one service that needs to run next (ex: in the RPA DAG, when we receive R1, we can now send results to (and then run) S2 AND S3)
    for (auto& service : m_dagOrchTracker.items())  // for each service in the tracker
    {
      if (service.value()["interestGenerated"] == 0)  // if we haven't generated an interest for this service, then analyze its rxed inputs
      {
        unsigned char allInputsTxed = 1;
        for (auto& serviceInput : m_dagOrchTracker[(std::string)service.key()]["inputsTxed"].items())
        {
          if (serviceInput.value() == 0)
          {
            allInputsTxed = 0;
            break;
          }
        }
        if (allInputsTxed == 1)
        {
          // generate the interest request for the data inputs for this service, and mark it down as generated
          std::string dagString = m_dagObject.dump();
          DagOrchestratorB_App::SendInterest(service.key(), dagString);
          service.value()["interestGenerated"] = 1;
        }
      }
    }
  }

  // Note that Interests send out by the app will not be sent back to the app !

  /*
  auto data = std::make_shared<ndn::Data>(interest->getName());
  data->setFreshnessPeriod(ndn::time::milliseconds(9000));
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
DagOrchestratorB_App::OnData(std::shared_ptr<const ndn::Data> data)
{
  NS_LOG_DEBUG("Receiving Data packet for " << data->getName());

  //std::cout << "DATA received for name " << data->getName() << std::endl;

  //std::cout << "content = " << data->getContent() << std::endl;

  ndn::Name simpleName;
  simpleName = (data->getName()).getPrefix(-1); // remove the last component of the name (the parameter digest) so we have just the raw name, and convert to Uri string
  simpleName = simpleName.getSubName(1); // remove the first component of the name (/interCACHE)
  //std::string rxedDataName = (data->getName()).getPrefix(-1).toUri(); // remove the last component of the name (the parameter digest) so we have just the raw name
  std::string rxedDataName = simpleName.toUri();


  //std::cout << "Storing the received result at m_vectorOfServiceInputs[" << std::to_string(m_serviceInputIndex) << "]\n";
  // store the received result, so we can later send it to downstream services
  // TODO: this is a HACK. I need a better way to get to the first byte of the content. Right now, I'm just incrementing the pointer past the TLV type, and size.
  // and then getting to the first byte (which is all I'm using for data)
  //unsigned char serviceOutput = 0;
  uint8_t *pServiceInput = 0;
  pServiceInput = (uint8_t *)(data->getContent().data()); // this points to the first byte, which is the TLV-TYPE (21 for data packet content)
  pServiceInput++;  // now this points to the second byte, containing 253 (0xFD), meaning size (1024) is expressed with 2 octets
  pServiceInput++;  // now this points to the first size octet
  pServiceInput++;  // now this points to the second size octet
  pServiceInput++;  // now we are pointing at the first byte of the true content
  //m_mapOfServiceInputs[rxedDataName] = (*pServiceInput);
  m_vectorOfServiceInputs.push_back(*pServiceInput); // add it at the end (should be same as putting it at index m_serviceInputIndex)
  //m_vectorOfServiceInputs[m_serviceInputIndex] = (unsigned char)(*pServiceInput);


  ndn::time::milliseconds data_freshnessPeriod = data->getFreshnessPeriod();
  if (data_freshnessPeriod < m_lowestFreshness) {
    m_lowestFreshness = data_freshnessPeriod;
  }

  //std::cout << "Marking down this input as having been received\n";
  // mark down this input as having been received for all services that use this data as an input
  // default object value is -1 (not received)
  // any other value (0 and up) means it has been received, and has been stored at the index specified by that value
  for (auto& service : m_dagOrchTracker.items())
  {
    for (auto& serviceInput : m_dagOrchTracker[(std::string)service.key()]["inputsRxed"].items())
    {
      if (serviceInput.key() == rxedDataName)
      {
        serviceInput.value() = m_serviceInputIndex;
      }
    }
  }
  //std::cout << "Updated dagOrchTracker data structure: " << std::setw(2) << m_dagOrchTracker << '\n';
  m_serviceInputIndex++; // get it ready for the next data input that we receive






  // now we check to see which services that we have not generated interests for yet have all their inputs satisfied, and generate new data request interests if satisfied.
  // note there might be more than one service that needs data request interests generated so they can run next (ex: in the RPA DAG, when we receive R1, we can now run S2 AND S3)
  for (auto& service : m_dagOrchTracker.items())  // for each service in the tracker
  {
    if (service.value()["interestGenerated"] == 0)  // if we haven't generated an interest for this service, then analyze its rxed inputs
    {
      unsigned char allInputsRxed = 1;
      for (auto& serviceInput : m_dagOrchTracker[(std::string)service.key()]["inputsRxed"].items())
      {
        if (serviceInput.value() == -1)
        {
          allInputsRxed = 0;
          break;
        }
      }
      if (allInputsRxed == 1)
      {
        for (auto& serviceInput : m_dagOrchTracker[(std::string)service.key()]["inputsRxed"].items())
        {
          // generate the interest request for the data inputs for this service, (and mark it down as generated?)

          // this data request name will be: "/service1/datarequest/0/sensor" for example to request data for input # 0, which will come from /sensor
          // we need to cross reference the input index number (0 in this case), by searching for this service in the dag workflow data-structure
          // for example for service 4, which has two inputs, I need to generate 2 requests for inputs, and thus need to get their two indices!
          unsigned char inputNum = -1;
          for (auto& dagServiceSource : m_dagObject["dag"].items())
          {
            if (dagServiceSource.key() == serviceInput.key())
            {
              //for (auto& dagServiceFeed : m_dagObject["dag"][dagServiceSource.key()].items())
              //{
                //if (dagServiceFeed.key() == service.key())
                //{
                  //inputNum = m_dagObject["dag"][(std::string)dagServiceSource.key()][(std::string)dagServiceFeed.key()];
                  //inputNum = dagServiceFeed.value();
                  inputNum = dagServiceSource.value()[service.key()];
                //}
              //}
            }
          }
          if (inputNum < 0)
          {
            std::cout << "  Orchestrator ERROR!! inputNum for m_vectorOfServiceInputs cannot be negative! Something went wrong!!" << '\n';
          }
          else
          {
            //std::cout << "Generating interest request for data inputs for service " << (std::string)service.key() << '\n';
            //std::cout << "Request will have name: " << service.key() << "/dataRequest/" << std::to_string(inputNum) << (std::string)serviceInput.key() << '\n';
            std::string interestRequestName = (std::string)service.key() + "/dataRequest/" + std::to_string(inputNum) + (std::string)serviceInput.key();
            //name = /service1/dataRequest/sensor
            //name = /service.key()/dataRequest/serviceInput.key()
            std::string dagString = m_dagObject.dump();
            DagOrchestratorB_App::SendInterest(interestRequestName, dagString);
            //service.value()["interestGenerated"] = 1; // this is done only AFTER the interest for data is received, data packets are sent to the service, and the interest to run the service is also sent!
          }
        }
      }
    }
  }



  // if the result of the last service has been received, then generate data packet with the final result for the consumer.
  for (auto sinkNode : m_listOfSinkNodes)
  {
    //if this data packet feeds sink node
    //std::cout << "        CHECKING SINK: rxedData is " << rxedDataName << ", and current sink node is " << sinkNode << '\n';
    for (auto& serviceFeed : m_dagObject["dag"][rxedDataName].items())
    {
      if (sinkNode == serviceFeed.key())
      {
        NS_LOG_DEBUG("Final data packet! Creating data for name: " << m_nameAndDigest << ", with freshness" << m_lowestFreshness << std::endl);   // m_name doesn't have the sha256 digest, so it doesn't match the original interest!
                                                                                                                                                  // We use m_nameAndDigest to store the old name with the digest.
        auto new_data = std::make_shared<ndn::Data>(m_nameAndDigest);
        new_data->setFreshnessPeriod(ndn::time::milliseconds(m_lowestFreshness));

        new_data->setContent(data->getContent());
        ndn::StackHelper::getKeyChain().sign(*new_data);
        // Call trace (for logging purposes)
        m_transmittedDatas(new_data, this, m_face);
        m_appLink->onReceiveData(*new_data);

        // now that we have run the service (and sent the result data out - and caching it), we set inputs to "not received"
        // this is done so when cached results expire due to freshness, any new interests will trigger inputs to be fetched again, and the service will run again.
        /*
        for (auto& service : m_dagOrchTracker.items())  // for each service in the tracker
        {
          for (auto& serviceInput : m_dagOrchTracker[(std::string)service.key()]["inputsRxed"].items())
          {
            serviceInput.value() = -1;
          }
          for (auto& serviceInput : m_dagOrchTracker[(std::string)service.key()]["inputsTxed"].items())
          {
            serviceInput.value() = 0;
          }
          service.value()["interestGenerated"] = 0;
        }
        */
        m_dagOrchTracker.clear();
        //m_dagObject.clear(); // can't delete here, as we are still using it for iteration. No need to delete anyways, as we overwrite the old one when a workflow interest comes in from the user.
        m_vectorOfServiceInputs.erase(m_vectorOfServiceInputs.begin(), m_vectorOfServiceInputs.end());
        m_listOfServicesWithInputs.clear();
        m_listOfRootServices.clear();
        //m_listOfSinkNodes.clear(); // can't delete here, as we are still using it for iteration. Delete above when we receive the workflow request from the user.
        m_serviceInputIndex = 0;
        m_lowestFreshness = ndn::time::milliseconds(100000); // set to a high value (I know no producer freshness value is higher than 100 seconds)
        //std::cout << "Updated dagOrchTracker data structure: " << std::setw(2) << m_dagOrchTracker << '\n';

      }
    }
  }

}

} // namespace ns3
