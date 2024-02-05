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

// cabeee-dag-forwarder-app.hpp

#ifndef SERVICE_H_
#define SERVICE_H_

#include "ns3/ndnSIM/model/ndn-common.hpp"
#include <map>

namespace ns3 {

//class Service : public DagForwarderApp {
class Service {
public:
  //unsigned char
  //Service1();
  friend unsigned char
  //Service::Service1(std::vector <unsigned char> vectorOfServiceInputs)
  service3(std::vector <unsigned char> vectorOfServiceInputs);


//private:
  //void
  //privateFunc();
  
//private:
  //bool privateVar;
};

} // namespace ns3

#endif // SERVICE_H_
