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

#ifndef NDNSIM_CABEEE_DAG_FORWARDER_APP_OPT_HPP
#define NDNSIM_CABEEE_DAG_FORWARDER_APP_OPT_HPP

#include <ndn-cxx/face.hpp>
#include <ndn-cxx/interest.hpp>
#include <ndn-cxx/security/key-chain.hpp>
#include <ndn-cxx/util/scheduler.hpp>
#include <ndn-cxx/security/signing-helpers.hpp>

#include <iostream>
#include <ndn-cxx/util/io.hpp>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

using namespace ndn;
namespace app {

class DagForwarderAppOpt
{
public:
  DagForwarderAppOpt(ndn::KeyChain& keyChain)
    : m_keyChain(keyChain)
  {
    // register prefix and set interest filter on producer face
    //m_faceProducer.setInterestFilter("/hello", std::bind(&DagForwarderAppOpt::respondToAnyInterest, this, _2),
                                     //std::bind([]{}), std::bind([]{}));

    // use scheduler to send interest later on consumer face
    //m_scheduler.schedule(ndn::time::seconds(2), [this] {
        //m_faceConsumer.expressInterest(ndn::Interest("/hello/world"),
                                       //std::bind([] { std::cout << "Hello!" << std::endl; }),
                                       //std::bind([] { std::cout << "NACK!" << std::endl; }),
                                       //std::bind([] { std::cout << "Bye!.." << std::endl; }));
      //});
  }

  void
  run(std::string PREFIX, std::string servicePrefix)
  {
    m_PREFIX = PREFIX;
    std::string fullPrefix(PREFIX);
    fullPrefix.append(servicePrefix);
    m_service = servicePrefix;
    std::cout << "Forwarder listening to: " << fullPrefix << '\n';
    m_face.setInterestFilter(fullPrefix,
                             std::bind(&DagForwarderAppOpt::onInterest, this, _2),
                             nullptr, // RegisterPrefixSuccessCallback is optional
                             std::bind(&DagForwarderAppOpt::onRegisterFailed, this, _1, _2));
    m_face.setInterestFilter("/interCACHE/shortcutOPT", //TODO: don't hardcode the interCACHE prefix
                             std::bind(&DagForwarderAppOpt::onInterest, this, _2),
                             nullptr, // RegisterPrefixSuccessCallback is optional
                             std::bind(&DagForwarderAppOpt::onRegisterFailed, this, _1, _2));

    auto cert = m_keyChain.getPib().getDefaultIdentity().getDefaultKey().getDefaultCertificate();
    m_certServeHandle = m_face.setInterestFilter(security::extractIdentityFromCertName(cert.getName()),
                                                 [this, cert] (auto&&...) {
                                                   m_face.put(cert);
                                                 },
                                                 std::bind(&DagForwarderAppOpt::onRegisterFailed, this, _1, _2));

    m_nameUri = m_service.ndn::Name::toUri();
    m_face.processEvents();
  }


/*
private:
  void
  respondToAnyInterest(const ndn::Interest& interest)
  {
    auto data = std::make_shared<ndn::Data>(interest.getName());
    m_keyChain.sign(*data);
    m_faceProducer.put(*data);
  }
*/


private:
  void
  sendInterest(const std::string& interestName, std::string dagString)
  {

    /////////////////////////////////////
    // Sending one Interest packet out //
    /////////////////////////////////////

    //std::cout << "Should send new interest now" << std::endl;



    Interest interest(m_PREFIX + interestName);
    interest.setMustBeFresh(true); // forcing fresh data allows us to run "non-caching" scenarios back-to-back (waiting for data packet lifetime to expire), and we won't be utilizing cached packets from a previous run.
    interest.setInterestLifetime(6_s); // The default is 4 seconds


    // overwrite the dag parameter "head" value and generate new application parameters (thus calculating new sha256 digest)
    //std::string dagString = std::string(reinterpret_cast<const char*>(dagParameter.value()), dagParameter.value_size());
    auto dagObject = json::parse(dagString);

    //std::cout << "\n\n\n\n\n\n\n\n\n\n\n\nStarting new SendInterest for " << interestName << '\n';
    //std::cout << "Full DAG before pruning: " << std::setw(2) << dagObject << '\n';

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
    //std::cout << "Sending Interest with Pruned DAG with new head name: " << std::setw(2) << dagObject << '\n';
    std::string updatedDagString = dagObject.dump();
    //std::cout << "updatedDagString: " << updatedDagString << std::endl;

    std::shared_ptr<Buffer> dagApplicationParameters;
    std::istringstream is(updatedDagString);
    dagApplicationParameters = io::loadBuffer(is, io::NO_ENCODING);

    //add modified DAG workflow as a parameter to the new interest
    interest.setApplicationParameters(dagApplicationParameters);



    //std::cout << "Forwarder: Sending Interest packet for " << interest << std::endl;

    m_face.expressInterest(interest,
                           std::bind(&DagForwarderAppOpt::onData, this,  _1, _2),
                           std::bind(&DagForwarderAppOpt::onNack, this, _1, _2),
                           std::bind(&DagForwarderAppOpt::onTimeout, this, _1));

    // we need to prevent this from blocking! If more than one interest will be generated, we want both to be sent out at the same time!
    m_face.processEvents(ndn::time::milliseconds(-1)); // If a negative timeout is specified, then processEvents will not block and will process only pending events.



  }







private:
  void
  onInterest(const Interest& interest)
  {
    //std::cout << ">> I: " << interest << std::endl;

    // decode the DAG string contained in the application parameters, so we can generate the new interest(s)
    //extract custom parameter from interest packet

    //std::cout << "Forwarder InterestName: " << interest.getName() << '\n';



    auto dagParameterFromInterest = interest.getApplicationParameters();
    std::string dagString = std::string(reinterpret_cast<const char*>(dagParameterFromInterest.value()), dagParameterFromInterest.value_size());

    // read the dag parameters and figure out which interests to generate next. Change the dagParameters accordingly (head will be different)
    json dagObject = json::parse(dagString);
    //std::cout << "Full DAG as received: " << std::setw(2) << dagObject << std::endl;


    // create the tracking data structure using JSON
    json nullJson;
    ndn::Name simpleName;
    simpleName = (interest.getName()).getPrefix(-1); // remove the last component of the name (the parameter digest) so we have just the raw name, and convert to Uri string
    simpleName = simpleName.getSubName(1); // remove the first component of the name (/interCACHE)
    std::string rxedInterestName = simpleName.toUri();
    //std::cout << "Forwarder rxedInterestName: " << rxedInterestName << std::endl;


    if (m_dagObject.empty() && m_service.toUri() == dagObject["head"])
    {
      // store the m_dagObject that we will use for regular forwarding (not for shortcut optimization)
      m_dagObject = json::parse(dagString);
    }



    //std::cout << "Forwarder dagObject[head] = " << dagObject["head"] << std::endl;
    // generate interests for inputs into hosted services early (shortcut optimization to parallelize workflow)
    if (rxedInterestName == "/shortcutOPT")
    {
      if (m_service.toUri() != dagObject["head"])
      {
        // only if we haven't already received a request for the service
        if (m_dagServTracker.empty()) // if we haven't generated an interest for this service, the dagServTracker will be empty
        {
          //std::cout << " we are hosting service " << m_service.toUri() << ", looking for this service in the DAG!" << std::endl;
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

          //std::cout << "Generarating all interests for required inputs..." << '\n';
          // generate all the interests for required inputs
          //for (auto& serviceInput : m_dagServTracker[(std::string)m_nameUri]["inputsRxed"].items())
          for (auto& serviceInput : m_dagServTracker[(std::string)m_service.toUri()]["inputsRxed"].items())
          {
            if (serviceInput.value() == 0)
            {
              // generate the interest for this input, sendInterest will prune the DAG and set the head properly
              std::string dagString = dagObject.dump();
              //std::cout << "Forwarder: Generating interest for " << serviceInput.key() << '\n';
              sendInterest(serviceInput.key(), dagString);
            }
          }

          m_nameAndDigest = interest.getName();   // store the name with digest so that we can later generate the data packet with the same name/digest!
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


      //std::cout << "Generarating all interests for required inputs..." << '\n';
      // generate all the interests for required inputs
      //for (auto& serviceInput : m_dagServTracker[(std::string)m_nameUri]["inputsRxed"].items())
      for (auto& serviceInput : m_dagServTracker[(std::string)rxedInterestName]["inputsRxed"].items())
      {
        if (serviceInput.value() == 0)
        {
          // generate the interest for this input
          std::string dagString = m_dagObject.dump();
          //std::cout << "Forwarder: Generating interest for " << serviceInput.key() << '\n';
          sendInterest(serviceInput.key(), dagString);
        }
      }



      m_nameAndDigest = interest.getName();   // store the name with digest so that we can later generate the data packet with the same name/digest!
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





private:
  void
  onData(const Interest&, const Data& data)
  {
    //std::cout << "Received Data: " << data << std::endl;
    //std::cout << "Data Content: " << data.getContent().value() << std::endl;



    ndn::Block myRxedBlock = data.getContent();
    uint8_t *pContent = (uint8_t *)(myRxedBlock.data()); // this points to the first byte, which is the TLV-TYPE (21 for data packet contet)
    pContent++;  // now this points to the second byte, containing 253 (0xFD), meaning size (1024) is expressed with 2 octets
    pContent++;  // now this points to the first size octet
    pContent++;  // now this points to the second size octet
    pContent++;  // now we are pointing at the first byte of the true content
    //std::cout << "\n  The received data value is: " <<  (int)(*pContent) << std::endl << "\n\n";
    /*
    m_validator.validate(data,
                         [] (const Data&) {
                           std::cout << "Data conforms to trust schema" << std::endl;
                         },
                         [] (const Data&, const security::ValidationError& error) {
                           std::cout << "Error authenticating data: " << error << std::endl;
                         });
    */



    ndn::Name simpleName;
    simpleName = (data.getName()).getPrefix(-1); // remove the last component of the name (the parameter digest) so we have just the raw name, and convert to Uri string
    simpleName = simpleName.getSubName(1); // remove the first component of the name (/interCACHE)
    //std::string rxedDataName = (data.getName()).getPrefix(-1).toUri(); // remove the last component of the name (the parameter digest) so we have just the raw name
    std::string rxedDataName = simpleName.toUri();


    // TODO: this is a HACK. I need a better way to get to the first byte of the content. Right now, I'm just incrementing the pointer past the TLV type, and size.
    // and then getting to the first byte (which is all I'm using for data)
    unsigned char serviceOutput = 0;
    uint8_t *pServiceInput = 0;
    //pServiceInput = (uint8_t *)(m_mapOfRxedBlocks.back().data());
    pServiceInput = (uint8_t *)(data.getContent().data()); // this points to the first byte, which is the TLV-TYPE (21 for data packet content)
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


    //std::cout << " data received - looking for index number. m_dagObject is currently: " << std::setw(2) << m_dagObject << std::endl;
    //std::cout << " data received - rxedDataName: " << rxedDataName << std::endl;
    //std::cout << " data received - m_nameUri: " << m_nameUri << std::endl;
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
      //std::cout << ("Fake running service " << m_service);
      //std::cout << "Running service " << m_service << std::endl;

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
    
      //std::cout << "Service " << m_service.ndn::Name::toUri() << " has output: " << (int)serviceOutput << std::endl;
  
      // this following line is for linear workflows only!
      //now add the service name in front of the data name
      //std::string new_name = m_service.ndn::Name::toUri() + data->getName().ndn::Name::toUri();
      //std::cout << ("Creating data for new name: " << new_name);
      // for dag workflows, FOR NOW we just generate the data packet with the name of the service we ran. We don't support repeated services yet. For that we need higharchical names/results such as "/S2/S1/sensor" for example

      //std::cout << "Creating data for name: " << m_nameAndDigest << std::endl;  // m_name doesn't have the sha256 digest, so it doesn't match the original interest!
                                                                    // We use m_nameAndDigest to store the old name with the digest.


      // Create new Data packet
      auto new_data = std::make_shared<Data>();
      new_data->setName(m_nameAndDigest);
      new_data->setFreshnessPeriod(9_s);

      unsigned char myBuffer[1024];
      // write to the buffer
      myBuffer[0] = serviceOutput;
      new_data->setContent(myBuffer);


      // In order for the consumer application to be able to validate the packet, you need to setup
      // the following keys:
      // 1. Generate example trust anchor
      //
      //         ndnsec key-gen /example
      //         ndnsec cert-dump -i /example > example-trust-anchor.cert
      //
      // 2. Create a key for the producer and sign it with the example trust anchor
      //
      //         ndnsec key-gen /example/testApp
      //         ndnsec sign-req /example/testApp | ndnsec cert-gen -s /example -i example | ndnsec cert-install -

      // Sign Data packet with default identity
      //m_keyChain.sign(*new_data);
      // m_keyChain.sign(*new_data, signingByIdentity(<identityName>));
      // m_keyChain.sign(*new_data, signingByKey(<keyName>));
      // m_keyChain.sign(*new_data, signingByCertificate(<certName>));
      m_keyChain.sign(*new_data, signingWithSha256());

      // Return Data packet to the requester
      //std::cout << "<< D: " << *new_data << std::endl;
      m_face.put(*new_data);

    }
    //else
    //{
      //std::cout << "    Even though we received data packet, we are still waiting for more inputs!" << std::endl;
    //}

  
  }



  void
  onNack(const Interest&, const lp::Nack& nack) const
  {
    std::cout << "Received Nack with reason " << nack.getReason() << std::endl;
  }

  void
  onTimeout(const Interest& interest) const
  {
    std::cout << "Timeout for " << interest << std::endl;
  }


  void
  onRegisterFailed(const Name& prefix, const std::string& reason)
  {
    std::cerr << "ERROR: Failed to register prefix '" << prefix
              << "' with the local forwarder (" << reason << ")\n";
    m_face.shutdown();
  }


private:
  ndn::KeyChain& m_keyChain;
  ScopedRegisteredPrefixHandle m_certServeHandle;
  ndn::Face m_face;
  std::string m_PREFIX;
  ndn::Name m_service;
  std::string m_nameUri;
  ndn::Name m_nameAndDigest;
  json m_dagServTracker; // with this data structure, we can keep track of WHICH inputs have arrived, rather than just the NUMBER of inputs. (in case one inputs arrives multiple times)
  json m_dagObject;
  std::vector <unsigned char> m_vectorOfServiceInputs;
};

} // namespace app

#endif // NDNSIM_CABEEE_DAG_FORWARDER_APP_OPT_HPP