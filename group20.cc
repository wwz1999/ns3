#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/config-store-module.h"
#include "ns3/netanim-module.h"

using namespace ns3;

int main(int argc, char *argv[])
{

  //ue  amount
  uint16_t numberOfUes = 1;
  //enb amount
  uint16_t numberOfEnbs = 3;
  //packet trans interval/ms
  double interPacketInterval = 100;
  //length of simulate  time
  double simTime = 10;
  //enb power
  double enbTxPowerDbm = 46.0;

  //algorithm type
  std::string handoverAlgo = "A3";
  //attri of A3 algorithm
  double hysteresis = 1.0;
  //attri of A3 algorithm
  uint16_t timeToTrigger = 64;
  //attri of A2A4 algorithm
  uint16_t servingCellThreshold = 30;
  //attri of A2A4 algorithm
  uint16_t neighbourCellOffset = 1;

  // Command line arguments
  CommandLine cmd;
  cmd.AddValue("simTime", "Total duration of the simulation /seconds", simTime);
  cmd.AddValue("enbTxPowerDbm", "TX power  used by HeNBs /dbm", enbTxPowerDbm);
  cmd.AddValue("handoverAlgo", "type of  handover algorithm(A3 orA2A4)", handoverAlgo);
  cmd.AddValue("hysteresis", "hysteresis of A3 algorithm", hysteresis);
  cmd.AddValue("timeToTrigger", "timeToTrigger of A3 algorithm", timeToTrigger);
  cmd.AddValue("neighbourCellOffset", "neighbourCellOffset of A2A4 algorithm", neighbourCellOffset);
  cmd.AddValue("servingCellThreshold", "servingCellThreshold of A2A4 algorithm", servingCellThreshold);
  // cmd.AddValue("", "", );
  // cmd.AddValue("", "", );
  // cmd.AddValue("", "", );
  // cmd.AddValue("", "", );
  cmd.Parse(argc, argv);

  // change some default attributes
  Config::SetDefault("ns3::UdpClient::Interval", TimeValue(MilliSeconds(10)));
  Config::SetDefault("ns3::UdpClient::MaxPackets", UintegerValue(1000000));
  Config::SetDefault("ns3::LteHelper::UseIdealRrc", BooleanValue(true));
  //change some physic attribute about lte
  Config::SetDefault("ns3::LteEnbPhy::TxPower", DoubleValue(enbTxPowerDbm));
  //and so on

  Ptr<LteHelper> lteHelper = CreateObject<LteHelper>();
  Ptr<PointToPointEpcHelper> epcHelper = CreateObject<PointToPointEpcHelper>();
  lteHelper->SetEpcHelper(epcHelper);

  lteHelper->SetSchedulerType("ns3::RrFfMacScheduler");

  lteHelper->SetHandoverAlgorithmType("ns3::A3RsrpHandoverAlgorithm");
  //set some attribute of A3 algorithm
  lteHelper->SetHandoverAlgorithmAttribute("Hysteresis",
                                           DoubleValue(hysteresis));
  lteHelper->SetHandoverAlgorithmAttribute("TimeToTrigger",
                                           TimeValue(MilliSeconds(timeToTrigger)));
  if (handoverAlgo == "A2A4")
  {

    lteHelper->SetHandoverAlgorithmType("ns3::A2A4RsrqHandoverAlgorithm");
    lteHelper->SetHandoverAlgorithmAttribute("ServingCellThreshold",
                                             UintegerValue(servingCellThreshold));
    lteHelper->SetHandoverAlgorithmAttribute("NeighbourCellOffset",
                                             UintegerValue(neighbourCellOffset));
  }
  //get pgw node
  Ptr<Node> pgw = epcHelper->GetPgwNode();

  // Create a single RemoteHost
  NodeContainer remoteHostContainer;
  remoteHostContainer.Create(1);
  Ptr<Node> remoteHost = remoteHostContainer.Get(0);
  InternetStackHelper internet;
  internet.Install(remoteHostContainer);

  // Create the Internet
  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute("DataRate", DataRateValue(DataRate("100Gb/s")));
  p2ph.SetDeviceAttribute("Mtu", UintegerValue(1500));
  p2ph.SetChannelAttribute("Delay", TimeValue(Seconds(0.010)));

  NetDeviceContainer internetDevices = p2ph.Install(pgw, remoteHost);
  Ipv4AddressHelper ipv4h;
  ipv4h.SetBase("1.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign(internetDevices);
  Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress(1);

  // Routing of the Internet Host (towards the LTE network)
  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting(remoteHost->GetObject<Ipv4>());
  // interface 0 is localhost, 1 is the p2p device
  remoteHostStaticRouting->AddNetworkRouteTo(Ipv4Address("7.0.0.0"), Ipv4Mask("255.0.0.0"), 1);

  //create ue and enb nodes
  NodeContainer ueNodes;
  NodeContainer enbNodes;
  enbNodes.Create(numberOfEnbs);
  ueNodes.Create(numberOfUes);

  // set position of enbs
  MobilityHelper enbMobility;
  enbMobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  enbMobility.SetPositionAllocator("ns3::GridPositionAllocator", "MinX", DoubleValue(500.0), "MinY",
                                   DoubleValue(500.0), "DeltaX", DoubleValue(500));
  enbMobility.Install(enbNodes);

  //set position of pgw
  MobilityHelper pgwMobility;
  pgwMobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  pgwMobility.SetPositionAllocator("ns3::GridPositionAllocator", "MinX", DoubleValue(1100.0), "MinY",
                                   DoubleValue(1300.0));
  pgwMobility.Install(pgw);

  //set position of remote host
  MobilityHelper remoteHostMobility;
  remoteHostMobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  remoteHostMobility.SetPositionAllocator("ns3::GridPositionAllocator", "MinX", DoubleValue(1400.0), "MinY",
                                          DoubleValue(1300.0));
  remoteHostMobility.Install(remoteHostContainer);

  //make ue move
  MobilityHelper ueMobility;
  ueMobility.SetPositionAllocator(
      "ns3::GridPositionAllocator", "MinX", DoubleValue(10.0), "MinY",
      DoubleValue(10.0));
  ueMobility.SetMobilityModel("ns3::WaypointMobilityModel");
  ueMobility.Install(ueNodes);
  //set ue waypoints
  Ptr<WaypointMobilityModel> waypoint = ueNodes.Get(0)->GetObject<WaypointMobilityModel>();
  waypoint->AddWaypoint(Waypoint(Seconds(0.0), Vector(0, 400, 0.0)));
  waypoint->AddWaypoint(Waypoint(Seconds(4.0), Vector(2000, 400, 0.0)));
  waypoint->AddWaypoint(Waypoint(Seconds(6.0), Vector(2000, 700, 0.0)));
  waypoint->AddWaypoint(Waypoint(Seconds(9.0), Vector(0, 700, 0.0)));

  // Install LTE Devices in eNB and UEs
  NetDeviceContainer enbLteDevs = lteHelper->InstallEnbDevice(enbNodes);
  NetDeviceContainer ueLteDevs = lteHelper->InstallUeDevice(ueNodes);

  // Install the IP stack on the UEs
  internet.Install(ueNodes);
  Ipv4InterfaceContainer ueIpIfaces;
  ueIpIfaces = epcHelper->AssignUeIpv4Address(NetDeviceContainer(ueLteDevs));

  // Set the default gateway for the UE
  Ptr<Node> ue = ueNodes.Get(0);
  Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting(ue->GetObject<Ipv4>());
  ueStaticRouting->SetDefaultRoute(epcHelper->GetUeDefaultGatewayAddress(), 1);

  // Attach UE to the first eNodeB
  lteHelper->Attach(ueLteDevs.Get(0), enbLteDevs.Get(0));

  // Install and start applications on UE and remote host
  uint16_t dlPort = 10000;
  uint16_t ulPort = 20000;
  uint16_t otherPort = 3000;

  //install applications
  ApplicationContainer clientApps;
  ApplicationContainer serverApps;
  for (uint32_t u = 0; u < ueNodes.GetN(); ++u)
  {
    ++ulPort;
    ++otherPort;
    PacketSinkHelper dlPacketSinkHelper("ns3::UdpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), dlPort));
    PacketSinkHelper ulPacketSinkHelper("ns3::UdpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), ulPort));
    PacketSinkHelper packetSinkHelper("ns3::UdpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), otherPort));
    serverApps.Add(dlPacketSinkHelper.Install(ueNodes.Get(u)));
    serverApps.Add(ulPacketSinkHelper.Install(remoteHost));
    serverApps.Add(packetSinkHelper.Install(ueNodes.Get(u)));

    UdpClientHelper dlClient(ueIpIfaces.GetAddress(u), dlPort);
    dlClient.SetAttribute("Interval", TimeValue(MilliSeconds(interPacketInterval)));
    dlClient.SetAttribute("MaxPackets", UintegerValue(1000000));

    UdpClientHelper ulClient(remoteHostAddr, ulPort);
    ulClient.SetAttribute("Interval", TimeValue(MilliSeconds(interPacketInterval)));
    ulClient.SetAttribute("MaxPackets", UintegerValue(1000000));

    UdpClientHelper client(ueIpIfaces.GetAddress(u), otherPort);
    client.SetAttribute("Interval", TimeValue(MilliSeconds(interPacketInterval)));
    client.SetAttribute("MaxPackets", UintegerValue(1000000));

    clientApps.Add(dlClient.Install(remoteHost));
    clientApps.Add(ulClient.Install(ueNodes.Get(u)));
    if (u + 1 < ueNodes.GetN())
    {
      clientApps.Add(client.Install(ueNodes.Get(u + 1)));
    }
    else
    {
      clientApps.Add(client.Install(ueNodes.Get(0)));
    }
  } // end for b
  Time startTime = Seconds(0.01);
  serverApps.Start(startTime);
  clientApps.Start(startTime);
  // Add X2 inteface
  lteHelper->AddX2Interface(enbNodes);

  //enable traces
  lteHelper->EnablePhyTraces();
  lteHelper->EnableMacTraces();
  lteHelper->EnableRlcTraces();
  lteHelper->EnablePdcpTraces();
  //enable  pcap  traces
  p2ph.EnablePcapAll("group20");

  //not know yet
  Ptr<RadioBearerStatsCalculator> rlcStats = lteHelper->GetRlcStats();
  rlcStats->SetAttribute("EpochDuration", TimeValue(Seconds(1.0)));
  Ptr<RadioBearerStatsCalculator> pdcpStats = lteHelper->GetPdcpStats();
  pdcpStats->SetAttribute("EpochDuration", TimeValue(Seconds(1.0)));

  //animation
  //set the xml file name
  AnimationInterface anim("group20.xml");
  //img url
  //absolute path !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  //Tower img resource
  uint32_t resourceTower;
  //Phone imig resource
  uint32_t resourcePhone;
  resourceTower = anim.AddResource("/home/wwz/ns3/ns-allinone-3.28/ns-3.28/signalTower.png");
  resourcePhone = anim.AddResource("/home/wwz/ns3/ns-allinone-3.28/ns-3.28/iPhone.png");
  //set some attributes
  anim.UpdateNodeDescription(remoteHost, "Host");
  anim.UpdateNodeColor(remoteHost, 255, 0, 0);
  anim.UpdateNodeDescription(pgw, "LTE-PGW");
  anim.UpdateNodeColor(pgw, 255, 0, 0);
  //sfz:update tower img
  anim.UpdateNodeImage(2, resourceTower);
  anim.UpdateNodeImage(3, resourceTower);
  anim.UpdateNodeImage(4, resourceTower);
  //sfz:update phone img
  anim.UpdateNodeImage(5, resourcePhone);
  //sfz:update node size
  anim.UpdateNodeSize(2, 200, 200);
  anim.UpdateNodeSize(3, 200, 200);
  anim.UpdateNodeSize(4, 200, 200);
  anim.UpdateNodeSize(5, 100, 200);

  anim.UpdateNodeDescription(5, "Ue");
  anim.UpdateNodeDescription(2, "eNB");
  anim.UpdateNodeDescription(3, "eNB");
  anim.UpdateNodeDescription(4, "eNB");

  Simulator::Stop(Seconds(simTime));
  Simulator::Run();
  Simulator::Destroy();
  return 0;
}
