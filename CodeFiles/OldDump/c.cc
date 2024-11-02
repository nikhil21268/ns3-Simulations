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
#include <map>
#include <string>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("WifiDownloadUploadSimulation");

std::map<Ipv4Address, double> startTime;
std::map<Ipv4Address, double> endTime;

void TotalRx(Ptr<const Packet> packet, const Address &from, Ptr<OutputStreamWrapper> stream)
{
    std::cout << "Callback triggered at time: " << Simulator::Now().GetSeconds() << std::endl;
    Ipv4Address srcAddr = InetSocketAddress::ConvertFrom(from).GetIpv4();
    double now = Simulator::Now().GetSeconds();

    if (startTime.find(srcAddr) == startTime.end()) {
        startTime[srcAddr] = now;  // Record start time
    }
    endTime[srcAddr] = now;  // Always update to the last packet's time

    *stream->GetStream() << now << "\t" << packet->GetSize() << std::endl;
}


int main (int argc, char *argv[])
{
    uint32_t numClients = 10;
    uint16_t port = 50000;
    double simulationTime = 100; // Simulation time in seconds

    NodeContainer wifiClients;
    wifiClients.Create(numClients);
    NodeContainer wifiApNode;
    wifiApNode.Create(1);
    NodeContainer serverNode;
    serverNode.Create(1);

    MobilityHelper mobility;
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(wifiClients);
    mobility.Install(wifiApNode);
    mobility.Install(serverNode);

    YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
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

    InternetStackHelper stack;
    stack.Install(wifiApNode);
    stack.Install(wifiClients);
    stack.Install(serverNode);

    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer p2pInterfaces = address.Assign(apDevices);
    address.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer clientInterfaces = address.Assign(clientDevices);

    BulkSendHelper bulkSend("ns3::TcpSocketFactory", Address());
    bulkSend.SetAttribute("MaxBytes", UintegerValue(5000000));
    ApplicationContainer sourceApps;
    for (uint32_t i = 0; i < clientInterfaces.GetN(); ++i) {
        bulkSend.SetAttribute("Remote", AddressValue(InetSocketAddress(clientInterfaces.GetAddress(i), port)));
        ApplicationContainer tempApps = bulkSend.Install(serverNode.Get(0));
        tempApps.Start(Seconds(1.0));
        tempApps.Stop(Seconds(simulationTime));
        sourceApps.Add(tempApps);
    }

    PacketSinkHelper packetSinkHelper("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port));
    ApplicationContainer sinkApps = packetSinkHelper.Install(wifiClients);
    sinkApps.Start(Seconds(1.0));
    sinkApps.Stop(Seconds(simulationTime));

    // Setting up the upload application using OnOffHelper
    OnOffHelper onOff("ns3::UdpSocketFactory", Address(InetSocketAddress(Ipv4Address::GetAny(), port)));
    onOff.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
    onOff.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
    onOff.SetAttribute("DataRate", DataRateValue(DataRate("150Kbps"))); // Average of 100-200 Kbps
    onOff.SetAttribute("PacketSize", UintegerValue(200)); // Average packet size in bytes

    ApplicationContainer clientUploadApps;
    for (uint32_t i = 0; i < clientInterfaces.GetN(); ++i) {
        AddressValue remoteAddress(InetSocketAddress(p2pInterfaces.GetAddress(0), port + 10 + i)); // Different port for each client to avoid collision
        onOff.SetAttribute("Remote", remoteAddress);
        ApplicationContainer tempApps = onOff.Install(wifiClients.Get(i));
        tempApps.Start(Seconds(1.0));
        tempApps.Stop(Seconds(simulationTime));
        clientUploadApps.Add(tempApps);
    }

    AsciiTraceHelper ascii;
    Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream("wifi-download-completion-times.tr");
    for (uint32_t i = 0; i < sinkApps.GetN(); ++i) {
        Ptr<Application> app = sinkApps.Get(i);
        app->TraceConnectWithoutContext("Rx", MakeBoundCallback(&TotalRx, stream));
    }

    Simulator::Stop(Seconds(simulationTime));
    Simulator::Run();
    Simulator::Destroy();

    // Calculate and print download completion times for each client
    for (auto& pair : endTime) {
        std::cout << "Client " << pair.first << " completed download in " << pair.second - startTime[pair.first] << " seconds." << std::endl;
    }

    return 0;
}
