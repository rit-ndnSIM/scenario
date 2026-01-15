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

// cabeee-custom-app-csUpdater.cpp

#include "cabeee-custom-app-csUpdater.hpp"

#include "ns3/ptr.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "ns3/string.h"
#include "ns3/uinteger.h"

#include "ns3/ndnSIM/helper/ndn-stack-helper.hpp"
#include "ns3/ndnSIM/helper/ndn-fib-helper.hpp"

#include "ns3/random-variable-stream.h"

NS_LOG_COMPONENT_DEFINE("CustomAppCsUpdater");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED(CustomAppCsUpdater);

// register NS-3 type
TypeId
CustomAppCsUpdater::GetTypeId()
{
  static TypeId tid = TypeId("CustomAppCsUpdater")
    .SetParent<ndn::App>()
    .AddConstructor<CustomAppCsUpdater>()
    .AddAttribute("Prefix", "Requested prefix", StringValue("/dumb-interest"),
                    ndn::MakeNameAccessor(&CustomAppCsUpdater::m_prefix), ndn::MakeNameChecker())
    .AddAttribute("Service", "Requested service", StringValue("dumb-service"),
                    ndn::MakeNameAccessor(&CustomAppCsUpdater::m_service), ndn::MakeNameChecker());
  return tid;
}

CustomAppCsUpdater::CustomAppCsUpdater()
  : m_isRunning(false)
{
}

// Processing upon start of the application
void
CustomAppCsUpdater::StartApplication()
{
  // initialize ndn::App
  ndn::App::StartApplication();
  m_isRunning = true;

  m_name = m_prefix.ndn::Name::toUri() + m_service.ndn::Name::toUri();

  // Add entry to FIB for `/prefix/sub`
  //ndn::FibHelper::AddRoute(GetNode(), "/prefix/sub", m_face, 0);
  ndn::FibHelper::AddRoute(GetNode(), m_name, m_face, 0);

  NS_LOG_DEBUG("csUpdater added route to listen to: " << m_name);

  // Schedule send of first interest
  //Simulator::Schedule(Seconds(1.0), &CustomAppCsUpdater::SendInterest, this);
}

// Processing when application is stopped
void
CustomAppCsUpdater::StopApplication()
{
  m_isRunning = false;
  // cleanup ndn::App
  ndn::App::StopApplication();
}

void
CustomAppCsUpdater::SendInterest()
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
CustomAppCsUpdater::OnInterest(std::shared_ptr<const ndn::Interest> interest)
{
  ndn::App::OnInterest(interest);

  NS_LOG_DEBUG("CsUpdater received Interest packet for " << interest->getName());

  // Note that Interests send out by the app will not be sent back to the app !

  if(interest->getName().getPrefix(2).toUri() == "/nesco/csUpdate")
  {
    auto csParameterFromInterest = interest->getApplicationParameters();
    std::string csContentName = std::string(reinterpret_cast<const char*>(csParameterFromInterest.value()), csParameterFromInterest.value_size());

    NS_LOG_DEBUG("CsUpdater setting interest filter for cached content name: " << csContentName << '\n');
    // Add entry to FIB for `/prefix/sub`
    //ndn::FibHelper::AddRoute(GetNode(), "/prefix/sub", m_face, 0);
    ndn::FibHelper::AddRoute(GetNode(), csContentName, m_face, 0);
  }

}

// Callback that will be called when Data arrives
void
CustomAppCsUpdater::OnData(std::shared_ptr<const ndn::Data> data)
{
  NS_LOG_DEBUG("csUpdater application received dta (it's not supposed to). Data: " << data->getName());

  std::cout << "DATA received for name " << data->getName() << std::endl;
}

} // namespace ns3
