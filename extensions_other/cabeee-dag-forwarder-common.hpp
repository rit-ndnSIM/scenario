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

// cabeee-dag-forwarder-common.hpp

#ifndef DAG_FORWARDER_COMMON_H_
#define DAG_FORWARDER_COMMON_H_

//#include "ns3/ndnSIM/model/ndn-common.hpp"
#include <list>

namespace ns3 {
namespace ndn {
namespace cabeee {

//using ::ndn::Name;
//namespace name = ::ndn::name;
//ATTRIBUTE_HELPER_HEADER(Name);

using std::shared_ptr;
using std::make_shared;

using ::ndn::Interest;
using ::ndn::Data;
using ::ndn::KeyLocator;
using ::ndn::SignatureInfo;
using ::ndn::Block;
using ::ndn::KeyChain;

using ::nfd::Face;
using ::ndn::FaceUri;

#ifndef DOXYGEN
// For python bindings
namespace nfd = ::nfd;
#endif // DOXYGEN

//using namespace std;

typedef struct {
  std::string thisService;
  std::list<std::string> feeds;
  //std::list<std::string> needs[]; //not necessary?
} DAG_SERVICE;

typedef struct {
  std::string head;
  //std::list<std::string> feeds; //not necessary?
  std::list<DAG_SERVICE> dagWorkflow;
  std::string hash;
} DAG_INTEREST;


//using ::ndn::make_unique;

} // namespace cabeee
} // namespace ndn
} // namespace ns3


#endif // DAG_FORWARDER_COMMON_H_
