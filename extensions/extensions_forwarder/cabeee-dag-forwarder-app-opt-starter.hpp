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

#ifndef NDNSIM_CABEEE_DAG_FORWARDER_APP_OPT_STARTER_HPP
#define NDNSIM_CABEEE_DAG_FORWARDER_APP_OPT_STARTER_HPP

#include "cabeee-dag-forwarder-app-opt.hpp"


#include "ns3/ndnSIM/helper/ndn-stack-helper.hpp"
#include "ns3/ndnSIM/helper/ndn-fib-helper.hpp"
#include "ns3/application.h"
#include "ns3/string.h"

#include "model/ndn-l3-protocol.hpp"
#include "model/ndn-app-link-service.hpp"
#include "model/null-transport.hpp"

NS_LOG_COMPONENT_DEFINE("DagForwarderAppOpt");

namespace ns3 {

// Class inheriting from ns3::Application
class DagForwarderAppOptStarter : public Application
{
public:
  static TypeId
  GetTypeId()
  {
    static TypeId tid = TypeId("DagForwarderAppOptStarter")
      .SetParent<Application>()
      .AddConstructor<DagForwarderAppOptStarter>()
      .AddAttribute("Prefix", "Requested prefix", StringValue("/dumb-interest"),
                      ndn::MakeNameAccessor(&DagForwarderAppOptStarter::m_prefix), ndn::MakeNameChecker())
      .AddAttribute("Service", "Requested service", StringValue("dumb-service"),
                      ndn::MakeNameAccessor(&DagForwarderAppOptStarter::m_service), ndn::MakeNameChecker());

    return tid;
  }

protected:
  // inherited from Application base class.
  virtual void
  StartApplication()
  {
    /*
    // step 1. Create a face
    auto appLink = make_unique<AppLinkService>(this);
    auto transport = make_unique<NullTransport>("appFace://", "appFace://", ::ndn::nfd::FACE_SCOPE_LOCAL);
    // @TODO Consider making AppTransport instead
    m_face = std::make_shared<Face>(std::move(appLink), std::move(transport));
    m_appLink = static_cast<AppLinkService*>(m_face->getLinkService());
    m_face->setMetric(1);
    // step 2. Add face to the Ndn stack
    GetNode()->GetObject<L3Protocol>()->addFace(m_face);

    ndn::Name name = m_prefix.ndn::Name::toUri() + m_service.ndn::Name::toUri();
    ndn::FibHelper::AddRoute(GetNode(), name, m_face, 0);
    */
    // Create an instance of the app, and passing the dummy version of KeyChain (no real signing)
    m_instance.reset(new app::DagForwarderAppOpt(ndn::StackHelper::getKeyChain()));
    //m_instance.reset(new app::DagForwarderAppOpt());
    m_instance->run(m_prefix.toUri(), m_service.toUri()); // can be omitted
  }

  virtual void
  StopApplication()
  {
    //m_face->close();
    // Stop and destroy the instance of the app
    m_instance.reset();
  }

private:
  std::unique_ptr<app::DagForwarderAppOpt> m_instance;
  ndn::Name m_prefix;
  ndn::Name m_service;
//protected:
  //shared_ptr<Face> m_face;
  //AppLinkService* m_appLink;
};

} // namespace ns3

#endif // NDNSIM_CABEEE_DAG_FORWARDER_APP_OPT_STARTER_HPP