#include <iostream>
#include <cmath>
#include <random>
#include "ns3/netanim-module.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/traffic-control-module.h"

using namespace std;
using namespace ns3;

#define LINK_CAPACITY 1
#define TRAFFIC_INTENSITY 5
#define ARR_INT 2.0
#define PORT 10000
#define MAX_SIM_TIME 1000

NS_LOG_COMPONENT_DEFINE("TestQueue");

Ipv4Address ip(char type, int id, int path = 0, int endpoint = 0)
{
	int p1, p2, p3;
	switch (type)
	{
	case 'L':
		p1 = 10;
		p2 = id / 100;
		p3 = id % 100;
		break;
	case 'S':
	case 'D':
		p1 = type == 'S' ? 20 : 30;
		p2 = id;
		p3 = path;
		break;
	}
	char ip[50];
	sprintf(ip, "%d.%d.%d.%d", p1, p2, p3, endpoint);
	return Ipv4Address(ip);
}

int main(void) {
	TrafficControlHelper tch;

	///
	/// Build network
	///

	// Create routers and end systems
	NodeContainer nodes, routers;
	nodes.Create(2);
	routers.Create(2);

	// Internet stack installation
	InternetStackHelper internet;
	internet.Install(nodes);
	internet.Install(routers);

	PointToPointHelper p2p;
	Ipv4AddressHelper ipv4;
	
	// Connect routers
	p2p.SetDeviceAttribute("DataRate", DataRateValue(DataRate(round(LINK_CAPACITY * 1024 * 1024 * 8 / 10))));
	p2p.SetChannelAttribute("Delay", StringValue("0ms"));
	p2p.SetQueue("ns3::DropTailQueue", "MaxPackets", UintegerValue(5));
	NetDeviceContainer e = p2p.Install(routers.Get(0), routers.Get(1));

	// Routers' ip: 208.104.71.0/24 (WAN)
	ipv4.SetBase(Ipv4Address("208.104.71.0"), "255.255.255.0","0.0.0.1");
	ipv4.Assign(e);
	tch.Uninstall(e);

	// Connect sender with its router
	p2p.SetDeviceAttribute("DataRate", StringValue("100Gbps"));
	p2p.SetChannelAttribute("Delay", StringValue("0ms"));
	NetDeviceContainer es = p2p.Install(nodes.Get(0), routers.Get(0));
	NetDeviceContainer et = p2p.Install(nodee.Get(1), routers.Get(1));

	// Assign ip addresses to LAN: 208.104.70.0/24 (sender), 208.104.72.0/24 (receiver)
	ipv4.SetBase(Ipv4Address("208.104.70.0"), "255.255.255.0", "0.0.0.1");
	ipv4.Assign(es);
	tch.Uninstall(es);

	ipv4.SetBase(Ipv4Address("208.104.72.0"), "255.255.255.0", "0.0.0.1");
	ipv4.Assign(et);
	tch.Uninstall(et);

	// Static routing setting
	Ipv4StaticRoutingHelper routing;

	// for end systems
	Ptr<Ipv4StaticRouting> rs = routing.GetStaticRouting(nodes.Get(0)->GetObject<ipv4>());
	Ptr<Ipv4StaticRouting> rt = routing.GetStaticRouting(nodes.Get(1)->GetObject<ipv4>());
	rs->AddNetworkRouteTo(Ipv4Address::GetAny(), Ipv4Mask::GetZero(), 1);
	rt->AddNetworkRouteTo(Ipv4Address::GetAny(), Ipv4Mask::GetZero(), 1);

	// for routers
	Ptr<Ipv4StaticRouting> ra = routing.GetStaticRouting(routers.Get(0)->GetObject<ipv4>());
	Ptr<Ipv4StaticRouting> rb = routing.GetStaticRouting(routers.Get(1)->GetObject<ipv4>());
	ra->AddHostRouteTo(Ipv4Address("208.104.70.1"), es.Get(1)->GetIfIndex());
	rb->AddHostRouteTo(Ipv4Address("208.104.72.1"), et.Get(1)->GetIfIndex());
	ra->AddHostRouteTo(Ipv4Address("208.104.72.1"), e.Get(0)->GetIfIndex());
	rb->AddHostRouteTo(Ipv4Address("208.104.70.1"), e.Get(1)->GetIfIndex());

	///
	/// Build traffic
	///

	// Random variables
	default_random_engine generator;
	exponential_distribution<> arrival(1 / ARR_INT);
	double mean = TRAFFIC_INTENSITY * 1024 * 1024 / 10;
	normal_distribution<> filesize(mean, FILE_SIZE_DEV / 10);

	// Sender
	double temp = max(filesize(generator), 1.0);
	OnOffHelper onOffHelper("ns3::UdpSocketFactory", InetSocketAddress(Ipv4Address("208.104.72.1"), PORT));
	onOffHelper.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
	onOffHelper.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
	int rateToSet = round(temp * 8);
	if (rateToSet < 1) rateToSet = 1;
	onOffHelper.SetAttribute("DataRate", DataRateValue(DataRate(rateToSet)));

	ApplicationContainer onOffApp = onOffHelper.Install(nodes.Get(0));
	onOffApp.Start(Seconds(1.0));
	onOffApp.Stop(Seconds(MAX_SIM_TIME - 100));

	// Receiver
	PacketSinkHelper sink("ns3::UdpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), PORT));

	ApplicationContainer sinkApp = sink.Install(nodes.Get(1));
	sinkApp.Start(Seconds(0));
	sinkApp.Stop(Seconds(MAX_SIM_TIME));


	// Simulation start!
	Simulator::Run();
	Simulator::Destroy();
	

	return 0;
}