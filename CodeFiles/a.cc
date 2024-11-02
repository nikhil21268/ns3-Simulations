#include "ns3/command-line.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("WifiNetworkExample");

int main (int argc, char *argv[])
{
    LogComponentEnable("WifiHelper", LOG_LEVEL_ALL);
    // Example to enable basic logging
    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);

    uint32_t numClients = 5; // Specify the number of WiFi clients


    CommandLine cmd;
    cmd.AddValue ("numClients", "Number of WiFi clients", numClients);
    cmd.Parse (argc, argv);


    
    NodeContainer wifiClients;
    wifiClients.Create (numClients);
    NodeContainer wifiApNode;
    wifiApNode.Create (1);
    NodeContainer serverNode;
    serverNode.Create (1);  

    // Setup mobility for all nodes
    /*
    MobilityHelper mobility;
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(wifiClients);
    mobility.Install(wifiApNode);
    mobility.Install(serverNode);
    */
    
    /*
    // Setup mobility for the clients
    MobilityHelper mobility;

    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();

    // Set initial distance and increment
    double initialDistance = 5.0; // Adjust as desired
    double incrementDistance = 1.0; // Increase distance by 1 meter per client

    for (uint32_t i = 0; i < wifiClients.GetN (); ++i)
    {
        double distance = initialDistance + i * incrementDistance;
        positionAlloc->Add (Vector (distance, 0.0, 0.0));
    }

    mobility.SetPositionAllocator (positionAlloc);
    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobility.Install (wifiClients);

    // The rest of your code remains unchanged


    mobility.Install (wifiApNode);

    // No need to set position for the server node if it's connected via point-to-point link


    mobility.Install(serverNode);

    */

    MobilityHelper mobilityClients;

    Ptr<ListPositionAllocator> positionAllocClients = CreateObject<ListPositionAllocator> ();

    double radius = 5.0; // Adjust the radius as desired
    numClients = wifiClients.GetN();

    for (uint32_t i = 0; i < numClients; ++i)
    {
        double angle = i * (2.0 * M_PI / numClients);
        double x = radius * std::cos(angle);
        double y = radius * std::sin(angle);
        positionAllocClients->Add (Vector (x, y, 0.0));
    }

    mobilityClients.SetPositionAllocator (positionAllocClients);
    mobilityClients.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobilityClients.Install (wifiClients);

    // Setup mobility for the AP
    MobilityHelper mobilityAp;
    Ptr<ListPositionAllocator> positionAllocAp = CreateObject<ListPositionAllocator> ();
    positionAllocAp->Add (Vector (0.0, 0.0, 0.0)); // AP at origin
    mobilityAp.SetPositionAllocator (positionAllocAp);
    mobilityAp.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobilityAp.Install (wifiApNode);

    // Point-to-point link between AP and server
    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("1000Mbps"));
    pointToPoint.SetChannelAttribute ("Delay", StringValue ("100ms"));

    NetDeviceContainer p2pDevices;
    p2pDevices = pointToPoint.Install (wifiApNode.Get (0), serverNode.Get (0));

    // Set up the channel
    YansWifiChannelHelper channel = YansWifiChannelHelper();
    channel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
    channel.AddPropagationLoss("ns3::LogDistancePropagationLossModel");

    // Create a PHY helper
    YansWifiPhyHelper phy = YansWifiPhyHelper();
    phy.SetChannel(channel.Create());


    WifiHelper wifi;
    wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                              "DataMode", StringValue("HtMcs7"),
                              "ControlMode", StringValue("HtMcs0"));

    WifiMacHelper mac;
    Ssid ssid = Ssid ("ns3-wifi");

    // Setup the AP
    mac.SetType ("ns3::ApWifiMac",
                 "Ssid", SsidValue (ssid));

    NetDeviceContainer apDevices;
    apDevices = wifi.Install (phy, mac, wifiApNode);

    // Setup the clients
    mac.SetType ("ns3::StaWifiMac",
                 "Ssid", SsidValue (ssid),
                 "ActiveProbing", BooleanValue (false));

    NetDeviceContainer clientDevices;
    clientDevices = wifi.Install (phy, mac, wifiClients);

    InternetStackHelper stack;
    stack.Install (wifiApNode);
    stack.Install (wifiClients);
    stack.Install (serverNode);

    Ipv4AddressHelper address;

    // IP addresses for the p2p network
    address.SetBase ("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer p2pInterfaces;
    p2pInterfaces = address.Assign (p2pDevices);

    // IP addresses for the WiFi network
    address.SetBase ("10.1.2.0", "255.255.255.0");
    address.Assign (apDevices);
    address.Assign (clientDevices);

    Simulator::Stop (Seconds (10.0));
    Simulator::Run ();
    Simulator::Destroy ();
    return 0;
}
