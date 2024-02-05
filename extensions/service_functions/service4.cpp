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

// service1.cpp

#include "service4.hpp"


NS_LOG_COMPONENT_DEFINE("Service");

namespace ns3 {


//static unsigned char
unsigned char
//Service::Service1(std::vector <unsigned char> vectorOfServiceInputs)
Service::service4(std::vector <unsigned char> vectorOfServiceInputs)
{

  NS_LOG_DEBUG("Running service4 " << '\n');

  unsigned char serviceOutput = (vectorOfServiceInputs[0])*3 + (vectorOfServiceInputs[1])*4;
    
  NS_LOG_DEBUG("Service1 has output: " << (int)serviceOutput);
  
  return serviceOutput;
  
}

} // namespace ns3
