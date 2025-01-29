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

// cabeee-custom-app-producer.cpp

#include "cabeee-custom-app-producer.hpp"

#include "ns3/ptr.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "ns3/string.h"
#include "ns3/uinteger.h"

#include "ns3/ndnSIM/helper/ndn-stack-helper.hpp"
#include "ns3/ndnSIM/helper/ndn-fib-helper.hpp"

#include "ns3/random-variable-stream.h"

NS_LOG_COMPONENT_DEFINE("CustomAppProducer");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED(CustomAppProducer);

// register NS-3 type
TypeId
CustomAppProducer::GetTypeId()
{
  static TypeId tid = TypeId("CustomAppProducer")
    .SetParent<ndn::App>()
    .AddConstructor<CustomAppProducer>()
    .AddAttribute("FreshnessPeriod_ms", "Requested freshness period milliseconds", UintegerValue(60000),
                    MakeUintegerAccessor(&CustomAppProducer::m_freshnessPeriod), MakeUintegerChecker<uint32_t>())
    .AddAttribute("UniformFreshness", "Requested uniform distribution for freshness period", UintegerValue(0),
                    MakeUintegerAccessor(&CustomAppProducer::m_uniformFreshness), MakeUintegerChecker<uint32_t>())
    .AddAttribute("minFreshness_ms", "Requested freshness period milliseconds", UintegerValue(100),
                    MakeUintegerAccessor(&CustomAppProducer::m_freshnessMin), MakeUintegerChecker<uint32_t>())
    .AddAttribute("maxFreshness_ms", "Requested freshness period milliseconds", UintegerValue(1000),
                    MakeUintegerAccessor(&CustomAppProducer::m_freshnessMax), MakeUintegerChecker<uint32_t>())
    .AddAttribute("Prefix", "Requested prefix", StringValue("/dumb-interest"),
                    ndn::MakeNameAccessor(&CustomAppProducer::m_prefix), ndn::MakeNameChecker())
    .AddAttribute("Service", "Requested service", StringValue("dumb-service"),
                    ndn::MakeNameAccessor(&CustomAppProducer::m_service), ndn::MakeNameChecker());
  return tid;
}

CustomAppProducer::CustomAppProducer()
  : m_isRunning(false)
{
}

// Processing upon start of the application
void
CustomAppProducer::StartApplication()
{
  // initialize ndn::App
  ndn::App::StartApplication();
  m_isRunning = true;

  m_name = m_prefix.ndn::Name::toUri() + m_service.ndn::Name::toUri();

  // Add entry to FIB for `/prefix/sub`
  //ndn::FibHelper::AddRoute(GetNode(), "/prefix/sub", m_face, 0);
  ndn::FibHelper::AddRoute(GetNode(), m_name, m_face, 0);

  if (m_uniformFreshness == 1)
  {
    Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();
    m_freshnessPeriod = rand->GetValue(m_freshnessMin, m_freshnessMax);
  }

  // Schedule send of first interest
  //Simulator::Schedule(Seconds(1.0), &CustomAppProducer::SendInterest, this);
}

// Processing when application is stopped
void
CustomAppProducer::StopApplication()
{
  m_isRunning = false;
  // cleanup ndn::App
  ndn::App::StopApplication();
}

void
CustomAppProducer::SendInterest()
{
  /////////////////////////////////////
  // Sending one Interest packet out //
  /////////////////////////////////////

  // Create and configure ndn::Interest
  //auto interest = std::make_shared<ndn::Interest>("/prefix/sub");
  auto interest = std::make_shared<ndn::Interest>(m_name); // cabeee - it is now set externally from the scenario
  Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();
  interest->setNonce(rand->GetValue(0, std::numeric_limits<uint32_t>::max()));
  interest->setInterestLifetime(ndn::time::seconds(5));
  interest->setMustBeFresh(true);

  NS_LOG_DEBUG("Sending Interest packet for " << *interest);

  // Call trace (for logging purposes)
  m_transmittedInterests(interest, this, m_face);

  m_appLink->onReceiveInterest(*interest);
}

// Callback that will be called when Interest arrives
void
CustomAppProducer::OnInterest(std::shared_ptr<const ndn::Interest> interest)
{
  ndn::App::OnInterest(interest);

  NS_LOG_DEBUG("Received Interest packet for " << interest->getName());

  // Note that Interests send out by the app will not be sent back to the app !

  auto data = std::make_shared<ndn::Data>(interest->getName());
  if (m_uniformFreshness == 2)
  {
    Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();
    m_freshnessPeriod = rand->GetValue(m_freshnessMin, m_freshnessMax);
  }
  //data->setFreshnessPeriod(ndn::time::milliseconds(60000));
  data->setFreshnessPeriod(ndn::time::milliseconds(m_freshnessPeriod));

  //data->setContent(std::make_shared< ::ndn::Buffer>(1024));
  unsigned char myBuffer[1024];
  // write to the buffer
  myBuffer[0] = 5;
  data->setContent(myBuffer, 1024);

  ndn::StackHelper::getKeyChain().sign(*data);

  NS_LOG_DEBUG("Sending Data packet for " << data->getName());

  // Call trace (for logging purposes)
  m_transmittedDatas(data, this, m_face);

  m_appLink->onReceiveData(*data);
}

// Callback that will be called when Data arrives
void
CustomAppProducer::OnData(std::shared_ptr<const ndn::Data> data)
{
  NS_LOG_DEBUG("Receiving Data packet for " << data->getName());

  std::cout << "DATA received for name " << data->getName() << std::endl;
}

} // namespace ns3
