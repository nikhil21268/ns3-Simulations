#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/packet-sink.h"
#include <iostream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("WifiDownloadSimulation");

std::map<Ipv4Address, double> startTime;
std::map<Ipv4Address, double> endTime;
std::map<Ipv4Address, uint64_t> bytesReceived;


void TotalRx(Ptr<OutputStreamWrapper> stream, Ipv4Address clientAddress, Ptr<const Packet> packet, const Address &from)
{
    double now = Simulator::Now().GetSeconds();
    std::cout << "TotalRx called for client " << clientAddress << " at time " << now << std::endl;

    // Initialize start time and bytes received for this client
    if (startTime.find(clientAddress) == startTime.end()) {
        startTime[clientAddress] = now;
        bytesReceived[clientAddress] = 0;
    }

    // Update cumulative bytes received
    bytesReceived[clientAddress] += packet->GetSize();

    // Expected total bytes (5 MB)
    uint64_t expectedBytes = 100;

    // Check if the client has completed the download
    if (bytesReceived[clientAddress] >= expectedBytes && endTime.find(clientAddress) == endTime.end()) {
        endTime[clientAddress] = now;  // Record completion time

        double completionTime = endTime[clientAddress] - startTime[clientAddress];

        // Output the completion time for this client
        *stream->GetStream() << "Client " << clientAddress << " completion time: " << completionTime << " seconds" << std::endl;
    }
}



int main (int argc, char *argv[])
{

	LogComponentEnable("WifiHelper", LOG_LEVEL_ALL);
	// Example to enable basic logging
	LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
	LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);
	LogComponentEnable("WifiDownloadSimulation", LOG_LEVEL_INFO);
	LogComponentEnable("OnOffApplication", LOG_LEVEL_INFO);
	LogComponentEnable("PacketSink", LOG_LEVEL_INFO);


	uint32_t numClients = 10; // Number of WiFi clients
	uint16_t port = 50000;        // Port number for applications
	double simulationTime = 1000; // Simulation time in seconds
	double simulationEndTime = simulationTime + 1;

	NodeContainer wifiClients;
	wifiClients.Create(numClients);
	NodeContainer wifiApNode;
	wifiApNode.Create(1);
	NodeContainer serverNode;
	serverNode.Create(1);

	// Setup mobility
	MobilityHelper mobility;
	mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
	
	mobility.Install(wifiClients);
	mobility.Install(wifiApNode);
	mobility.Install(serverNode);

	for (uint32_t i = 0; i < wifiClients.GetN(); ++i) {
		Ptr<Node> node = wifiClients.Get(i);
		Ptr<ConstantPositionMobilityModel> mob = node->GetObject<ConstantPositionMobilityModel>();
		mob->SetPosition(Vector(i * 5.0, 0.0, 0.0)); // Space out clients linearly
	}

	// Create P2P link
	PointToPointHelper pointToPoint;
	pointToPoint.SetDeviceAttribute("DataRate", StringValue("1000Mbps"));
	pointToPoint.SetChannelAttribute("Delay", StringValue("100ms"));

	NetDeviceContainer p2pDevices;
	p2pDevices = pointToPoint.Install(wifiApNode.Get(0), serverNode.Get(0));

	// Configure WiFi
	YansWifiChannelHelper channel = YansWifiChannelHelper();
	channel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
    channel.AddPropagationLoss("ns3::LogDistancePropagationLossModel");

	YansWifiPhyHelper phy = YansWifiPhyHelper();
	phy.SetChannel(channel.Create());

	WifiHelper wifi;
	wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode", StringValue("HtMcs7"), "ControlMode", StringValue("HtMcs0"));
	
	WifiMacHelper mac;
	Ssid ssid = Ssid("NS3-WIFI");
	
	mac.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid));
	
	NetDeviceContainer apDevices = wifi.Install(phy, mac, wifiApNode);

	mac.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid), "ActiveProbing", BooleanValue(false));
	
	NetDeviceContainer clientDevices = wifi.Install(phy, mac, wifiClients);

	// Install Internet stack
	InternetStackHelper stack;
	stack.Install(wifiApNode);
	stack.Install(wifiClients);
	stack.Install(serverNode);

	Ipv4AddressHelper address;

	address.SetBase("10.1.1.0", "255.255.255.0");
	Ipv4InterfaceContainer p2pInterfaces = address.Assign(p2pDevices);
	
	address.SetBase("10.1.2.0", "255.255.255.0");
	address.Assign(apDevices);
	Ipv4InterfaceContainer clientInterfaces = address.Assign(clientDevices);

	Ipv4GlobalRoutingHelper::PopulateRoutingTables();

	std::cout << "Server IP address: " << p2pInterfaces.GetAddress(0) << std::endl;

	
	// Server application
	BulkSendHelper bulkSend("ns3::TcpSocketFactory", Address());
	bulkSend.SetAttribute("MaxBytes", UintegerValue(5242880));


	ApplicationContainer sourceApps;
	for (uint32_t i = 0; i < clientInterfaces.GetN(); ++i) {
		// Create a new BulkSendHelper for each client
		BulkSendHelper bulkSend("ns3::TcpSocketFactory", Address());
		bulkSend.SetAttribute("MaxBytes", UintegerValue(5242880));
		bulkSend.SetAttribute("Remote", AddressValue(InetSocketAddress(clientInterfaces.GetAddress(i), port)));

		ApplicationContainer tempApps = bulkSend.Install(serverNode.Get(0));
		tempApps.Start(Seconds(1.0));
		tempApps.Stop(Seconds(simulationTime));
		sourceApps.Add(tempApps);
	}


	// Client application
	PacketSinkHelper packetSinkHelper("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port));
	ApplicationContainer sinkApps = packetSinkHelper.Install(wifiClients);
	sinkApps.Start(Seconds(1.0));
	sinkApps.Stop(Seconds(simulationTime));


	AsciiTraceHelper ascii;
	Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream("wifi-download-completion-times.tr");
	if (stream->GetStream()->fail()) {
		std::cerr << "Failed to open file for writing." << std::endl;
		return -1; // or handle error differently
	}
	for (uint32_t i = 0; i < sinkApps.GetN(); ++i)
	{
		Ptr<Application> app = sinkApps.Get(i);
		Ipv4Address clientAddress = clientInterfaces.GetAddress(i);

		app->TraceConnectWithoutContext("Rx", MakeBoundCallback(&TotalRx, stream, clientAddress));
	}

	Simulator::Stop(Seconds(simulationEndTime));
	Simulator::Run();
	Simulator::Destroy();

	// Check total bytes received by each client
    for (uint32_t i = 0; i < sinkApps.GetN(); ++i) {
        Ptr<PacketSink> sink = DynamicCast<PacketSink>(sinkApps.Get(i));
        std::cout << "Client " << clientInterfaces.GetAddress(i) << " total bytes received: " << sink->GetTotalRx() << std::endl;
    }

	return 0;
}