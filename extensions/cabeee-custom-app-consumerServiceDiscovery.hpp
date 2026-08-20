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

// cabeee-custom-app-consumerServiceDiscovery.hpp

#ifndef CUSTOM_APP_CONSUMERSERVICEDISCOVERY_H_
#define CUSTOM_APP_CONSUMERSERVICEDISCOVERY_H_

#include "ns3/ndnSIM/model/ndn-common.hpp"

#include "ns3/ndnSIM/apps/ndn-app.hpp"

namespace ns3 {

/**
 * @brief A simple custom application
 *
 * This applications demonstrates how to send Interests and respond with Datas to incoming interests
 *
 * When application starts it "sets interest filter" (install FIB entry) for /prefix/sub, as well as
 * sends Interest for this prefix
 *
 * When an Interest is received, it is replied with a Data with 1024-byte fake payload
 */
class CustomAppConsumerServiceDiscovery : public ndn::App {
public:
  // register NS-3 type "CustomAppConsumerServiceDiscovery"
  static TypeId
  GetTypeId();

  CustomAppConsumerServiceDiscovery();

  // (overridden from ndn::App) Processing upon start of the application
  virtual void
  StartApplication();

  // (overridden from ndn::App) Processing when application is stopped
  virtual void
  StopApplication();

  // (overridden from ndn::App) Callback that will be called when Interest arrives
  virtual void
  OnInterest(std::shared_ptr<const ndn::Interest> interest);

  // (overridden from ndn::App) Callback that will be called when Data arrives
  virtual void
  OnData(std::shared_ptr<const ndn::Data> contentObject);

private:
  void
  SendInterest();
  void
  SendSDInterest();
  
private:
  bool m_isRunning;
  ndn::Name m_prefix;
  ndn::Name m_service;
  ndn::Name m_SDName;

  // Cached toUri() of the ndn::Name members above. These are ns-3 Attributes, fixed before
  // StartApplication() and never reassigned, so their URI form is constant for the run. Calling
  // toUri() rebuilds and reallocates the string every time, and it was being called on these
  // several times per interest on the hot path, so we build each once in StartApplication().
  std::string m_prefixUri;
  std::string m_serviceUri;
  std::string m_SDNameUri;

  uint16_t m_orchestrate;
  uint16_t m_serviceDiscovery;
  uint16_t m_resourceUtilization;
  uint16_t m_resourceAllocation;
  uint16_t m_allocationReuse;
  uint16_t m_scheduleCompaction;
  std::string m_dagPath;
  Time m_appStartTime;
  bool m_SDrunning;
  bool m_WFrunning;
  Time m_SDstartTimeOffset;
  Time m_SDstartTime;
  Time m_SDendTime;
  Time m_WFstartTimeOffset;
  Time m_WFstartTime;
  Time m_WFendTime;
  double m_frequency; // how many interests per second on average?
  uint16_t m_numInterests; // total number of interests to generate
  uint16_t m_poisson; // 1 or 0 for true/false
  uint16_t m_interestNum; // to keep track of how many interests we've generated
  double m_SDtimeoutComputationMultiplier; // multiplier for how long to consider computation in EFT timeouts. -1 means don't perform timeout optmization at all. Zero means computation not considered. >0 assigns the multiplier. Ex: 10 means 10x faster computation available assumed in nodes that have not yet responded.
};

} // namespace ns3

#endif // CUSTOM_APP_CONSUMERSERVICEDISCOVERY_H_
