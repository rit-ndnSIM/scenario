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

// cabeee-cs-tools.cpp


#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ndnSIM-module.h"
#include "ns3/string.h"


namespace ns3 {

void
printCsHeader(std::ostream& os)
{
  os << "Time"
     << "\t"
     << "Node"
     << "\t"
     << "CsStaleUsage"
     << "\t"
     << "CsFreshUsage"
     << "\t"
     << "CsTotalUsage"
     << "\n";
}

void
printCsUsage(std::ostream& os, Time nextPrintTime, std::string PREFIX)
{
  Time simTime = Simulator::Now();


  uint64_t csAllFreshCount = 0;
  uint64_t csAllStaleCount = 0;
  uint64_t csAllTotalCount = 0;
  for (NodeList::Iterator node = NodeList::Begin(); node != NodeList::End(); node++) {
  

    //if (true != true) {
      //Ptr<ndn::ContentStore> cs = (*node)->GetObject<ndn::ContentStore>();
      //if (cs != 0)
        //csAllCount += cs->GetSize();
    //}
    //else {
      auto csSize = (*node)->GetObject<ndn::L3Protocol>()->getForwarder()->getCs().size();
      if (csSize != 0)
        csAllTotalCount += csSize;
    //}

    uint64_t csFreshSize = 0;
    uint64_t csStaleSize = 0;
    for (const auto& entry : (*node)->GetObject<ndn::L3Protocol>()->getForwarder()->getCs())
    {
      if (entry.getFullName().getPrefix(1).toUri() == PREFIX) //if the first part of the name matches /nesco (so that we analyze only entries used by our schemes, and not any /localhost/nfd forwarder entries)
      {
        if (entry.isFresh())
        {
          csFreshSize++;
          csAllFreshCount++;
        }
        else
        {
          csStaleSize++;
          csAllStaleCount++;
        }
      }
    }

    os << simTime.ToDouble(Time::S) << "\t";
    os << Names::FindName(*node) << "\t";
    os << csFreshSize << "\t";
    os << csStaleSize << "\t";
    os << csSize << "\n";



/*
    if (Names::FindName(*node) == "rtr-1")
    {
      //print cs contents
      for (const auto& entry : (*node)->GetObject<ndn::L3Protocol>()->getForwarder()->getCs())
      {
        std::cout << "CS Entry name: " << entry.getFullName() << ", with freshness: " << (entry.isFresh()?"Fresh":"NotFresh") << std::endl;
      }
    }
*/


  }

  os << simTime.ToDouble(Time::S) << "\t";
  os << "All Nodes\t";
  os << csAllFreshCount << "\t";
  os << csAllStaleCount << "\t";
  os << csAllTotalCount << "\n";


  Simulator::Schedule(nextPrintTime, &printCsUsage, ref(os), nextPrintTime, PREFIX);

}




} // namespace ns3
