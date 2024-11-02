#include "ns3/command-line.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

#include <iostream>
#include <vector>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("WifiNetworkExample");

// Struct to hold client data
struct ClientData
{
    Ptr<PacketSink> sink;
    uint32_t clientId;
    Time completionTime;
    bool completed;
};

std::vector<ClientData> clientDataList;

void CheckCompletion ()
{
    for (auto &clientData : clientDataList)
    {
        if (!clientData.completed)
        {
            uint64_t totalBytes = clientData.sink->GetTotalRx ();

            if (totalBytes >= 5 * 1024 * 1024) // 5MB
            {
                clientData.completionTime = Simulator::Now ();
                clientData.completed = true;

                std::cout << "Client " << clientData.clientId
                          << " completed at time " << clientData.completionTime.GetSeconds ()
                          << " seconds" << std::endl;
            }
        }
    }

    Simulator::Schedule (MilliSeconds (10), &CheckCompletion);
}

int main (int argc, char *argv[])
{
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
    */

    // Setup mobility for the clients
    /*
    MobilityHelper mobility;

    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();

    for (uint32_t i = 0; i < wifiClients.GetN (); ++i)
    {
        double distance = 5.0 + i * 5.0; // Increase distance by 5 meters for each client
        positionAlloc->Add (Vector (distance, 0.0, 0.0));
    }

    mobility.SetPositionAllocator (positionAlloc);
    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobility.Install (wifiClients);

    // Setup mobility for the AP
    Ptr<ListPositionAllocator> apPositionAlloc = CreateObject<ListPositionAllocator> ();
    apPositionAlloc->Add (Vector (0.0, 0.0, 0.0));

    mobility.SetPositionAllocator (apPositionAlloc);
    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    */

    // Setup mobility for the clients
    /*
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

     // (Optional) Setup mobility for the server node
    MobilityHelper mobilityServer;
    Ptr<ListPositionAllocator> positionAllocServer = CreateObject<ListPositionAllocator> ();
    positionAllocServer->Add (Vector (0.0, -20.0, 0.0)); // Position it below the AP
    mobilityServer.SetPositionAllocator (positionAllocServer);
    mobilityServer.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobilityServer.Install (serverNode);

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

    // Set up the channel with path loss and fading model
    YansWifiChannelHelper channel = YansWifiChannelHelper();
    channel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
    // channel.AddPropagationLoss("ns3::LogDistancePropagationLossModel");
    channel.AddPropagationLoss("ns3::LogDistancePropagationLossModel",
                           "Exponent", DoubleValue(4.0));
    // channel.AddPropagationLoss("ns3::NakagamiPropagationLossModel");
    channel.AddPropagationLoss("ns3::NakagamiPropagationLossModel",
                           "m0", DoubleValue(0.5),
                           "m1", DoubleValue(0.75),
                           "m2", DoubleValue(1.0));


    // Create a PHY helper
    YansWifiPhyHelper phy = YansWifiPhyHelper();
    phy.SetChannel(channel.Create());

    // For both part d and part e

    // phy.Set ("TxPowerStart", DoubleValue (20.0)); // Max transmit power
    // phy.Set ("TxPowerEnd", DoubleValue (20.0));
    // phy.Set ("RxSensitivity", DoubleValue (-90.0)); // Increase sensing range


    WifiHelper wifi;
    // Set MinstrelHT as the rate adaptation manager
    wifi.SetRemoteStationManager ("ns3::MinstrelHtWifiManager");

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
    Ipv4InterfaceContainer wifiApInterface = address.Assign (apDevices);
    Ipv4InterfaceContainer clientInterfaces = address.Assign (clientDevices);

    // Install PacketSink on each client for download
    ApplicationContainer clientApps;

    for (uint32_t i = 0; i < wifiClients.GetN (); ++i)
    {
        uint16_t port = 50000 + i;

        PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory",
                                           InetSocketAddress (Ipv4Address::GetAny (), port));

        ApplicationContainer app = packetSinkHelper.Install (wifiClients.Get (i));

        app.Start (Seconds (0.0));
        app.Stop (Seconds (20.0));

        clientApps.Add (app);

        Ptr<PacketSink> sink = DynamicCast<PacketSink> (app.Get (0));

        ClientData clientData;
        clientData.sink = sink;
        clientData.clientId = i;
        clientData.completionTime = Seconds (0.0);
        clientData.completed = false;

        clientDataList.push_back (clientData);
    }

    // Install BulkSendApplication on the server for each client (Download)
    ApplicationContainer serverApps;

    for (uint32_t i = 0; i < wifiClients.GetN (); ++i)
    {
        uint16_t port = 50000 + i;

        BulkSendHelper bulkSend ("ns3::TcpSocketFactory",
                                 InetSocketAddress (clientInterfaces.GetAddress (i), port));

        bulkSend.SetAttribute ("MaxBytes", UintegerValue (5 * 1024 * 1024)); // 5MB

        ApplicationContainer app = bulkSend.Install (serverNode.Get (0));

        app.Start (Seconds (1.0));
        app.Stop (Seconds (20.0));

        serverApps.Add (app);
    }

    // **Part d: Add Upload Application with path loss and fading model**
    /*
    // Install PacketSink on the server to receive uploads from clients (TCP)
    uint16_t uploadPort = 60000;
    PacketSinkHelper serverPacketSinkHelper ("ns3::TcpSocketFactory",
                                            InetSocketAddress (Ipv4Address::GetAny (), uploadPort));

    ApplicationContainer serverSinkApp = serverPacketSinkHelper.Install (serverNode.Get (0));
    serverSinkApp.Start (Seconds (0.0));
    serverSinkApp.Stop (Seconds (20.0));

    // Install BulkSendApplication on each client to upload data to the server (TCP)
    for (uint32_t i = 0; i < wifiClients.GetN (); ++i)
    {
        BulkSendHelper bulkSend ("ns3::TcpSocketFactory",
                                InetSocketAddress (p2pInterfaces.GetAddress (1), uploadPort));

        bulkSend.SetAttribute ("MaxBytes", UintegerValue (5 * 1024 * 1024)); // 5MB

        ApplicationContainer clientUploadApp = bulkSend.Install (wifiClients.Get (i));
        clientUploadApp.Start (Seconds (1.0));
        clientUploadApp.Stop (Seconds (20.0));
    }
    */

    // Install PacketSink on the server to receive uploads from clients (UDP)
    uint16_t uploadPort = 60000;
    PacketSinkHelper serverPacketSinkHelper ("ns3::UdpSocketFactory",
                                            InetSocketAddress (Ipv4Address::GetAny (), uploadPort));

    ApplicationContainer serverSinkApp = serverPacketSinkHelper.Install (serverNode.Get (0));
    serverSinkApp.Start (Seconds (0.0));
    serverSinkApp.Stop (Seconds (20.0));

    // Install OnOffApplication on each client to upload data to the server (UDP)
    for (uint32_t i = 0; i < wifiClients.GetN (); ++i)
    {
        OnOffHelper clientOnOff ("ns3::UdpSocketFactory",
                                InetSocketAddress (p2pInterfaces.GetAddress (1), uploadPort));

        // Set the data rate between 100 Kbps and 200 Kbps
        std::ostringstream rate;
        rate << (200 + (0 % 101)) << "Kbps"; // 100 Kbps to 200 Kbps
        clientOnOff.SetAttribute ("DataRate", StringValue (rate.str ()));
        clientOnOff.SetAttribute ("PacketSize", UintegerValue (10)); // 1KB packets
        // clientOnOff.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1000]")); // Long on time
        // clientOnOff.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));    // Zero off time

        ApplicationContainer clientUploadApp = clientOnOff.Install (wifiClients.Get (i));
        clientUploadApp.Start (Seconds (0.0));
        clientUploadApp.Stop (Seconds (20.0));
    }

    // Start checking for completion
    Simulator::Schedule (Seconds (1.1), &CheckCompletion);

    // Enable routing
    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

    Simulator::Stop (Seconds (20.0));
    Simulator::Run ();
    Simulator::Destroy ();
    return 0;
}