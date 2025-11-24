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

// cabeee-dag-serviceDiscovery-app.cpp

#include "cabeee-dag-serviceDiscovery-app.hpp"

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

NS_LOG_COMPONENT_DEFINE("DagServiceDiscoveryApp");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED(DagServiceDiscoveryApp);

// register NS-3 type
TypeId
DagServiceDiscoveryApp::GetTypeId()
{
  static TypeId tid = TypeId("DagServiceDiscoveryApp")
    .SetParent<ndn::App>()
    .AddConstructor<DagServiceDiscoveryApp>()
    .AddAttribute("Prefix", "Requested prefix", StringValue("/dumb-interest"),
                    ndn::MakeNameAccessor(&DagServiceDiscoveryApp::m_prefix), ndn::MakeNameChecker())
    .AddAttribute("Service", "Requested service", StringValue("dumb-service"),
                    ndn::MakeNameAccessor(&DagServiceDiscoveryApp::m_service), ndn::MakeNameChecker());
  return tid;
}

DagServiceDiscoveryApp::DagServiceDiscoveryApp()
  : m_isRunning(false)
{
}

// Processing upon start of the application
void
DagServiceDiscoveryApp::StartApplication()
{
  // initialize ndn::App
  ndn::App::StartApplication();
  m_isRunning = true;

  m_name = m_prefix.ndn::Name::toUri() + "/serviceDiscovery" + m_service.ndn::Name::toUri();
  
  NS_LOG_DEBUG("serviceDiscoveryAPP is performing AddRoute on name: " << m_name);

  // Add entry to FIB for `/prefix/sub`
  //ndn::FibHelper::AddRoute(GetNode(), "/prefix/sub", m_face, 0); //cabeee took this out, let the global router figure it out.
  ndn::FibHelper::AddRoute(GetNode(), m_name, m_face, 0);

  //ndn::FibHelper::AddRoute(GetNode(), "/nesco/shortcutOPT", m_face, 0);
  //NS_LOG_DEBUG("   cabeee application CABEEEshortcutOPT" << m_service << " is showing faceID: " << m_face->getId());
  //ndn::Face::setInterestFilter("/nesco/shortcutOPT", std::bind(&DagServiceDiscoveryApp::OnInterest, this, _2));
  //m_face->setInterestFilter("/nesco/shortcutOPT", &DagServiceDiscoveryApp::OnInterest);

  m_nameUri = m_service.ndn::Name::toUri();


  m_lowestFreshness = ndn::time::milliseconds(100000); // set to a high value (I know no producer freshness value is higher than 100 seconds)

}

// Processing when application is stopped
void
DagServiceDiscoveryApp::StopApplication()
{
  m_isRunning = false;
  // cleanup ndn::App
  ndn::App::StopApplication();
}





std::string
DagServiceDiscoveryApp::PruneDagWorkflow(const std::string& interestName, std::string dagString)
{

  auto dagObject = json::parse(dagString);

  // we PRUNE the DAG workflow to not include anything further downstream than this service

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
        //std::cout << "ServiceDiscovery prunning current sink node: " << sinkNode << '\n';
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

  std::string updatedDagString = dagObject.dump();

  return updatedDagString;


}



void
DagServiceDiscoveryApp::SendInterest(const std::string& interestName, std::string dagString)
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
  auto interest = std::make_shared<ndn::Interest>(m_prefix.ndn::Name::toUri() + "/serviceDiscovery" + interestName);
  Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();
  interest->setNonce(rand->GetValue(0, std::numeric_limits<uint32_t>::max()));
  interest->setInterestLifetime(ndn::time::seconds(10));
  interest->setMustBeFresh(true);

  // in order to convert from std::string to a char[] datatype we do the following (https://stackoverflow.com/questions/7352099/stdstring-to-char):
  char *dagStringParameter = new char[dagString.length() + 1];
  strcpy(dagStringParameter, dagString.c_str());
  size_t length = strlen(dagStringParameter);
  //add modified DAG workflow as a parameter to the new interest
  interest->setApplicationParameters((const uint8_t *)dagStringParameter, length);

  NS_LOG_DEBUG("ServiceDiscoveryAPP: Sending Interest packet for " << *interest);

  // Call trace (for logging purposes)
  m_transmittedInterests(interest, this, m_face);

  m_appLink->onReceiveInterest(*interest);
}







// Callback that will be called when Interest arrives
void
DagServiceDiscoveryApp::OnInterest(std::shared_ptr<const ndn::Interest> interest)
{
  ndn::App::OnInterest(interest);


  NS_LOG_DEBUG("ServiceDiscoveryAPP Received Interest packet for " << interest->getName());

  // decode the DAG string contained in the application parameters, so we can generate the new interest(s)
  //extract custom parameter from interest packet
  auto dagParameterFromInterest = interest->getApplicationParameters();
  std::string dagString = std::string(reinterpret_cast<const char*>(dagParameterFromInterest.value()), dagParameterFromInterest.value_size());


  // read the dag parameters and figure out which interests to generate next. Change the dagParameters accordingly (head will be different)
  json dagObject = json::parse(dagString);


  //NS_LOG_DEBUG("\n\nServiceDiscoveryAPP - Full DAG as received: " << std::setw(2) << dagObject << '\n');


  // create the tracking data structure using JSON
  ndn::Name simpleName;
  ndn::Name simpleNameAndHash;
  ndn::Name fullNameAndHash;

  simpleName        = (interest->getName()).getPrefix(-1); // remove the last component of the name (the parameter digest) so we have just the raw name
  simpleNameAndHash = interest->getName();
  fullNameAndHash   = interest->getName();

  simpleName        = simpleName.getSubName(2,1); // remove the zeroeth component of the name (/nesco), and the first component of the name (/serviceDiscovery). starting at component 2, keep 1 component
  simpleNameAndHash = simpleNameAndHash.getSubName(2,simpleNameAndHash.size()); // remove the zeroeth component of the name (/nesco), and the first component of the name (/serviceDiscovery). starting at component 2, keep the rest of the components, including the application parameter hash
  fullNameAndHash   = fullNameAndHash.getSubName(0,fullNameAndHash.size()); // remove the zeroeth component of the name (/nesco), and the first component of the name (/serviceDiscovery). starting at component 0, keep the rest of the components, including the application parameter hash

  std::string rxedInterestName        = simpleName.toUri();
  std::string rxedInterestNameAndHash = simpleNameAndHash.toUri();
  std::string rxedInterestFullNameAndHash = fullNameAndHash.toUri();

  //NS_LOG_DEBUG("ServiceDiscoveryAPP rxedInterestName -> simpleName: " << rxedInterestName << '\n');
  //NS_LOG_DEBUG("ServiceDiscoveryAPP rxedInterestNameAndHash: " << rxedInterestNameAndHash << '\n');
  //NS_LOG_DEBUG("ServiceDiscoveryAPP rxedInterestFullNameAndHash: " << rxedInterestFullNameAndHash << '\n');


  // We add prevHash to dagObject so that generated interests are unique
  dagObject["prevHash"] = rxedInterestNameAndHash; // to keep the historical path of where interests have been, so they are all unique!



  // generate interests for inputs into hosted services early (shortcut optimization to parallelize workflow)
  if (rxedInterestName == "/shortcutOPT")
  {
    if (m_service.toUri() != dagObject["head"])
    {
      // only if we haven't already received a request for the service
      if (m_dagServTracker.empty()) // if we haven't generated an interest for this service, the dagServTracker will be empty
      {
        //NS_LOG_DEBUG("\n\nshortcutOPT: We are hosting service " << m_service.toUri() << ", looking for this service in the DAG!" << std::endl);
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
              //std::cout << "ServiceDiscovery dagServTracker data structure before: " << std::setw(2) << m_dagServTracker << '\n';
              m_dagServTracker[(std::string)y.key()]["inputsRxed"][(std::string)x.key()] = 0;
              //std::cout << "ServiceDiscovery dagServTracker data structure after: " << std::setw(2) << m_dagServTracker << '\n';
              //std::cout << "x.key is " << x.key() << ", and y.key is " << y.key() << '\n';

              //m_vectorOfServiceInputs.push_back(0);             // for now, just create vector entries for the inputs, so that if they arrive out of order, we can insert at any index location
            }
          }
        }
        //std::cout << "ServiceDiscovery dagServTracker data structure: " << std::setw(2) << m_dagServTracker << '\n';
        //NS_LOG_DEBUG("\n\nshortcutOPT: Generarating all interests for required inputs..." << '\n');
        // generate all the interests for required inputs
        //for (auto& serviceInput : m_dagServTracker[(std::string)m_nameUri]["inputsRxed"].items())
        for (auto& serviceInput : m_dagServTracker[(std::string)m_service.toUri()]["inputsRxed"].items())
        {
          if (serviceInput.value() == 0)
          {
            // generate the interest for this input, sendInterest will prune the DAG and set the head properly
            std::string dagString = dagObject.dump();
            //NS_LOG_DEBUG("\n\nshortcutOPT: Generating interest for " << serviceInput.key() << '\n');
            DagServiceDiscoveryApp::SendInterest(serviceInput.key(), dagString);
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
  }



  else // not dealing with a shortcut optimization interest
  {

    // if there are no inputs to this service (root service), or if we are hosting results for this pDAG, then generate data packet containing calculated EFT and current (tx) timestamp.
    //if (rxedInterestName == "/sensor") // don't hard-code this to just be /sensor. It needs to do this for ANY ROOT NODE!
    //if ((std::find(listOfRootServices.begin(), listOfRootServices.end(), rxedInterestName) != listOfRootServices.end())) // if rxedInterestName exists in listOfRootServices
    if (dagObject["dag"].empty()) // ROOT NODE!
    {
      NS_LOG_DEBUG("ServiceDiscoveryAPP Interest received for root service: " << rxedInterestName << ", generating data packet!");
      m_nameAndDigest = interest->getName();   // store the name with digest so that we can later generate the data packet with the same name/digest!
      auto new_data = std::make_shared<ndn::Data>(m_nameAndDigest);
      new_data->setFreshnessPeriod(ndn::time::milliseconds(m_lowestFreshness));

      //new_data->setContent(std::make_shared< ::ndn::Buffer>(1024));
      unsigned char myBuffer[1024];
      json dataPacketContents;
      Time timeNow;
      timeNow = Simulator::Now();
      // Convert to integer in milliseconds and then to string
      int64_t timeNowNS = timeNow.ToInteger(ns3::Time::NS);
      std::string timeStringNS = std::to_string(timeNowNS);
      dataPacketContents["EFT"] = timeNowNS;
      dataPacketContents["txTime"] = timeNowNS;
      dataPacketContents["serviceLatency"] = 0; //TODO: for now, I'm just assuming all root node services take 0ms to run (keep track of EFT in nanoseconds for granularity), but this will need to come from somewhere based on service and node
      //NS_LOG_DEBUG("timeStringNS = " << timeStringNS);

      std::string dataPacketString = dataPacketContents.dump();

      // instead of just writing a single value to the buffer, now we write the JSON data structure containing EFT and tx timestamp
      // write to the buffer, after making sure it's big enough
      if (strlen(dataPacketString.c_str())+1 > 1024) // string length plus NULL terminating character
      {
        NS_LOG_DEBUG("SD APP ERROR!! The data packet size is larger than 1024!!!");
      }
      //else
      //{
        //NS_LOG_DEBUG("The data packet size using strlen+1 is " << strlen(dataPacketString.c_str())+1);
        //NS_LOG_DEBUG("The data packet size using length+1 operator is " << dataPacketString.length()+1);
      //}
      memcpy(myBuffer, dataPacketString.c_str(), strlen(dataPacketString.c_str())+1);
      //new_data->setContent(myBuffer, 1024); // make the data always 1024 bytes long
      new_data->setContent(myBuffer, strlen(dataPacketString.c_str())+1); // make the data just big enough to fit the json object

      ndn::StackHelper::getKeyChain().sign(*new_data);
      NS_LOG_DEBUG("ServiceDiscoveryAPP Sending Data packet for " << new_data->getName());
      // Call trace (for logging purposes)
      m_transmittedDatas(new_data, this, m_face);
      m_appLink->onReceiveData(*new_data);
    }
    else
    {


      if (m_service.toUri() != dagObject["head"])
      {
        // Here we would determine which interfaces can be used to reach the named service, and generate a new interest for the name service out of each of the faces.
        // we would also keep track of which ones have left, so that when data packets arrive, I can evaluate their EFTs. Once all have returned, generate data packet with lowest EFT.
        // BUT ACTUALLY, WE CAN'T DO THIS HERE. WE NEED TO DO THIS IN THE NFD FORWARDER! This app will not be receiving these interests.
        // THIS SHOULD NOT HAPPEN, REPORT AN ERROR. This APP should not receive an interest for a service that it's not hosting.
        NS_LOG_DEBUG("SD APP ERROR!! This APP should not receive an interest for a service that it's not hosting. Interest name: " << interest->getName());
      }
      else
      {
        // we are hosting this service, so look at required inputs, and generate those interests using the pDAG but keep the original interest absolute start time.

        // create data structure to keep track of which inputs have arrived. For now, we just create it. We mark them as received in "onData() below".
        for (auto& x : dagObject["dag"].items())
        {
          //std::cout << "Checking x.key: " << (std::string)x.key() << '\n';
          for (auto& y : dagObject["dag"][x.key()].items())
          {
            //std::cout << "Checking y.key: " << (std::string)y.key() << '\n';
            //if (y.key() == m_nameUri)
            if (y.key() == rxedInterestName)
            {
              std::string updatedDagString = dagObject.dump();

              // prune the workflow here, so that the hash we create is the real one that we use when sending the interest out. Prunning also changes the head.
              std::string simpleServiceName = x.key();
              updatedDagString = DagServiceDiscoveryApp::PruneDagWorkflow(simpleServiceName, updatedDagString);

              // Create interest with simplename x.key(), then add application parameters, and use the new name&hash for the data structure inputsRxed item.
              // in order to convert from std::string to a char[] datatype we do the following (https://stackoverflow.com/questions/7352099/stdstring-to-char):
              char *dagStringParameter = new char[updatedDagString.length() + 1];
              strcpy(dagStringParameter, updatedDagString.c_str());
              size_t length = strlen(dagStringParameter);
              //add modified object as a parameter to the new interest
              auto new_interest = std::make_shared<ndn::Interest>(m_prefix.ndn::Name::toUri() + "/serviceDiscovery" + (std::string)x.key());
              //Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();
              //new_interest->setNonce(rand->GetValue(0, std::numeric_limits<uint32_t>::max()));
              //new_interest->setInterestLifetime(ndn::time::seconds(10));
              //new_interest->setMustBeFresh(true);

              new_interest->setApplicationParameters((const uint8_t *)dagStringParameter, length);
              //std::string serviceInputNameAndHash = new_interest->getName().getSubName(0,new_interest->getName().size()).toUri(); // starting at name element 0, get enough name elements to get us to the end of the name (get the full name)
              std::string serviceInputNameAndHash = new_interest->getName().getSubName(2,new_interest->getName().size()).toUri(); // starting at name element 2, get enough name elements to get us to the end of the name (get the full name)
              //NFD_LOG_DEBUG("NFDServiceDiscovery created serviceInputNameAndHash: " << serviceInputNameAndHash);

              m_dagServTracker[rxedInterestFullNameAndHash]["inputsRxed"][serviceInputNameAndHash] = 0; // initialize to 0, meaning this input has not been received yet.
              m_dagServTracker[rxedInterestFullNameAndHash]["EFT"][serviceInputNameAndHash] = -1; // initialize to -1, meaning it is an invalid EFT value that hasn't been calculated yet.

              // generate the interest for this input
              //DagServiceDiscoveryApp::SendInterest(serviceInputNameAndHash, updatedDagString);
              NS_LOG_DEBUG("ServiceDiscoveryAPP: Sending Interest packet for " << *new_interest);
              // Call trace (for logging purposes)
              m_transmittedInterests(new_interest, this, m_face);
              m_appLink->onReceiveInterest(*new_interest);

              //std::cout << "x.key is " << x.key() << ", and y.key is " << y.key() << '\n';

            }
          }
        }
        //std::cout << "ServiceDiscovery dagServTracker data structure: " << std::setw(2) << m_dagServTracker << '\n';
        //NS_LOG_DEBUG("\n\nServiceDiscoveryAPP dagServTracker data structure (on Interest): " << std::setw(2) << m_dagServTracker << '\n');

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
DagServiceDiscoveryApp::OnData(std::shared_ptr<const ndn::Data> data)
{
  NS_LOG_DEBUG("Receiving Data packet for " << data->getName());


  //std::cout << "numRxedInputs = " << m_numRxedInputs << std::endl;
  //std::cout << "content = " << data->getContent() << std::endl;


  ndn::Name simpleName;
  ndn::Name simpleNameAndHash;
  simpleName        = (data->getName()).getPrefix(-1); // remove the last component of the name (the parameter digest) so we have just the raw name
  simpleNameAndHash = data->getName();
  simpleName        = simpleName.getSubName(2,1); // remove the zeroeth component of the name (/nesco), and the first component of the name (/serviceDiscovery). starting at component 2, keep 1 component
  simpleNameAndHash = simpleNameAndHash.getSubName(2,simpleNameAndHash.size()); // remove the zeroeth component of the name (/nesco), and the first component of the name (/serviceDiscovery). starting at component 2, keep the rest of the components, including the application parameter hash
  //std::string rxedDataName      = (interest->getName()).getPrefix(-1).toUri(); // remove the last component of the name (the parameter digest) so we have just the raw name, and convert to Uri string
  std::string rxedDataName        = simpleName.toUri();
  std::string rxedDataNameAndHash = simpleNameAndHash.toUri();



  //NS_LOG_DEBUG("Now reading it into string...");

  std::string dataPacketString;
  dataPacketString = (const char *)data->getContent().value();
  //NS_LOG_DEBUG("Data string received: " << dataPacketString);


  //NS_LOG_DEBUG("Now parsing it into JSON...");
  json dataPacketContents = json::parse(dataPacketString);
  NS_LOG_DEBUG("Data received - EFT: " << dataPacketContents["EFT"] << ", txTime: " << dataPacketContents["txTime"]);


  int64_t dataTxTime = dataPacketContents["txTime"];

  ns3::Time timeNow;
  timeNow = ns3::Simulator::Now();
  ns3::Time timeTx;
  timeTx = ns3::Time::FromInteger(dataTxTime, ns3::Time::NS);
  ns3::Time linkDelay;
  linkDelay = timeNow - timeTx;
  //NS_LOG_DEBUG("Calculated link delay is: time.now " << timeNow.ToInteger(ns3::Time::NS) << " - timeTx " << timeTx.ToInteger(ns3::Time::NS) << " = " << linkDelay.ToInteger(ns3::Time::NS) << "ns");


  int64_t dataEFT = dataPacketContents["EFT"];

  ns3::Time eft;
  eft = ns3::Time::FromInteger(dataEFT, ns3::Time::NS);
  ns3::Time newEFT;
  newEFT = eft + linkDelay;
  //NS_LOG_DEBUG("Calculated EFT out of this face is: EFT " << eft.ToInteger(ns3::Time::NS) << " + upstreamLinkDelay " << linkDelay.ToInteger(ns3::Time::NS) << " = " << newEFT.ToInteger(ns3::Time::NS) << "ns");

  // go through data structure elements, looking at the names in "inputsRxed" to see which one matches rxedDataNameAndHash, and then set serviceNameAndHash to that name
  std::string serviceNameAndHash;
  for (auto& serviceIterator : m_dagServTracker.items())
  {
    for (auto& inputIterator : m_dagServTracker[serviceIterator.key()]["inputsRxed"].items())
    {
      if (inputIterator.key() == rxedDataNameAndHash)
      {
        serviceNameAndHash = serviceIterator.key();
        //NS_LOG_DEBUG("Match for " << rxedDataNameAndHash << " found: " << serviceIterator.key());
      }
    }
  }
  //NS_LOG_DEBUG("ServiceNameAndHash (key) is: " << serviceNameAndHash);

  // Mark this input as received
  //m_dagServTracker[m_nameUri]["inputsRxed"][rxedDataName] = 1;
  m_dagServTracker[serviceNameAndHash]["inputsRxed"][rxedDataNameAndHash] = 1;


  // Convert Time to integer in milliseconds and then to string
  int64_t linkDelayNS = linkDelay.ToInteger(ns3::Time::NS);
  std::string linkDelayStringNS = std::to_string(linkDelayNS);
  int64_t eftNS = newEFT.ToInteger(ns3::Time::NS);
  std::string eftStringNS = std::to_string(eftNS);


  m_dagServTracker[serviceNameAndHash]["EFT"][rxedDataNameAndHash] = eftNS;


  //NS_LOG_DEBUG("\n\nServiceDiscoveryAPP dagServTracker data structure (on Data): " << std::setw(2) << m_dagServTracker << '\n');


  // check if all inputs have been received
  int allRxed = 1;
  int64_t highestEFT = -1;  // initialize to invalid EFT
  //for (auto& serviceInput : m_dagServTracker[m_nameUri]["inputsRxed"].items())
  for (auto& serviceInput : m_dagServTracker[serviceNameAndHash]["inputsRxed"].items())
  {
      //if (m_dagServTracker[m_nameUri]["inputsRxed"][serviceInput.key()] != 1)
      if (m_dagServTracker[serviceNameAndHash]["inputsRxed"][serviceInput.key()] != 1)
      {
        allRxed = 0;
      }

      // figure out which is the highest EFT of all the input services that we've received packets for so far
      //int64_t thisEFT = m_dagServTracker[m_nameUri]["EFT"][serviceInput.key()];
      int64_t thisEFT = m_dagServTracker[serviceNameAndHash]["EFT"][serviceInput.key()];
      if (highestEFT == -1)
      {
        highestEFT = thisEFT; // initialize to the first one
      }
      if (thisEFT != -1 && thisEFT > highestEFT)
      {
        highestEFT = thisEFT; // this becomes the highestEFT EFT found so far
      }
  }
  // if all inputs have been received, then calculate largest EFT of all inputs, and generate data packet downstream
  if (allRxed == 1)
  {

    // generate new data packet downstream with the calculated EFT and TX timestamp

    //NS_LOG_DEBUG("creating data for name: " << serviceNameAndHash);

    auto new_data = std::make_shared<ndn::Data>(serviceNameAndHash);
    new_data->setFreshnessPeriod(data->getFreshnessPeriod());

    unsigned char myBuffer[1024];
    //json dataPacketContents;
    //ns3::Time timeNow;
    timeNow = ns3::Simulator::Now();
    // Convert to integer in milliseconds and then to string
    int64_t timeNowNS = timeNow.ToInteger(ns3::Time::NS);
    std::string timeStringNS = std::to_string(timeNowNS);
    dataPacketContents.clear();
    dataPacketContents["txTime"] = timeNowNS;
    dataPacketContents["EFT"] = highestEFT;
    dataPacketContents["serviceLatency"] = 1000000; //TODO: for now, I'm just assuming all services take 1ms to run (keep track of EFT in nanoseconds for granularity), but this will need to come from somewhere based on service and node
    NS_LOG_DEBUG("ServiceDiscoveryAPP - Sending Data packet with JSON data packet contents: " << dataPacketContents);

    //std::string dataPacketString;
    dataPacketString = dataPacketContents.dump();

    // instead of just writing a single value to the buffer, now we write the JSON data structure containing EFT and tx timestamp
    // write to the buffer, after making sure it's big enough
    if (strlen(dataPacketString.c_str())+1 > 1024) // string length plus NULL terminating character
    {
      NS_LOG_DEBUG("SD APP ERROR!! The data packet size is larger than 1024!!!");
    }
    //else
    //{
      //NS_LOG_DEBUG("The data packet size using strlen+1 is " << strlen(dataPacketString.c_str())+1);
      //NS_LOG_DEBUG("The data packet size using length+1 operator is " << dataPacketString.length()+1);
    //}
    memcpy(myBuffer, dataPacketString.c_str(), strlen(dataPacketString.c_str())+1);
    //new_data->setContent(myBuffer, 1024); // make the data always 1024 bytes long
    new_data->setContent(myBuffer, strlen(dataPacketString.c_str())+1); // make the data just big enough to fit the json object

    NS_LOG_DEBUG("ServiceDiscoveryAPP - Sending Data packet for " << new_data->getName());


    ndn::StackHelper::getKeyChain().sign(*new_data);
    // Call trace (for logging purposes)
    m_transmittedDatas(new_data, this, m_face);
    m_appLink->onReceiveData(*new_data);



    //m_dagServTracker.clear();
    m_dagServTracker.erase(serviceNameAndHash);
    m_lowestFreshness = ndn::time::milliseconds(100000); // set to a high value (I know no producer freshness value is higher than 100 seconds)


  }
  else
  {
    NS_LOG_DEBUG("    Even though we received data packet, we are still waiting for more inputs!");
  }

  
}

} // namespace ns3
