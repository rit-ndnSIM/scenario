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
    .AddAttribute("Prefix", "Requested prefix", StringValue("/dumb-interest"),
                    ndn::MakeNameAccessor(&DagForwarderApp::m_prefix), ndn::MakeNameChecker())
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

  m_name = m_prefix.ndn::Name::toUri() + m_service.ndn::Name::toUri();
  
  // Add entry to FIB for `/prefix/sub`
  //ndn::FibHelper::AddRoute(GetNode(), "/prefix/sub", m_face, 0); //cabeee took this out, let the global router figure it out.
  ndn::FibHelper::AddRoute(GetNode(), m_name, m_face, 0);

  //ndn::FibHelper::AddRoute(GetNode(), "/nesco/shortcutOPT", m_face, 0);
  //NS_LOG_DEBUG("   cabeee application CABEEEshortcutOPT" << m_service << " is showing faceID: " << m_face->getId());
  //ndn::Face::setInterestFilter("/nesco/shortcutOPT", std::bind(&DagForwarderApp::OnInterest, this, _2));
  //m_face->setInterestFilter("/nesco/shortcutOPT", &DagForwarderApp::OnInterest);

  m_nameUri = m_service.ndn::Name::toUri();

  //m_inputTotal = 0;
  //m_numRxedInputs = 0;

  m_lowestFreshness = ndn::time::milliseconds(100000); // set to a high value (I know no producer freshness value is higher than 100 seconds)

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
DagForwarderApp::SendInterest(const std::string& interestName, std::string dagString)
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
  interest->setMustBeFresh(true);



  // overwrite the dag parameter "head" value and generate new application parameters (thus calculating new sha256 digest)
  //std::string dagString = std::string(reinterpret_cast<const char*>(dagParameter.value()), dagParameter.value_size());
  auto dagObject = json::parse(dagString);



  //std::cout << "\n\n\n\n\n\n\n\n\n\n\n\nStarting new SendInterest for " << interestName << '\n';
  //std::cout << "Full DAG as received: " << std::setw(2) << dagObject << '\n';



  // we PRUNE the DAG workflow to not include anything further downstream than this service,
  // so that when caching is implemented, we can reuse intermediate results for other DAG workflows which use portions of the current DAG.

  // start by removing the head service received from downstream
  //std::cout << "DAG before erasing head: " << std::setw(2) << dagObject << '\n';
  dagObject["dag"].erase((std::string)dagObject["head"]);
  //std::cout << "DAG after erasing head: " << std::setw(2) << dagObject << '\n';

  char prunedLastIteration = 1;
  while (prunedLastIteration > 0)
  {
    prunedLastIteration = 0;

    //std::cout << "\n\n\n\nNew main iteration, looking for sink nodes in the following dag: " << std::setw(2) << dagObject << '\n';


    //find Sink Nodes
    std::list <std::string> listOfServicesWithInputs;   // keeps track of which services have inputs
    std::list <std::string> listOfRootServices;         // keeps track of which services don't have any inputs
    std::list <std::string> listOfSinkNodes;            // keeps track of which node doesn't have an output (usually this is just the consumer)
    for (auto& x : dagObject["dag"].items())
    {
      listOfRootServices.push_back(x.key()); // for now, add ALL keys to the list, we'll remove non-root ones later
      for (auto& y : dagObject["dag"][x.key()].items())
      {
        listOfServicesWithInputs.push_back(y.key()); // add all values to the list
        if ((std::find(listOfSinkNodes.begin(), listOfSinkNodes.end(), y.key()) == listOfSinkNodes.end())) // if y.key() does not exist in listOfSinkNodes
        {
          listOfSinkNodes.push_back(y.key()); // for now, add ALL values to the list, we'll remove non-sinks later
        }
      }
    }
    //std::cout << "removing services that feed into other services" << '\n';
    // now remove services that feed into other services from the list of sink nodes
    for (auto& x : dagObject["dag"].items())
    {
      if (!(std::find(listOfSinkNodes.begin(), listOfSinkNodes.end(), x.key()) == listOfSinkNodes.end())) // if x.key() exists in listOfSinkNodes
      {
        listOfSinkNodes.remove(x.key());
      }
    }
    //std::cout << "done finding sink nodes. Num found: " << std::to_string(listOfSinkNodes.size()) << '\n';


    // for each sink node found
    for (auto sinkNode : listOfSinkNodes) // for (each sink node)
    {
      //std::cout << "  Comparing sinkNode: " << sinkNode << " with interestName: " << interestName << '\n';
      //std::cout << "  prunedLastIteration = " << std::to_string(prunedLastIteration) << '\n';
      if (sinkNode != interestName) //this service name must include the "version", ex: "/service2/B"
      {
        // prune sink Node
        //std::cout << "Forwarder prunning current sink node: " << sinkNode << '\n';
        dagObject["dag"].erase(sinkNode);

        // now that the sink node has been pruned, remove it from all feeds from key services. Key services that end up as new sinks will be removed in next iteration.
        for (auto& x : dagObject["dag"].items())
        {
          char prunedLastIterationY = 1;
          while (prunedLastIterationY > 0) // since we have iteration loops that deal with key/value pairs, we can only prune one at a time. If more than one prunning is necessary, we need to re-iterate
          {
            for (auto& y : dagObject["dag"][x.key()].items())
            {
              prunedLastIterationY = 0;
              if (y.key() == sinkNode)
              {
                //std::cout << "   prunning sink node feed, key: " << x.key() << ", feed: " << y.key() << '\n';
                dagObject["dag"][x.key()].erase(y.key());
                //std::cout << "   after prunning feed: " << std::setw(2) << dagObject << '\n';
                prunedLastIterationY++;
                break;
              }
            }
            if (dagObject["dag"][x.key()].size() == 0)
              break;
          }
        }

        prunedLastIteration++;
        break;
      }
    }

    // now prune any keys that are left with no values
    char prunedLastIterationX = 1;
    while (prunedLastIterationX > 0) // since we have iteration loops that deal with key/value pairs, we can only prune one at a time. If more than one prunning is necessary, we need to re-iterate
    {
      prunedLastIterationX = 0;
      for (auto& x : dagObject["dag"].items())
      {
        if (dagObject["dag"][x.key()].size() == 0)
        {
          // x doesn't have any more feeds, we can prune it.
          //std::cout << "   no feeds left, prunning key: " << x.key() << '\n';
          dagObject["dag"].erase(x.key());
          //std::cout << "   after prunning key: " << std::setw(2) << dagObject << '\n';
          prunedLastIterationX++;
          break;
        }
      }
    }

  }

  // DAG now contains current interest service as the only sink

  // Now prune this service (upstream DAG should not contain this service as a source - but leave where it appears as a feed!)
  dagObject["dag"].erase(interestName);
  //std::cout << "All keys pruned: " << std::setw(2) << dagObject << '\n';

  

  dagObject["head"] = interestName;
  //std::cout << "Pruned DAG with new head name: " << std::setw(2) << dagObject << '\n';
  std::string updatedDagString = dagObject.dump();
  // in order to convert from std::string to a char[] datatype we do the following (https://stackoverflow.com/questions/7352099/stdstring-to-char):
  char *dagStringParameter = new char[updatedDagString.length() + 1];
  strcpy(dagStringParameter, updatedDagString.c_str());
  size_t length = strlen(dagStringParameter);



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


  // read the dag parameters and figure out which interests to generate next. Change the dagParameters accordingly (head will be different)
  json dagObject = json::parse(dagString);
  //NS_LOG_DEBUG("Interest parameter head: " << dagObject["head"] << ", m_name attribute: " << m_name.ndn::Name::toUri());
  //if (dagObject["head"] != m_name.ndn::Name::toUri())
  //{
    //NS_LOG_DEBUG("Forwarder app ERROR: received interest with DAG head different than m_name attribute for this service.");
    //NS_LOG_DEBUG("Interest parameter head: " << dagObject["head"] << ", m_name attribute: " << m_name.ndn::Name::toUri());
  //}
  //NS_LOG_DEBUG("Interest parameter number of services: " << dagObject["dag"].size());
  //NS_LOG_DEBUG("Interest parameter sensor feeds " << (dagObject["dag"]["/sensor"].size()) << " services: " << dagObject["dag"]["/sensor"]);
  //NS_LOG_DEBUG("Interest parameter s1 feeds " << (dagObject["dag"]["/S1"].size()) << " services: " << dagObject["dag"]["/S1"]);

  //std::cout << "Full DAG as received: " << std::setw(2) << m_dagObject << '\n';
  NS_LOG_DEBUG("\n\nFull DAG as received: " << std::setw(2) << dagObject << '\n');


  // create the tracking data structure using JSON
  json nullJson;
  ndn::Name simpleName;
  simpleName = (interest->getName()).getPrefix(-1); // remove the last component of the name (the parameter digest) so we have just the raw name, and convert to Uri string
  simpleName = simpleName.getSubName(1,1); // remove the zeroeth component of the name (/nesco). starting at component 1, keep 1 component
  //std::string rxedInterestName = (interest->getName()).getPrefix(-1).toUri(); // remove the last component of the name (the parameter digest) so we have just the raw name, and convert to Uri string
  std::string rxedInterestName = simpleName.toUri();
  //std::cout << "Forwarder rxedInterestName: " << rxedInterestName << '\n';
  //m_dagServTracker[m_nameUri].push_back( json::object_t::value_type("inputsRxed", nullJson ) );
  //m_dagServTracker[rxedInterestName].push_back( json::object_t::value_type("inputsRxed", nullJson ) );



  if (m_dagObject.empty() && m_service.toUri() == dagObject["head"])
  {
    // store the m_dagObject that we will use for regular forwarding (not for shortcut optimization)
    m_dagObject = json::parse(dagString);
  }


  // generate interests for inputs into hosted services early (shortcut optimization to parallelize workflow)
  if (rxedInterestName == "/shortcutOPT")
  {
    if (m_service.toUri() != dagObject["head"])
    {
      // only if we haven't already received a request for the service
      if (m_dagServTracker.empty()) // if we haven't generated an interest for this service, the dagServTracker will be empty
      {
        NS_LOG_DEBUG("\n\nshortcutOPT: We are hosting service " << m_service.toUri() << ", looking for this service in the DAG!" << std::endl);
        for (auto& x : dagObject["dag"].items())
        {
          //std::cout << "Checking x.key: " << (std::string)x.key() << '\n';
          for (auto& y : dagObject["dag"][x.key()].items())
          {
            //std::cout << "Checking y.key: " << (std::string)y.key() << '\n';
            //if (y.key() == m_nameUri)
            if (y.key() == m_service.toUri())
            {
              //std::cout << " FOUND IT!!" << std::endl;
              //std::cout << "Forwarder dagServTracker data structure before: " << std::setw(2) << m_dagServTracker << '\n';
              m_dagServTracker[(std::string)y.key()]["inputsRxed"][(std::string)x.key()] = 0;
              //std::cout << "Forwarder dagServTracker data structure after: " << std::setw(2) << m_dagServTracker << '\n';
              //std::cout << "x.key is " << x.key() << ", and y.key is " << y.key() << '\n';

              m_vectorOfServiceInputs.push_back(0);             // for now, just create vector entries for the inputs, so that if they arrive out of order, we can insert at any index location
            }
          }
        }
        //std::cout << "Forwarder dagServTracker data structure: " << std::setw(2) << m_dagServTracker << '\n';

        NS_LOG_DEBUG("\n\nshortcutOPT: Generarating all interests for required inputs..." << '\n');
        // generate all the interests for required inputs
        //for (auto& serviceInput : m_dagServTracker[(std::string)m_nameUri]["inputsRxed"].items())
        for (auto& serviceInput : m_dagServTracker[(std::string)m_service.toUri()]["inputsRxed"].items())
        {
          if (serviceInput.value() == 0)
          {
            // generate the interest for this input, sendInterest will prune the DAG and set the head properly
            std::string dagString = dagObject.dump();
            NS_LOG_DEBUG("\n\nshortcutOPT: Generating interest for " << serviceInput.key() << '\n');
            DagForwarderApp::SendInterest(serviceInput.key(), dagString);
          }
        }

        m_nameAndDigest = interest->getName();   // store the name with digest so that we can later generate the data packet with the same name/digest!
                                                // TODO6: this has issues - we cannot use the same service in more than one location in the DAG workflow!
                                                // for this, we would need to be able to store and retrieve unique interests and their digest (perhaps using a fully hierarchical name?)
                                        // right now, this application only has one m_nameAndDigest private variable, and thus can only "store" one service instance.
                                        // we could turn that variable into a list of ndn::Name variables, and add to the list for each instance of the service?
                                    // we also have only one m_vectorOfServiceInputs[], so we would need a list of them, one for each version of the service.
                                    // once a service is fullfilled, we should remove the list items for it to clean up.

        //TODO: for now, I just force the single m_nameUri to be the same as the received interest name.
        m_nameUri = m_service.toUri();
      }
    }
    //else
    //{
      //  std::cout << "   m_service is the same as dag head, dropping this interest." << std::endl;
    //}
  }

  else // not dealing with a shortcut optimization interest
  {


    for (auto& x : m_dagObject["dag"].items())
    {
      //std::cout << "Checking x.key: " << (std::string)x.key() << '\n';
      for (auto& y : m_dagObject["dag"][x.key()].items())
      {
        //std::cout << "Checking y.key: " << (std::string)y.key() << '\n';
        //if (y.key() == m_nameUri)
        if (y.key() == rxedInterestName)
        {
          m_dagServTracker[(std::string)y.key()]["inputsRxed"][(std::string)x.key()] = 0;
          //std::cout << "x.key is " << x.key() << ", and y.key is " << y.key() << '\n';

          m_vectorOfServiceInputs.push_back(0);             // for now, just create vector entries for the inputs, so that if they arrive out of order, we can insert at any index location
        }
      }
    }
    //std::cout << "Forwarder dagServTracker data structure: " << std::setw(2) << m_dagServTracker << '\n';



    // generate all the interests for required inputs
    //for (auto& serviceInput : m_dagServTracker[(std::string)m_nameUri]["inputsRxed"].items())
    for (auto& serviceInput : m_dagServTracker[(std::string)rxedInterestName]["inputsRxed"].items())
    {
      if (serviceInput.value() == 0)
      {
        // generate the interest for this input
        std::string dagString = m_dagObject.dump();
        DagForwarderApp::SendInterest(serviceInput.key(), dagString);
      }
    }



    m_nameAndDigest = interest->getName();  // store the name with digest so that we can later generate the data packet with the same name/digest!
                                            // TODO6: this has issues - we cannot use the same service in more than one location in the DAG workflow!
                                            // for this, we would need to be able to store and retrieve unique interests and their digest (perhaps using a fully hierarchical name?)
                                    // right now, this application only has one m_nameAndDigest private variable, and thus can only "store" one service instance.
                                    // we could turn that variable into a list of ndn::Name variables, and add to the list for each instance of the service?
                                // we also have only one m_vectorOfServiceInputs[], so we would need a list of them, one for each version of the service.
                                // once a service is fullfilled, we should remove the list items for it to clean up.

    //TODO: for now, I just force the single m_nameUri to be the same as the received interest name.
    m_nameUri = rxedInterestName;


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
DagForwarderApp::OnData(std::shared_ptr<const ndn::Data> data)
{
  NS_LOG_DEBUG("Receiving Data packet for " << data->getName());

  //std::cout << "DATA received for name " << data->getName() << std::endl;

  //std::cout << "numRxedInputs = " << m_numRxedInputs << std::endl;
  //std::cout << "content = " << data->getContent() << std::endl;

  ndn::Name simpleName;
  simpleName = (data->getName()).getPrefix(-1); // remove the last component of the name (the parameter digest) so we have just the raw name, and convert to Uri string
  simpleName = simpleName.getSubName(1); // remove the first component of the name (/nesco)
  //std::string rxedDataName = (data->getName()).getPrefix(-1).toUri(); // remove the last component of the name (the parameter digest) so we have just the raw name
  std::string rxedDataName = simpleName.toUri();


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
  // just read the index from the dagObject JSON structure
  //TODO1: look at the current m_dagServTracker data structure to compare rxedDataName with inputsRxed, rather than using m_nameUri
  // above won't work, m_dagServTracker can have more than one entry (one per each version of the hosted service)
  //TODO2: instead, we should extract the "head" value from the parameter. BUT the data packets don't carry the parameter!
  //TODO3: I need another way to determine the service version that made this specific data request, since I need the input index!
  // perhaps keep a mapping of signature to requesting service, and use the received signature to determine which service version asked for it
  //TODO4: for now, this works with a single version of each service. We can not have the same service appear twice in the same DAG until we make the changes above.
  /*
  auto dagParameterFromData = data->getApplicationParameters();
  std::string dagString = std::string(reinterpret_cast<const char*>(dagParameterFromData.value()), dagParameterFromData.value_size());
  m_dagObject = json::parse(dagString);
  NS_LOG_DEBUG("Interest parameter head: " << dagObject["head"] << ", m_name attribute: " << m_name.ndn::Name::toUri());
  std::string head = dagObject["head"];
  */

  ndn::time::milliseconds data_freshnessPeriod = data->getFreshnessPeriod();
  if (data_freshnessPeriod < m_lowestFreshness) {
    m_lowestFreshness = data_freshnessPeriod;
  }

  char index = -1;
  for (auto& x : m_dagObject["dag"].items())
  {
    if (x.key() == rxedDataName)
    {
      for (auto& y : m_dagObject["dag"][x.key()].items())
      {
        if (y.key() == m_nameUri)
        //if (y.key() == head)
        {
          //std::cout << "  HIT, y.key(): " << y.key() << ", y.value(): " << y.value() << '\n';
          index = y.value().template get<char>();
        }
      }
    }
  }
  if (index < 0)
  {
    std::cout << "  ERROR!! index for m_vectorOfServiceInputs cannot be negative! Something went wrong!!" << '\n';
  }
  else
  {
    m_vectorOfServiceInputs[index] = (*pServiceInput);
  }

  // mark this input as having been received
  m_dagServTracker[m_nameUri]["inputsRxed"][rxedDataName] = 1;
  //m_dagServTracker[head]["inputsRxed"][rxedDataName] = 1;
  //std::cout << "Forwarder dagServTracker data structure: " << std::setw(2) << m_dagServTracker << '\n';

  // we have to check if we have received all necessary inputs for this instance of the hosted service!
  //      if so, run the service below and generate new data packet to go downstream.
  //      otherwise, just wait for the other inputs.
  unsigned char allInputsReceived = 1;
  for (auto& serviceInput : m_dagServTracker[m_nameUri]["inputsRxed"].items())
  //for (auto& serviceInput : m_dagServTracker[head]["inputsRxed"].items())
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
    //NS_LOG_DEBUG("Fake running service " << m_service);
    NS_LOG_DEBUG("Running service " << m_service);

    // run operation. First we need to figure out what service this is, so we know the operation. This screams to be a function pointer! For now just use if's

    // TODO7: we should use function pointers here, and have each service be a function defined in a separate file. Figure out how to deal with potentially different num of inputs.

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
    if (m_service.ndn::Name::toUri() == "/service5"){
      serviceOutput = (m_vectorOfServiceInputs[0])*2;
    }
    if (m_service.ndn::Name::toUri() == "/service6"){
      serviceOutput = (m_vectorOfServiceInputs[0])+1;
    }
    if (m_service.ndn::Name::toUri() == "/service7"){
      serviceOutput = (m_vectorOfServiceInputs[0])+7;
    }
    if (m_service.ndn::Name::toUri() == "/service8"){
      serviceOutput = (m_vectorOfServiceInputs[0])*1 + (m_vectorOfServiceInputs[1])*1;
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
      serviceOutput = (m_vectorOfServiceInputs[0])+1;
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
      serviceOutput = (m_vectorOfServiceInputs[0])+1;
    }
    if (m_service.ndn::Name::toUri() == "/serviceP21"){
      serviceOutput =
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
    if (m_service.ndn::Name::toUri().rfind("/serviceCPM-", 0) == 0) { // if name of string starts with /serviceCPM- (any of the 001-100 CPM services)
      serviceOutput = (m_vectorOfServiceInputs[0])+1;
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
    new_data->setFreshnessPeriod(ndn::time::milliseconds(m_lowestFreshness));

    //new_data->setContent(std::make_shared< ::ndn::Buffer>(1024));
    unsigned char myBuffer[1024];
    // write to the buffer
    myBuffer[0] = serviceOutput;
    new_data->setContent(myBuffer, 1024);

    ndn::StackHelper::getKeyChain().sign(*new_data);
    // Call trace (for logging purposes)
    m_transmittedDatas(new_data, this, m_face);
    m_appLink->onReceiveData(*new_data);


    // now that we have run the service (and sent the result data out - and caching it), we set inputs to "not received"
    // this is done so when cached results expire due to freshness, any new interests will trigger inputs to be fetched again, and the service will run again.
    allInputsReceived = 0;
    /*
    for (auto& serviceInput : m_dagServTracker[m_nameUri]["inputsRxed"].items())
    {
      serviceInput.value() = 0;
    }
    */
    m_dagServTracker.clear();



  }
  else
  {
    NS_LOG_DEBUG("    Even though we received data packet, we are still waiting for more inputs!");
  }
  
}

} // namespace ns3
