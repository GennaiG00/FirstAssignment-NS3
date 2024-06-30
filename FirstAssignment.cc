#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/flow-monitor-module.h"

                 /*
Network Topology
                   ----------------
                
                   10.1.1.0 Network      10.1.2.0 Network
                  +---------------+      +---------------+
                  |               |      |               |
                  |   Node 0      |      |   Node 2      |
                  | (Client A)    |      | (Client B)    |
                  +---------------+      +---------------+
                         |                       |
                   Point-to-Point Link     Point-to-Point Link
                   DataRate: 5Mbps         DataRate: 5Mbps
                   Delay: 2ms              Delay: 2ms
                         |                       |
                  +---------------+      +---------------+
                  |               |      |               |
                  |   Node 1      |      |   Node 3      |
                  | (Server A)    |      | (Server B)    |
                  +---------------+      +---------------+
*/

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("FirstAssignment");

int main(int argc, char* argv[])
{
    std::cout << "----------------------------------" << std::endl;

    bool useTcp = true; // Option to choose between TCP and UDP
    bool verbose = false; // Option to enable verbose logging
    uint32_t maxPackets = 10;
    Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (1024)); // Set packet size to 1024 bytes

    CommandLine cmd(__FILE__);
    cmd.AddValue("useTcp", "Use TCP if true, UDP if false", useTcp);
    cmd.AddValue("verbose", "Tell echo applications to log if true", verbose);

    cmd.Parse(argc, argv);

    Time::SetResolution(Time::NS);
    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);

    NodeContainer nodesOne;
    nodesOne.Create(2);

    PointToPointHelper pointToPointA;
    pointToPointA.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    pointToPointA.SetChannelAttribute("Delay", StringValue("2ms"));

    NetDeviceContainer devicesA, devicesB;
    devicesA = pointToPointA.Install(nodesOne);

    InternetStackHelper stackA;
    stackA.Install(nodesOne);

    NodeContainer nodesTwo;
    nodesTwo.Create(2);

    PointToPointHelper pointToPointB;
    pointToPointB.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    pointToPointB.SetChannelAttribute("Delay", StringValue("2ms"));

    devicesB = pointToPointB.Install(nodesTwo);

    InternetStackHelper stackB;
    stackB.Install(nodesTwo);

    Ipv4AddressHelper addressOne;
    addressOne.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfacesA = addressOne.Assign(devicesA);

    Ipv4AddressHelper addressTwo;
    addressTwo.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer interfacesB = addressTwo.Assign(devicesB);

    ApplicationContainer serverAppsA, serverAppsB, clientAppsA, clientAppsB;

    if (!useTcp)
    {
        std::cout << "Using UDP\n" << std::endl;
        // UDP configuration
        UdpEchoServerHelper echoServerA(9);
        serverAppsA = echoServerA.Install(nodesOne.Get(1));
        serverAppsA.Start(Seconds(1.0));
        serverAppsA.Stop(Seconds(12.0));

        UdpEchoServerHelper echoServerB(10);
        serverAppsB = echoServerB.Install(nodesTwo.Get(1));
        serverAppsB.Start(Seconds(1.0));
        serverAppsB.Stop(Seconds(12.0));

        UdpEchoClientHelper echoClientA(interfacesA.GetAddress(1), 9);
        echoClientA.SetAttribute("MaxPackets", UintegerValue(maxPackets));
        echoClientA.SetAttribute("Interval", TimeValue(Seconds(.5)));
        echoClientA.SetAttribute("PacketSize", UintegerValue(1024));
        clientAppsA = echoClientA.Install(nodesOne.Get(0));
        clientAppsA.Start(Seconds(2.0));
        clientAppsA.Stop(Seconds(12.0));

        UdpEchoClientHelper echoClientB(interfacesB.GetAddress(1), 10);
        echoClientB.SetAttribute("MaxPackets", UintegerValue(maxPackets));
        echoClientB.SetAttribute("Interval", TimeValue(Seconds(.5)));
        echoClientB.SetAttribute("PacketSize", UintegerValue(1024));
        clientAppsB = echoClientB.Install(nodesTwo.Get(0));
        clientAppsB.Start(Seconds(2.0));
        clientAppsB.Stop(Seconds(12.0));

        if (verbose)
        {
            LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
            LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);

        }
    }
    else
    {
        std::cout << "Using TCP\n" << std::endl;
        // TCP configuration
        uint16_t portA = 8080;
        uint16_t portB = 8081;
        uint32_t packetSize = 1024; // Packet size in bytes
        uint32_t maxBytes = maxPackets * packetSize; // Total bytes to send

        Address serverAddressA(InetSocketAddress(interfacesA.GetAddress(1), portA));
        PacketSinkHelper packetSinkHelperA("ns3::TcpSocketFactory", serverAddressA);
        serverAppsA = packetSinkHelperA.Install(nodesOne.Get(1));
        serverAppsA.Start(Seconds(1.0));
        serverAppsA.Stop(Seconds(12.0));

        Address serverAddressB(InetSocketAddress(interfacesB.GetAddress(1), portB));
        PacketSinkHelper packetSinkHelperB("ns3::TcpSocketFactory", serverAddressB);
        serverAppsB = packetSinkHelperB.Install(nodesTwo.Get(1));
        serverAppsB.Start(Seconds(1.0));
        serverAppsB.Stop(Seconds(12.0));

        BulkSendHelper clientHelperA("ns3::TcpSocketFactory", serverAddressA);
        clientHelperA.SetAttribute("MaxBytes", UintegerValue(maxBytes));
        clientHelperA.SetAttribute("SendSize", UintegerValue(packetSize));
        clientAppsA = clientHelperA.Install(nodesOne.Get(0));
        clientAppsA.Start(Seconds(2.0));
        clientAppsA.Stop(Seconds(12.0));

        BulkSendHelper clientHelperB("ns3::TcpSocketFactory", serverAddressB);
        clientHelperB.SetAttribute("MaxBytes", UintegerValue(maxBytes));
        clientHelperB.SetAttribute("SendSize", UintegerValue(packetSize));
        clientAppsB = clientHelperB.Install(nodesTwo.Get(0));
        clientAppsB.Start(Seconds(2.0));
        clientAppsB.Stop(Seconds(12.0));
        if (verbose)
        {
            LogComponentEnable("TcpSocketBase", LOG_LEVEL_INFO);
        }
    }



    Ptr<FlowMonitor> flowMonitor;
    FlowMonitorHelper flowHelper;
    flowMonitor = flowHelper.InstallAll();

    Simulator::Stop(Seconds(10.0));
    Simulator::Run();

    flowMonitor->CheckForLostPackets();
    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(flowHelper.GetClassifier());
    std::map<FlowId, FlowMonitor::FlowStats> stats = flowMonitor->GetFlowStats();

    for (auto i = stats.begin(); i != stats.end(); ++i)
    {
        Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(i->first);
        std::cout << "Flow ID: " << i->first << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
        std::cout << "Tx Packets: " << i->second.txPackets << "\n";
        std::cout << "Rx Packets: " << i->second.rxPackets << "\n";
        std::cout << "Tx Bytes: " << i->second.txBytes << "\n";
        std::cout << "Rx Bytes: " << i->second.rxBytes << "\n";
        std::cout << "Throughput: " << i->second.rxBytes * 8.0 / (i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds()) / 1024 << " Kbps\n";
        std::cout << "Delay: " << i->second.delaySum.GetSeconds() / i->second.rxPackets << " s\n";
        std::cout << "Lost Packets: " << i->second.lostPackets << "\n";
        std::cout << "-------------------------------\n";
    }

    Simulator::Destroy();
    return 0;
}
