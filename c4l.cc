#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/csma-module.h"
#include "ns3/mobility-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("P2P-CSMA");

int main() {
	LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);

	NodeContainer p2pNodes;
	p2pNodes.Create(1);

	NodeContainer csmaNodes;
	csmaNodes.Create(4);

	NodeContainer bridgeNode;
	bridgeNode.Create(1);

	PointToPointHelper pointToPoint;
	pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
	pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));

	NodeContainer p2pDevices;
	p2pDevices.Add(p2pNodes.Get(0));
	p2pDevices.Add(bridgeNode.Get(0));

	NetDeviceContainer p2pDevice;
	p2pDevice = pointToPoint.Install(p2pDevices);

	CsmaHelper csma;
	csma.SetChannelAttribute("DataRate", StringValue("100Mbps"));
	csma.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6500)));

	NodeContainer csmaDevices;
	csmaDevices.Add(bridgeNode.Get(0));
	csmaDevices.Add(csmaNodes);

	NetDeviceContainer csmaDevice;
	csmaDevice = csma.Install(csmaDevices);

	InternetStackHelper stack;
	stack.Install(p2pNodes);
	stack.Install(csmaNodes);
	stack.Install(bridgeNode);

	Ipv4AddressHelper address;

	address.SetBase("10.1.1.0", "255.255.255.0");
	Ipv4InterfaceContainer p2pInterface = address.Assign(p2pDevice);

	address.SetBase("10.1.2.0", "255.255.255.0");
	Ipv4InterfaceContainer csmaInterface = address.Assign(csmaDevice);

	UdpEchoServerHelper echoServer(9);
	ApplicationContainer serverApps = echoServer.Install(csmaNodes.Get(0)); 
	serverApps.Start(Seconds(1.0));
	serverApps.Stop(Seconds(10.0));

	UdpEchoClientHelper echoClient(csmaInterface.GetAddress(1), 9);
	echoClient.SetAttribute("PacketSize", UintegerValue(1024));
	echoClient.SetAttribute("MaxPackets", UintegerValue(10));
	echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));

	ApplicationContainer clientApps = echoClient.Install(p2pDevices.Get(0));
	clientApps.Start(Seconds(2.0));
	clientApps.Stop(Seconds(10.0));

	Ipv4GlobalRoutingHelper::PopulateRoutingTables();

	AsciiTraceHelper ascii;
	pointToPoint.EnableAsciiAll(ascii.CreateFileStream("p2p-trace.tr"));
	csma.EnableAsciiAll(ascii.CreateFileStream("csma-trace.tr"));

	pointToPoint.EnablePcapAll("p2p");
	csma.EnablePcapAll("csma");

	AnimationInterface anim("p2p-csma-four-nodes.xml");
	anim.SetConstantPosition(p2pNodes.Get(0), 0, 50);
	anim.SetConstantPosition(bridgeNode.Get(0), 20, 50);

	for(uint32_t i = 0; i < csmaNodes.GetN(); i++) {
		anim.SetConstantPosition(csmaNodes.Get(i), 40 + i*20, 50);
	}

	Simulator::Stop(Seconds(10.0));
	Simulator::Run();
	Simulator::Destroy();

	return 0;
}
