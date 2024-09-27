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

// ndn-custom-apps.cpp

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ndnSIM-module.h"
#include "ns3/string.h"

#define PREFIX "/orchA"

namespace ns3 {

int
main(int argc, char* argv[])
{
  // Read optional command-line parameters (e.g., enable visualizer with ./waf --run=<> --visualize
  CommandLine cmd;
  cmd.Parse(argc, argv);

  // Creating nodes
  AnnotatedTopologyReader topologyReader("", 1);
  topologyReader.SetFileName("topologies/topo-cabeee-cpm-x_250.txt");
  topologyReader.Read();




  // Install NDN stack on all nodes
  ndn::StackHelper ndnHelper;
  ndnHelper.setPolicy("nfd::cs::lru");
  //ndnHelper.setCsSize(100);
  //ndnHelper.setCsSize(1);
  ndnHelper.SetDefaultRoutes(true);

  //ndnHelper.InstallAll();

  // Getting containers for the nodes
  Ptr<Node> producer = Names::Find<Node>("sensor");
  Ptr<Node> rtr001 = Names::Find<Node>("rtr-001");
  Ptr<Node> rtr002 = Names::Find<Node>("rtr-002");
  Ptr<Node> rtr003 = Names::Find<Node>("rtr-003");
  Ptr<Node> rtr004 = Names::Find<Node>("rtr-004");
  Ptr<Node> rtr005 = Names::Find<Node>("rtr-005");
  Ptr<Node> rtr006 = Names::Find<Node>("rtr-006");
  Ptr<Node> rtr007 = Names::Find<Node>("rtr-007");
  Ptr<Node> rtr008 = Names::Find<Node>("rtr-008");
  Ptr<Node> rtr009 = Names::Find<Node>("rtr-009");
  Ptr<Node> rtr010 = Names::Find<Node>("rtr-010");
  Ptr<Node> rtr011 = Names::Find<Node>("rtr-011");
  Ptr<Node> rtr012 = Names::Find<Node>("rtr-012");
  Ptr<Node> rtr013 = Names::Find<Node>("rtr-013");
  Ptr<Node> rtr014 = Names::Find<Node>("rtr-014");
  Ptr<Node> rtr015 = Names::Find<Node>("rtr-015");
  Ptr<Node> rtr016 = Names::Find<Node>("rtr-016");
  Ptr<Node> rtr017 = Names::Find<Node>("rtr-017");
  Ptr<Node> rtr018 = Names::Find<Node>("rtr-018");
  Ptr<Node> rtr019 = Names::Find<Node>("rtr-019");
  Ptr<Node> rtr020 = Names::Find<Node>("rtr-020");
  Ptr<Node> rtr021 = Names::Find<Node>("rtr-021");
  Ptr<Node> rtr022 = Names::Find<Node>("rtr-022");
  Ptr<Node> rtr023 = Names::Find<Node>("rtr-023");
  Ptr<Node> rtr024 = Names::Find<Node>("rtr-024");
  Ptr<Node> rtr025 = Names::Find<Node>("rtr-025");
  Ptr<Node> rtr026 = Names::Find<Node>("rtr-026");
  Ptr<Node> rtr027 = Names::Find<Node>("rtr-027");
  Ptr<Node> rtr028 = Names::Find<Node>("rtr-028");
  Ptr<Node> rtr029 = Names::Find<Node>("rtr-029");
  Ptr<Node> rtr030 = Names::Find<Node>("rtr-030");
  Ptr<Node> rtr031 = Names::Find<Node>("rtr-031");
  Ptr<Node> rtr032 = Names::Find<Node>("rtr-032");
  Ptr<Node> rtr033 = Names::Find<Node>("rtr-033");
  Ptr<Node> rtr034 = Names::Find<Node>("rtr-034");
  Ptr<Node> rtr035 = Names::Find<Node>("rtr-035");
  Ptr<Node> rtr036 = Names::Find<Node>("rtr-036");
  Ptr<Node> rtr037 = Names::Find<Node>("rtr-037");
  Ptr<Node> rtr038 = Names::Find<Node>("rtr-038");
  Ptr<Node> rtr039 = Names::Find<Node>("rtr-039");
  Ptr<Node> rtr040 = Names::Find<Node>("rtr-040");
  Ptr<Node> rtr041 = Names::Find<Node>("rtr-041");
  Ptr<Node> rtr042 = Names::Find<Node>("rtr-042");
  Ptr<Node> rtr043 = Names::Find<Node>("rtr-043");
  Ptr<Node> rtr044 = Names::Find<Node>("rtr-044");
  Ptr<Node> rtr045 = Names::Find<Node>("rtr-045");
  Ptr<Node> rtr046 = Names::Find<Node>("rtr-046");
  Ptr<Node> rtr047 = Names::Find<Node>("rtr-047");
  Ptr<Node> rtr048 = Names::Find<Node>("rtr-048");
  Ptr<Node> rtr049 = Names::Find<Node>("rtr-049");
  Ptr<Node> rtr050 = Names::Find<Node>("rtr-050");
  Ptr<Node> rtr051 = Names::Find<Node>("rtr-051");
  Ptr<Node> rtr052 = Names::Find<Node>("rtr-052");
  Ptr<Node> rtr053 = Names::Find<Node>("rtr-053");
  Ptr<Node> rtr054 = Names::Find<Node>("rtr-054");
  Ptr<Node> rtr055 = Names::Find<Node>("rtr-055");
  Ptr<Node> rtr056 = Names::Find<Node>("rtr-056");
  Ptr<Node> rtr057 = Names::Find<Node>("rtr-057");
  Ptr<Node> rtr058 = Names::Find<Node>("rtr-058");
  Ptr<Node> rtr059 = Names::Find<Node>("rtr-059");
  Ptr<Node> rtr060 = Names::Find<Node>("rtr-060");
  Ptr<Node> rtr061 = Names::Find<Node>("rtr-061");
  Ptr<Node> rtr062 = Names::Find<Node>("rtr-062");
  Ptr<Node> rtr063 = Names::Find<Node>("rtr-063");
  Ptr<Node> rtr064 = Names::Find<Node>("rtr-064");
  Ptr<Node> rtr065 = Names::Find<Node>("rtr-065");
  Ptr<Node> rtr066 = Names::Find<Node>("rtr-066");
  Ptr<Node> rtr067 = Names::Find<Node>("rtr-067");
  Ptr<Node> rtr068 = Names::Find<Node>("rtr-068");
  Ptr<Node> rtr069 = Names::Find<Node>("rtr-069");
  Ptr<Node> rtr070 = Names::Find<Node>("rtr-070");
  Ptr<Node> rtr071 = Names::Find<Node>("rtr-071");
  Ptr<Node> rtr072 = Names::Find<Node>("rtr-072");
  Ptr<Node> rtr073 = Names::Find<Node>("rtr-073");
  Ptr<Node> rtr074 = Names::Find<Node>("rtr-074");
  Ptr<Node> rtr075 = Names::Find<Node>("rtr-075");
  Ptr<Node> rtr076 = Names::Find<Node>("rtr-076");
  Ptr<Node> rtr077 = Names::Find<Node>("rtr-077");
  Ptr<Node> rtr078 = Names::Find<Node>("rtr-078");
  Ptr<Node> rtr079 = Names::Find<Node>("rtr-079");
  Ptr<Node> rtr080 = Names::Find<Node>("rtr-080");
  Ptr<Node> rtr081 = Names::Find<Node>("rtr-081");
  Ptr<Node> rtr082 = Names::Find<Node>("rtr-082");
  Ptr<Node> rtr083 = Names::Find<Node>("rtr-083");
  Ptr<Node> rtr084 = Names::Find<Node>("rtr-084");
  Ptr<Node> rtr085 = Names::Find<Node>("rtr-085");
  Ptr<Node> rtr086 = Names::Find<Node>("rtr-086");
  Ptr<Node> rtr087 = Names::Find<Node>("rtr-087");
  Ptr<Node> rtr088 = Names::Find<Node>("rtr-088");
  Ptr<Node> rtr089 = Names::Find<Node>("rtr-089");
  Ptr<Node> rtr090 = Names::Find<Node>("rtr-090");
  Ptr<Node> rtr091 = Names::Find<Node>("rtr-091");
  Ptr<Node> rtr092 = Names::Find<Node>("rtr-092");
  Ptr<Node> rtr093 = Names::Find<Node>("rtr-093");
  Ptr<Node> rtr094 = Names::Find<Node>("rtr-094");
  Ptr<Node> rtr095 = Names::Find<Node>("rtr-095");
  Ptr<Node> rtr096 = Names::Find<Node>("rtr-096");
  Ptr<Node> rtr097 = Names::Find<Node>("rtr-097");
  Ptr<Node> rtr098 = Names::Find<Node>("rtr-098");
  Ptr<Node> rtr099 = Names::Find<Node>("rtr-099");
  Ptr<Node> rtr100 = Names::Find<Node>("rtr-100");
  Ptr<Node> rtr101 = Names::Find<Node>("rtr-101");
  Ptr<Node> rtr102 = Names::Find<Node>("rtr-102");
  Ptr<Node> rtr103 = Names::Find<Node>("rtr-103");
  Ptr<Node> rtr104 = Names::Find<Node>("rtr-104");
  Ptr<Node> rtr105 = Names::Find<Node>("rtr-105");
  Ptr<Node> rtr106 = Names::Find<Node>("rtr-106");
  Ptr<Node> rtr107 = Names::Find<Node>("rtr-107");
  Ptr<Node> rtr108 = Names::Find<Node>("rtr-108");
  Ptr<Node> rtr109 = Names::Find<Node>("rtr-109");
  Ptr<Node> rtr110 = Names::Find<Node>("rtr-110");
  Ptr<Node> rtr111 = Names::Find<Node>("rtr-111");
  Ptr<Node> rtr112 = Names::Find<Node>("rtr-112");
  Ptr<Node> rtr113 = Names::Find<Node>("rtr-113");
  Ptr<Node> rtr114 = Names::Find<Node>("rtr-114");
  Ptr<Node> rtr115 = Names::Find<Node>("rtr-115");
  Ptr<Node> rtr116 = Names::Find<Node>("rtr-116");
  Ptr<Node> rtr117 = Names::Find<Node>("rtr-117");
  Ptr<Node> rtr118 = Names::Find<Node>("rtr-118");
  Ptr<Node> rtr119 = Names::Find<Node>("rtr-119");
  Ptr<Node> rtr120 = Names::Find<Node>("rtr-120");
  Ptr<Node> rtr121 = Names::Find<Node>("rtr-121");
  Ptr<Node> rtr122 = Names::Find<Node>("rtr-122");
  Ptr<Node> rtr123 = Names::Find<Node>("rtr-123");
  Ptr<Node> rtr124 = Names::Find<Node>("rtr-124");
  Ptr<Node> rtr125 = Names::Find<Node>("rtr-125");
  Ptr<Node> rtr126 = Names::Find<Node>("rtr-126");
  Ptr<Node> rtr127 = Names::Find<Node>("rtr-127");
  Ptr<Node> rtr128 = Names::Find<Node>("rtr-128");
  Ptr<Node> rtr129 = Names::Find<Node>("rtr-129");
  Ptr<Node> rtr130 = Names::Find<Node>("rtr-130");
  Ptr<Node> rtr131 = Names::Find<Node>("rtr-131");
  Ptr<Node> rtr132 = Names::Find<Node>("rtr-132");
  Ptr<Node> rtr133 = Names::Find<Node>("rtr-133");
  Ptr<Node> rtr134 = Names::Find<Node>("rtr-134");
  Ptr<Node> rtr135 = Names::Find<Node>("rtr-135");
  Ptr<Node> rtr136 = Names::Find<Node>("rtr-136");
  Ptr<Node> rtr137 = Names::Find<Node>("rtr-137");
  Ptr<Node> rtr138 = Names::Find<Node>("rtr-138");
  Ptr<Node> rtr139 = Names::Find<Node>("rtr-139");
  Ptr<Node> rtr140 = Names::Find<Node>("rtr-140");
  Ptr<Node> rtr141 = Names::Find<Node>("rtr-141");
  Ptr<Node> rtr142 = Names::Find<Node>("rtr-142");
  Ptr<Node> rtr143 = Names::Find<Node>("rtr-143");
  Ptr<Node> rtr144 = Names::Find<Node>("rtr-144");
  Ptr<Node> rtr145 = Names::Find<Node>("rtr-145");
  Ptr<Node> rtr146 = Names::Find<Node>("rtr-146");
  Ptr<Node> rtr147 = Names::Find<Node>("rtr-147");
  Ptr<Node> rtr148 = Names::Find<Node>("rtr-148");
  Ptr<Node> rtr149 = Names::Find<Node>("rtr-149");
  Ptr<Node> rtr150 = Names::Find<Node>("rtr-150");
  Ptr<Node> rtr151 = Names::Find<Node>("rtr-151");
  Ptr<Node> rtr152 = Names::Find<Node>("rtr-152");
  Ptr<Node> rtr153 = Names::Find<Node>("rtr-153");
  Ptr<Node> rtr154 = Names::Find<Node>("rtr-154");
  Ptr<Node> rtr155 = Names::Find<Node>("rtr-155");
  Ptr<Node> rtr156 = Names::Find<Node>("rtr-156");
  Ptr<Node> rtr157 = Names::Find<Node>("rtr-157");
  Ptr<Node> rtr158 = Names::Find<Node>("rtr-158");
  Ptr<Node> rtr159 = Names::Find<Node>("rtr-159");
  Ptr<Node> rtr160 = Names::Find<Node>("rtr-160");
  Ptr<Node> rtr161 = Names::Find<Node>("rtr-161");
  Ptr<Node> rtr162 = Names::Find<Node>("rtr-162");
  Ptr<Node> rtr163 = Names::Find<Node>("rtr-163");
  Ptr<Node> rtr164 = Names::Find<Node>("rtr-164");
  Ptr<Node> rtr165 = Names::Find<Node>("rtr-165");
  Ptr<Node> rtr166 = Names::Find<Node>("rtr-166");
  Ptr<Node> rtr167 = Names::Find<Node>("rtr-167");
  Ptr<Node> rtr168 = Names::Find<Node>("rtr-168");
  Ptr<Node> rtr169 = Names::Find<Node>("rtr-169");
  Ptr<Node> rtr170 = Names::Find<Node>("rtr-170");
  Ptr<Node> rtr171 = Names::Find<Node>("rtr-171");
  Ptr<Node> rtr172 = Names::Find<Node>("rtr-172");
  Ptr<Node> rtr173 = Names::Find<Node>("rtr-173");
  Ptr<Node> rtr174 = Names::Find<Node>("rtr-174");
  Ptr<Node> rtr175 = Names::Find<Node>("rtr-175");
  Ptr<Node> rtr176 = Names::Find<Node>("rtr-176");
  Ptr<Node> rtr177 = Names::Find<Node>("rtr-177");
  Ptr<Node> rtr178 = Names::Find<Node>("rtr-178");
  Ptr<Node> rtr179 = Names::Find<Node>("rtr-179");
  Ptr<Node> rtr180 = Names::Find<Node>("rtr-180");
  Ptr<Node> rtr181 = Names::Find<Node>("rtr-181");
  Ptr<Node> rtr182 = Names::Find<Node>("rtr-182");
  Ptr<Node> rtr183 = Names::Find<Node>("rtr-183");
  Ptr<Node> rtr184 = Names::Find<Node>("rtr-184");
  Ptr<Node> rtr185 = Names::Find<Node>("rtr-185");
  Ptr<Node> rtr186 = Names::Find<Node>("rtr-186");
  Ptr<Node> rtr187 = Names::Find<Node>("rtr-187");
  Ptr<Node> rtr188 = Names::Find<Node>("rtr-188");
  Ptr<Node> rtr189 = Names::Find<Node>("rtr-189");
  Ptr<Node> rtr190 = Names::Find<Node>("rtr-190");
  Ptr<Node> rtr191 = Names::Find<Node>("rtr-191");
  Ptr<Node> rtr192 = Names::Find<Node>("rtr-192");
  Ptr<Node> rtr193 = Names::Find<Node>("rtr-193");
  Ptr<Node> rtr194 = Names::Find<Node>("rtr-194");
  Ptr<Node> rtr195 = Names::Find<Node>("rtr-195");
  Ptr<Node> rtr196 = Names::Find<Node>("rtr-196");
  Ptr<Node> rtr197 = Names::Find<Node>("rtr-197");
  Ptr<Node> rtr198 = Names::Find<Node>("rtr-198");
  Ptr<Node> rtr199 = Names::Find<Node>("rtr-199");
  Ptr<Node> rtr200 = Names::Find<Node>("rtr-200");
  Ptr<Node> rtr201 = Names::Find<Node>("rtr-201");
  Ptr<Node> rtr202 = Names::Find<Node>("rtr-202");
  Ptr<Node> rtr203 = Names::Find<Node>("rtr-203");
  Ptr<Node> rtr204 = Names::Find<Node>("rtr-204");
  Ptr<Node> rtr205 = Names::Find<Node>("rtr-205");
  Ptr<Node> rtr206 = Names::Find<Node>("rtr-206");
  Ptr<Node> rtr207 = Names::Find<Node>("rtr-207");
  Ptr<Node> rtr208 = Names::Find<Node>("rtr-208");
  Ptr<Node> rtr209 = Names::Find<Node>("rtr-209");
  Ptr<Node> rtr210 = Names::Find<Node>("rtr-210");
  Ptr<Node> rtr211 = Names::Find<Node>("rtr-211");
  Ptr<Node> rtr212 = Names::Find<Node>("rtr-212");
  Ptr<Node> rtr213 = Names::Find<Node>("rtr-213");
  Ptr<Node> rtr214 = Names::Find<Node>("rtr-214");
  Ptr<Node> rtr215 = Names::Find<Node>("rtr-215");
  Ptr<Node> rtr216 = Names::Find<Node>("rtr-216");
  Ptr<Node> rtr217 = Names::Find<Node>("rtr-217");
  Ptr<Node> rtr218 = Names::Find<Node>("rtr-218");
  Ptr<Node> rtr219 = Names::Find<Node>("rtr-219");
  Ptr<Node> rtr220 = Names::Find<Node>("rtr-220");
  Ptr<Node> rtr221 = Names::Find<Node>("rtr-221");
  Ptr<Node> rtr222 = Names::Find<Node>("rtr-222");
  Ptr<Node> rtr223 = Names::Find<Node>("rtr-223");
  Ptr<Node> rtr224 = Names::Find<Node>("rtr-224");
  Ptr<Node> rtr225 = Names::Find<Node>("rtr-225");
  Ptr<Node> rtr226 = Names::Find<Node>("rtr-226");
  Ptr<Node> rtr227 = Names::Find<Node>("rtr-227");
  Ptr<Node> rtr228 = Names::Find<Node>("rtr-228");
  Ptr<Node> rtr229 = Names::Find<Node>("rtr-229");
  Ptr<Node> rtr230 = Names::Find<Node>("rtr-230");
  Ptr<Node> rtr231 = Names::Find<Node>("rtr-231");
  Ptr<Node> rtr232 = Names::Find<Node>("rtr-232");
  Ptr<Node> rtr233 = Names::Find<Node>("rtr-233");
  Ptr<Node> rtr234 = Names::Find<Node>("rtr-234");
  Ptr<Node> rtr235 = Names::Find<Node>("rtr-235");
  Ptr<Node> rtr236 = Names::Find<Node>("rtr-236");
  Ptr<Node> rtr237 = Names::Find<Node>("rtr-237");
  Ptr<Node> rtr238 = Names::Find<Node>("rtr-238");
  Ptr<Node> rtr239 = Names::Find<Node>("rtr-239");
  Ptr<Node> rtr240 = Names::Find<Node>("rtr-240");
  Ptr<Node> rtr241 = Names::Find<Node>("rtr-241");
  Ptr<Node> rtr242 = Names::Find<Node>("rtr-242");
  Ptr<Node> rtr243 = Names::Find<Node>("rtr-243");
  Ptr<Node> rtr244 = Names::Find<Node>("rtr-244");
  Ptr<Node> rtr245 = Names::Find<Node>("rtr-245");
  Ptr<Node> rtr246 = Names::Find<Node>("rtr-246");
  Ptr<Node> rtr247 = Names::Find<Node>("rtr-247");
  Ptr<Node> rtr248 = Names::Find<Node>("rtr-248");
  Ptr<Node> rtr249 = Names::Find<Node>("rtr-249");
  Ptr<Node> rtr250 = Names::Find<Node>("rtr-250");
  Ptr<Node> consumer = Names::Find<Node>("user");

  ndnHelper.setCsSize(0); // disable content store
  ndnHelper.Install(producer);
  ndnHelper.Install(rtr001);
  ndnHelper.Install(rtr002);
  ndnHelper.Install(rtr003);
  ndnHelper.Install(rtr004);
  ndnHelper.Install(rtr005);
  ndnHelper.Install(rtr006);
  ndnHelper.Install(rtr007);
  ndnHelper.Install(rtr008);
  ndnHelper.Install(rtr009);
  ndnHelper.Install(rtr010);
  ndnHelper.Install(rtr011);
  ndnHelper.Install(rtr012);
  ndnHelper.Install(rtr013);
  ndnHelper.Install(rtr014);
  ndnHelper.Install(rtr015);
  ndnHelper.Install(rtr016);
  ndnHelper.Install(rtr017);
  ndnHelper.Install(rtr018);
  ndnHelper.Install(rtr019);
  ndnHelper.Install(rtr020);
  ndnHelper.Install(rtr021);
  ndnHelper.Install(rtr022);
  ndnHelper.Install(rtr023);
  ndnHelper.Install(rtr024);
  ndnHelper.Install(rtr025);
  ndnHelper.Install(rtr026);
  ndnHelper.Install(rtr027);
  ndnHelper.Install(rtr028);
  ndnHelper.Install(rtr029);
  ndnHelper.Install(rtr030);
  ndnHelper.Install(rtr031);
  ndnHelper.Install(rtr032);
  ndnHelper.Install(rtr033);
  ndnHelper.Install(rtr034);
  ndnHelper.Install(rtr035);
  ndnHelper.Install(rtr036);
  ndnHelper.Install(rtr037);
  ndnHelper.Install(rtr038);
  ndnHelper.Install(rtr039);
  ndnHelper.Install(rtr040);
  ndnHelper.Install(rtr041);
  ndnHelper.Install(rtr042);
  ndnHelper.Install(rtr043);
  ndnHelper.Install(rtr044);
  ndnHelper.Install(rtr045);
  ndnHelper.Install(rtr046);
  ndnHelper.Install(rtr047);
  ndnHelper.Install(rtr048);
  ndnHelper.Install(rtr049);
  ndnHelper.Install(rtr050);
  ndnHelper.Install(rtr051);
  ndnHelper.Install(rtr052);
  ndnHelper.Install(rtr053);
  ndnHelper.Install(rtr054);
  ndnHelper.Install(rtr055);
  ndnHelper.Install(rtr056);
  ndnHelper.Install(rtr057);
  ndnHelper.Install(rtr058);
  ndnHelper.Install(rtr059);
  ndnHelper.Install(rtr060);
  ndnHelper.Install(rtr061);
  ndnHelper.Install(rtr062);
  ndnHelper.Install(rtr063);
  ndnHelper.Install(rtr064);
  ndnHelper.Install(rtr065);
  ndnHelper.Install(rtr066);
  ndnHelper.Install(rtr067);
  ndnHelper.Install(rtr068);
  ndnHelper.Install(rtr069);
  ndnHelper.Install(rtr070);
  ndnHelper.Install(rtr071);
  ndnHelper.Install(rtr072);
  ndnHelper.Install(rtr073);
  ndnHelper.Install(rtr074);
  ndnHelper.Install(rtr075);
  ndnHelper.Install(rtr076);
  ndnHelper.Install(rtr077);
  ndnHelper.Install(rtr078);
  ndnHelper.Install(rtr079);
  ndnHelper.Install(rtr080);
  ndnHelper.Install(rtr081);
  ndnHelper.Install(rtr082);
  ndnHelper.Install(rtr083);
  ndnHelper.Install(rtr084);
  ndnHelper.Install(rtr085);
  ndnHelper.Install(rtr086);
  ndnHelper.Install(rtr087);
  ndnHelper.Install(rtr088);
  ndnHelper.Install(rtr089);
  ndnHelper.Install(rtr090);
  ndnHelper.Install(rtr091);
  ndnHelper.Install(rtr092);
  ndnHelper.Install(rtr093);
  ndnHelper.Install(rtr094);
  ndnHelper.Install(rtr095);
  ndnHelper.Install(rtr096);
  ndnHelper.Install(rtr097);
  ndnHelper.Install(rtr098);
  ndnHelper.Install(rtr099);
  ndnHelper.Install(rtr100);
  ndnHelper.Install(rtr101);
  ndnHelper.Install(rtr102);
  ndnHelper.Install(rtr103);
  ndnHelper.Install(rtr104);
  ndnHelper.Install(rtr105);
  ndnHelper.Install(rtr106);
  ndnHelper.Install(rtr107);
  ndnHelper.Install(rtr108);
  ndnHelper.Install(rtr109);
  ndnHelper.Install(rtr110);
  ndnHelper.Install(rtr111);
  ndnHelper.Install(rtr112);
  ndnHelper.Install(rtr113);
  ndnHelper.Install(rtr114);
  ndnHelper.Install(rtr115);
  ndnHelper.Install(rtr116);
  ndnHelper.Install(rtr117);
  ndnHelper.Install(rtr118);
  ndnHelper.Install(rtr119);
  ndnHelper.Install(rtr120);
  ndnHelper.Install(rtr121);
  ndnHelper.Install(rtr122);
  ndnHelper.Install(rtr123);
  ndnHelper.Install(rtr124);
  ndnHelper.Install(rtr125);
  ndnHelper.Install(rtr126);
  ndnHelper.Install(rtr127);
  ndnHelper.Install(rtr128);
  ndnHelper.Install(rtr129);
  ndnHelper.Install(rtr130);
  ndnHelper.Install(rtr131);
  ndnHelper.Install(rtr132);
  ndnHelper.Install(rtr133);
  ndnHelper.Install(rtr134);
  ndnHelper.Install(rtr135);
  ndnHelper.Install(rtr136);
  ndnHelper.Install(rtr137);
  ndnHelper.Install(rtr138);
  ndnHelper.Install(rtr139);
  ndnHelper.Install(rtr140);
  ndnHelper.Install(rtr141);
  ndnHelper.Install(rtr142);
  ndnHelper.Install(rtr143);
  ndnHelper.Install(rtr144);
  ndnHelper.Install(rtr145);
  ndnHelper.Install(rtr146);
  ndnHelper.Install(rtr147);
  ndnHelper.Install(rtr148);
  ndnHelper.Install(rtr149);
  ndnHelper.Install(rtr150);
  ndnHelper.Install(rtr151);
  ndnHelper.Install(rtr152);
  ndnHelper.Install(rtr153);
  ndnHelper.Install(rtr154);
  ndnHelper.Install(rtr155);
  ndnHelper.Install(rtr156);
  ndnHelper.Install(rtr157);
  ndnHelper.Install(rtr158);
  ndnHelper.Install(rtr159);
  ndnHelper.Install(rtr160);
  ndnHelper.Install(rtr161);
  ndnHelper.Install(rtr162);
  ndnHelper.Install(rtr163);
  ndnHelper.Install(rtr164);
  ndnHelper.Install(rtr165);
  ndnHelper.Install(rtr166);
  ndnHelper.Install(rtr167);
  ndnHelper.Install(rtr168);
  ndnHelper.Install(rtr169);
  ndnHelper.Install(rtr170);
  ndnHelper.Install(rtr171);
  ndnHelper.Install(rtr172);
  ndnHelper.Install(rtr173);
  ndnHelper.Install(rtr174);
  ndnHelper.Install(rtr175);
  ndnHelper.Install(rtr176);
  ndnHelper.Install(rtr177);
  ndnHelper.Install(rtr178);
  ndnHelper.Install(rtr179);
  ndnHelper.Install(rtr180);
  ndnHelper.Install(rtr181);
  ndnHelper.Install(rtr182);
  ndnHelper.Install(rtr183);
  ndnHelper.Install(rtr184);
  ndnHelper.Install(rtr185);
  ndnHelper.Install(rtr186);
  ndnHelper.Install(rtr187);
  ndnHelper.Install(rtr188);
  ndnHelper.Install(rtr189);
  ndnHelper.Install(rtr190);
  ndnHelper.Install(rtr191);
  ndnHelper.Install(rtr192);
  ndnHelper.Install(rtr193);
  ndnHelper.Install(rtr194);
  ndnHelper.Install(rtr195);
  ndnHelper.Install(rtr196);
  ndnHelper.Install(rtr197);
  ndnHelper.Install(rtr198);
  ndnHelper.Install(rtr199);
  ndnHelper.Install(rtr200);
  ndnHelper.Install(rtr201);
  ndnHelper.Install(rtr202);
  ndnHelper.Install(rtr203);
  ndnHelper.Install(rtr204);
  ndnHelper.Install(rtr205);
  ndnHelper.Install(rtr206);
  ndnHelper.Install(rtr207);
  ndnHelper.Install(rtr208);
  ndnHelper.Install(rtr209);
  ndnHelper.Install(rtr210);
  ndnHelper.Install(rtr211);
  ndnHelper.Install(rtr212);
  ndnHelper.Install(rtr213);
  ndnHelper.Install(rtr214);
  ndnHelper.Install(rtr215);
  ndnHelper.Install(rtr216);
  ndnHelper.Install(rtr217);
  ndnHelper.Install(rtr218);
  ndnHelper.Install(rtr219);
  ndnHelper.Install(rtr220);
  ndnHelper.Install(rtr221);
  ndnHelper.Install(rtr222);
  ndnHelper.Install(rtr223);
  ndnHelper.Install(rtr224);
  ndnHelper.Install(rtr225);
  ndnHelper.Install(rtr226);
  ndnHelper.Install(rtr227);
  ndnHelper.Install(rtr228);
  ndnHelper.Install(rtr229);
  ndnHelper.Install(rtr230);
  ndnHelper.Install(rtr231);
  ndnHelper.Install(rtr232);
  ndnHelper.Install(rtr233);
  ndnHelper.Install(rtr234);
  ndnHelper.Install(rtr235);
  ndnHelper.Install(rtr236);
  ndnHelper.Install(rtr237);
  ndnHelper.Install(rtr238);
  ndnHelper.Install(rtr239);
  ndnHelper.Install(rtr240);
  ndnHelper.Install(rtr241);
  ndnHelper.Install(rtr242);
  ndnHelper.Install(rtr243);
  ndnHelper.Install(rtr244);
  ndnHelper.Install(rtr245);
  ndnHelper.Install(rtr246);
  ndnHelper.Install(rtr247);
  ndnHelper.Install(rtr248);
  ndnHelper.Install(rtr249);
  ndnHelper.Install(rtr250);
  ndnHelper.Install(consumer);



  std::string Prefix(PREFIX);

  // Choosing forwarding strategy
  //ndn::StrategyChoiceHelper::InstallAll("/prefix", "/localhost/nfd/strategy/best-route");
  //ndn::StrategyChoiceHelper::InstallAll("/prefix", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-001", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-002", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-003", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-004", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-005", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-006", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-007", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-008", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-009", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-010", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-011", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-012", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-013", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-014", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-015", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-016", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-017", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-018", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-019", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-020", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-021", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-022", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-023", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-024", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-025", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-026", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-027", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-028", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-029", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-030", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-031", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-032", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-033", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-034", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-035", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-036", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-037", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-038", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-039", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-040", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-041", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-042", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-043", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-044", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-045", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-046", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-047", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-048", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-049", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceCPM-050", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/sensor", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::InstallAll(Prefix + "/serviceOrchestration", "/localhost/nfd/strategy/multicast");

  // Installing global routing interface on all nodes
  //ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
  //ndnGlobalRoutingHelper.InstallAll();

  // Getting containers for the consumer/producer
  //Ptr<Node> consumer = Names::Find<Node>("user");
  //Ptr<Node> producer = Names::Find<Node>("sensor");
  //Ptr<Node> producer = Names::Find<Node>("rtr-1");

 


  // Installing applications

  // Custom App for Sensor(Producer)
  ndn::AppHelper sensorApp("CustomAppProducer");
  sensorApp.SetPrefix(Prefix);
  sensorApp.SetAttribute("Service", StringValue("sensor"));
  sensorApp.Install(producer).Start(Seconds(0));

  // Custom App for routers
  ndn::AppHelper routerApp("DagServiceA_App");
  routerApp.SetPrefix(Prefix);
  routerApp.SetAttribute("Service", StringValue("serviceCPM-001"));
  routerApp.Install(rtr005).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-002"));
  routerApp.Install(rtr010).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-003"));
  routerApp.Install(rtr015).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-004"));
  routerApp.Install(rtr020).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-005"));
  routerApp.Install(rtr025).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-006"));
  routerApp.Install(rtr030).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-007"));
  routerApp.Install(rtr035).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-008"));
  routerApp.Install(rtr040).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-009"));
  routerApp.Install(rtr045).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-010"));
  routerApp.Install(rtr050).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-011"));
  routerApp.Install(rtr055).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-012"));
  routerApp.Install(rtr060).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-013"));
  routerApp.Install(rtr065).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-014"));
  routerApp.Install(rtr070).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-015"));
  routerApp.Install(rtr075).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-016"));
  routerApp.Install(rtr080).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-017"));
  routerApp.Install(rtr085).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-018"));
  routerApp.Install(rtr090).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-019"));
  routerApp.Install(rtr095).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-020"));
  routerApp.Install(rtr100).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-021"));
  routerApp.Install(rtr105).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-022"));
  routerApp.Install(rtr110).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-023"));
  routerApp.Install(rtr115).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-024"));
  routerApp.Install(rtr120).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-025"));
  routerApp.Install(rtr125).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-026"));
  routerApp.Install(rtr130).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-027"));
  routerApp.Install(rtr135).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-028"));
  routerApp.Install(rtr140).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-029"));
  routerApp.Install(rtr145).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-030"));
  routerApp.Install(rtr150).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-031"));
  routerApp.Install(rtr155).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-032"));
  routerApp.Install(rtr160).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-033"));
  routerApp.Install(rtr165).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-034"));
  routerApp.Install(rtr170).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-035"));
  routerApp.Install(rtr175).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-036"));
  routerApp.Install(rtr180).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-037"));
  routerApp.Install(rtr185).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-038"));
  routerApp.Install(rtr190).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-039"));
  routerApp.Install(rtr195).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-040"));
  routerApp.Install(rtr200).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-041"));
  routerApp.Install(rtr205).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-042"));
  routerApp.Install(rtr210).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-043"));
  routerApp.Install(rtr215).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-044"));
  routerApp.Install(rtr220).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-045"));
  routerApp.Install(rtr225).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-046"));
  routerApp.Install(rtr230).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-047"));
  routerApp.Install(rtr235).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-048"));
  routerApp.Install(rtr240).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-049"));
  routerApp.Install(rtr245).Start(Seconds(0));
  routerApp.SetAttribute("Service", StringValue("serviceCPM-050"));
  routerApp.Install(rtr250).Start(Seconds(0));

  ndn::AppHelper orchestratorApp("DagOrchestratorA_App");
  orchestratorApp.SetPrefix(Prefix);
  orchestratorApp.SetAttribute("Service", StringValue("serviceOrchestration"));
  //orchestratorApp.Install(orchestrator).Start(Seconds(0));
  orchestratorApp.Install(consumer).Start(Seconds(0));

  // Custom App for User(Consumer)
  ndn::AppHelper userApp("CustomAppConsumer");
  userApp.SetPrefix(Prefix);
  userApp.SetAttribute("Service", StringValue("consumer"));
  userApp.SetAttribute("Workflow", StringValue("workflows/cpm-50_x.json"));
  userApp.SetAttribute("Orchestrate", UintegerValue(1));
  userApp.Install(consumer).Start(Seconds(0));



  Simulator::Stop(Seconds(40.1));


  Simulator::Run();
  Simulator::Destroy();

  return 0;
}

} // namespace ns3

int
main(int argc, char* argv[])
{
  return ns3::main(argc, argv);
}
