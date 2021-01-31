#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/config-store-module.h"
#include "ns3/netanim-module.h"
#include "math.h"

using namespace ns3;

int main(int argc, char *argv[])
{
//define related params
  //ue number
  uint16_t numberOfUes = 1;
  //enb number
  uint16_t numberOfEnbs = 16;
  //packet trans interval/ms
  double interPacketInterval = 100;
  //length of time
  double simTime = 10;
  //moving speed of ue
  double movingSpeedOfUes = 500;

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

  //tx power of the enbs
  //all enb have same tx power
  double enbTxPowerDbm = 46.0;
  //noiseFigure of the enbs
  double noiseFigure = 5.0;
  //macToChannelDelay of enbs
  uint16_t macToChannelDelay = 2;
  // Command line arguments
  CommandLine cmd;
  cmd.AddValue("simTime", "Total duration of the simulation /seconds", simTime);
  cmd.AddValue("numberOfEnbs", "Number of the enbs", numberOfEnbs);
  cmd.AddValue("movingSpeedOfUes", "Moving speed of the ues", movingSpeedOfUes);
  cmd.AddValue("enbTxPowerDbm", "TX power  used by HeNBs /dbm", enbTxPowerDbm);
  cmd.AddValue("handoverAlgo", "type of  handover algorithm(A3 orA2A4)", handoverAlgo);
  cmd.AddValue("hysteresis", "hysteresis of A3 algorithm", hysteresis);
  cmd.AddValue("timeToTrigger", "timeToTrigger of A3 algorithm", timeToTrigger);
  cmd.AddValue("neighbourCellOffset", "neighbourCellOffset of A2A4 algorithm", neighbourCellOffset);
  cmd.AddValue("servingCellThreshold", "servingCellThreshold of A2A4 algorithm", servingCellThreshold);
  cmd.AddValue("noiseFigure", "Loss (dB) in the Signal-to-Noise-Ratio due to  non-idealities in the receiver", noiseFigure);
  cmd.AddValue("macToChannelDelay", "The delay in TTI units that occurs between   a scheduling decision in the MAC and the actual  start of the transmission by the PHY", macToChannelDelay);

  // cmd.AddValue("", "", );
  // cmd.AddValue("", "", );
  cmd.Parse(argc, argv);
  //any other attributes needed can be config like above
  //and make it work like below 

//attributes and their description and default value can be found in 
//   /src/lte/model/a2-a4-rsrq-handover-algorithm.cc
//   /src/lte/model/a3-rsrq-handover-algorithm.cc
//  /src/lte/model/lte-enb-phy.cc
//  /src/lte/model/lte-enb-mac.cc
//  /src/lte/model/lte-ue-phy.cc
//  /src/lte/model/lte-ue-mac.cc
//  /src/lte/model/lte-ue-power-control.cc
//and so on 

  // change some default attributes
  Config::SetDefault("ns3::UdpClient::Interval", TimeValue(MilliSeconds(10)));
  Config::SetDefault("ns3::UdpClient::MaxPackets", UintegerValue(1000000));
  Config::SetDefault("ns3::LteHelper::UseIdealRrc", BooleanValue(true));
  //change some physic attribute about lte
  Config::SetDefault("ns3::LteEnbPhy::TxPower", DoubleValue(enbTxPowerDbm));
  Config::SetDefault("ns3::LteEnbPhy::NoiseFigure", DoubleValue(noiseFigure));
  Config::SetDefault("ns3::LteEnbPhy:: MacToChannelDelay", UintegerValue(macToChannelDelay));
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

  //create ue and enb nodes
  NodeContainer ueNodes;
  NodeContainer enbNodes;
  enbNodes.Create(numberOfEnbs);
  ueNodes.Create(numberOfUes);

  //lym:dynamicly adjust moving range
  int sqrtResult = (int)sqrt((double)numberOfEnbs);
  if (sqrtResult * sqrtResult != numberOfEnbs)
    sqrtResult = sqrtResult + 1;
  double xRange = (double)sqrtResult * 500;
  double yRange = (double)sqrtResult * 500;

  // set position of enbs
  // lym:change the layout of enbs
  MobilityHelper enbMobility;
  enbMobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  enbMobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                   "LayoutType", StringValue("RowFirst"),
                                   "MinX", DoubleValue(0.0),
                                   "MinY", DoubleValue(0.0),
                                   "DeltaX", DoubleValue(500),
                                   "DeltaY", DoubleValue(500),
                                   "GridWidth", UintegerValue(sqrtResult));
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
  //lym:random position initialize
  MobilityHelper ueMobility;

  ueMobility.SetPositionAllocator("ns3::RandomDiscPositionAllocator",
                                  "X", StringValue("500.0"),
                                  "Y", StringValue("500.0"),
                                  "Rho", StringValue("ns3::UniformRandomVariable[Min=200.0|Max=500.0]"),
                                  "Theta", StringValue("ns3::UniformRandomVariable[Min=0.0|Max=6.2830]"));
  // ueMobility.SetMobilityModel("ns3::WaypointMobilityModel");
  //lym:random move model
  //lym:modifiable moving speed
  char speedStr[100];
  sprintf(speedStr, "ns3::UniformRandomVariable[Min=%f|Max=%f]", movingSpeedOfUes, movingSpeedOfUes);
  ueMobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                              "Mode", EnumValue(RandomWalk2dMobilityModel::MODE_TIME),
                              //  "Distance", DoubleValue (300.0),
                              "Time", TimeValue(Seconds(6.0)),
                              "Direction", StringValue("ns3::UniformRandomVariable[Min=0.0|Max=30.28]"),
                              "Speed", StringValue(speedStr),
                              "Bounds", RectangleValue(Rectangle(0.0, xRange, 0.0, yRange)));
  ueMobility.Install(ueNodes);

  // Install LTE Devices in eNB and UEs
  Config::SetDefault("ns3::LteEnbPhy::TxPower", DoubleValue(enbTxPowerDbm));
  NetDeviceContainer enbLteDevs = lteHelper->InstallEnbDevice(enbNodes);
  NetDeviceContainer ueLteDevs = lteHelper->InstallUeDevice(ueNodes);

  // Install the IP stack on the UEs
  internet.Install(ueNodes);
  Ipv4InterfaceContainer ueIpIfaces;
  ueIpIfaces = epcHelper->AssignUeIpv4Address(NetDeviceContainer(ueLteDevs));

  // Create the Internet
  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute("DataRate", DataRateValue(DataRate("100Gb/s")));
  p2ph.SetDeviceAttribute("Mtu", UintegerValue(1500));
  p2ph.SetChannelAttribute("Delay", TimeValue(Seconds(0.010)));

  //let ue connect with remotehost
  //!not sure  link ue and remotehost or link pgw and remotehost  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // NetDeviceContainer internetDevices = p2ph.Install(pgw, remoteHost);
  NetDeviceContainer internetDevices = p2ph.Install(ueNodes.Get(0), remoteHost);
  Ipv4AddressHelper ipv4h;
  ipv4h.SetBase("1.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign(internetDevices);
  Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress(1);

  // Routing of the Internet Host (towards the LTE network)
  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting(remoteHost->GetObject<Ipv4>());
  // interface 0 is localhost, 1 is the p2p device
  remoteHostStaticRouting->AddNetworkRouteTo(Ipv4Address("7.0.0.0"), Ipv4Mask("255.0.0.0"), 1);

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

  //enable some traces
  lteHelper->EnablePhyTraces();
  lteHelper->EnableMacTraces();
  lteHelper->EnableRlcTraces();
  lteHelper->EnablePdcpTraces();

  //enable  pcap  traces save the data in /IPdata
  //should create the folder first
  //p2ph.EnablePcapAll("IPdata/group20");

  // TASK2 ENABLE PCAP TRACING
  // related pcap files will be saved in /IPdata
  p2ph.EnablePcap("IPdata/prefix", internetDevices);
  p2ph.EnablePcap("IPdata/prefix", enbLteDevs);
  p2ph.EnablePcap("IPdata/prefix", ueLteDevs);

  //not known yet
  Ptr<RadioBearerStatsCalculator> rlcStats = lteHelper->GetRlcStats();
  rlcStats->SetAttribute("EpochDuration", TimeValue(Seconds(1.0)));
  Ptr<RadioBearerStatsCalculator> pdcpStats = lteHelper->GetPdcpStats();
  pdcpStats->SetAttribute("EpochDuration", TimeValue(Seconds(1.0)));

  //animation
  //set the xml file name
  AnimationInterface anim("group20.xml");
 
  //Tower img resource
  uint32_t resourceTower;
  //Phone imig resource
  uint32_t resourcePhone;
   //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!   img url should use absolute path   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  resourceTower = anim.AddResource("/home/matrix/ns3/ns-allinone-3.28/ns-3.28/examples/tutorial/img/tower.png");
  resourcePhone = anim.AddResource("/home/matrix/ns3/ns-allinone-3.28/ns-3.28/examples/tutorial/img/phone.png");
  //set some attributes
  anim.UpdateNodeDescription(remoteHost, "Host");
  anim.UpdateNodeColor(remoteHost, 255, 0, 0);
  anim.UpdateNodeDescription(pgw, "LTE-PGW");
  anim.UpdateNodeColor(pgw, 255, 0, 0);

  //lym:sfz:update tower img
  for (int i = 2; i < 2 + numberOfEnbs; i++)
  {
    anim.UpdateNodeImage(i, resourceTower);
    anim.UpdateNodeSize(i, 200, 200);
  }
  //lym:sfz:update phone img
  for (int i = 2 + numberOfEnbs; i < 2 + numberOfEnbs + numberOfUes; i++)
  {
    anim.UpdateNodeImage(i, resourcePhone);
    anim.UpdateNodeSize(i, 100, 100);
  }

  Simulator::Stop(Seconds(simTime));
  Simulator::Run();
  Simulator::Destroy();
  return 0;
}
