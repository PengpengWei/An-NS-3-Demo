#include <iostream>
#include <cmath>
#include <random>
#include "ns3/netanim-module.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/csma-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/traffic-control-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/queue.h"

using namespace std;
using namespace ns3;

#define PORT 10000

// Tunable parameters:
/////////////////////////////////
#define LINK_CAPACITY 0.14      // MB/s
#define TRAFFIC_INTENSITY 0.12  // MB/s
#define TRAFFIC_DEV 0.03        // MB/s
#define MAX_SIM_TIME 3.0        // Seconds
#define NOISE_RESOLUTION 0.012  // Seconds
#define BUFFER_ON 1             // 1 - On, 0 - Off (Using default settings)
#define BUFFER_SIZE 5000        // Bytes
#define MAX_PKT_SIZE 40         // Bytes
/////////////////////////////////

// Trace
int droptimes = 0;
void MarkDrop(std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > arg
	, Ptr<const Packet> p) {
	droptimes++;
}

NS_LOG_COMPONENT_DEFINE("P2PSim");

int main(void) {
	Time::SetResolution(Time::NS);
	
	TrafficControlHelper tch;

	///
	/// Build network
	///

	// Create routers and end systems
	NodeContainer senderNodes, routerNodes, receiverNode;
	int num_of_sender = (int)(MAX_SIM_TIME / NOISE_RESOLUTION);
	senderNodes.Create(num_of_sender);
	routerNodes.Create(2);
	receiverNode.Create(1);

	// Internet stack installation
	InternetStackHelper internet;
	internet.Install(senderNodes);
	internet.Install(routerNodes);
	internet.Install(receiverNode);

	PointToPointHelper p2p;
	Ipv4AddressHelper ipv4;

	// Buffer setting for senders
	p2p.SetQueue("ns3::DropTailQueue", "Mode", EnumValue(Queue::QUEUE_MODE_BYTES));
	if (BUFFER_ON) {
		//p2p.SetQueue("ns3::DropTailQueue", "Mode", EnumValue(Queue::QUEUE_MODE_PACKETS));
		//p2p.SetQueue("ns3::DropTailQueue", "MaxPackets", UintegerValue(2));
		p2p.SetQueue("ns3::DropTailQueue", "MaxBytes", UintegerValue(BUFFER_SIZE));
	}

	// Connect routers
	ostringstream oss;
	oss.str("");
	oss << LINK_CAPACITY << "MB/s";
	p2p.SetDeviceAttribute("DataRate", StringValue(oss.str())); 
	p2p.SetChannelAttribute("Delay", StringValue("0ms"));
	NetDeviceContainer routerDevs = p2p.Install(routerNodes.Get(0), routerNodes.Get(1));

	// Trace field
	{
		ostringstream oss2;
		oss2 << "/NodeList/"
			<< routerNodes.Get(0)->GetId()
			<< "/DeviceList/"
			<< "*/$ns3::PointToPointNetDevice/TxQueue/Drop";
		Config::Connect(oss2.str(), MakeCallback(&MarkDrop));
	}

	// Routers' ip: 208.104.71.0/24 (WAN)
	ipv4.SetBase(Ipv4Address("208.104.71.0"), "255.255.255.0", "0.0.0.1");
	ipv4.Assign(routerDevs);
	tch.Uninstall(routerDevs);

	CsmaHelper csma;

	//csma.SetQueue("ns3::DropTailQueue", "Mode", EnumValue(Queue::QUEUE_MODE_BYTES));
	//if (BUFFER_ON) {
	//	//p2p.SetQueue("ns3::DropTailQueue", "Mode", EnumValue(Queue::QUEUE_MODE_PACKETS));
	//	//p2p.SetQueue("ns3::DropTailQueue", "MaxPackets", UintegerValue(2));
	//	csma.SetQueue("ns3::DropTailQueue", "MaxBytes", UintegerValue(BUFFER_SIZE));
	//}

	//// Trace field
	//{
	//	ostringstream oss2;
	//	oss2 << "/NodeList/*"
	//		//	<< routerNodes.Get(0)->GetId()
	//		<< "/DeviceList/"
	//		<< "*/$ns3::CsmaNetDevice/TxQueue/Drop";
	//	Config::Connect(oss2.str(), MakeCallback(&MarkDrop));
	//}

	// Not to set buffer limitation on receiver's side.
	if (BUFFER_ON) {
		p2p.SetQueue("ns3::DropTailQueue", "MaxBytes", UintegerValue(BUFFER_SIZE * 10));
	}

	// Connect sender & receiver with its router
	csma.SetChannelAttribute("DataRate", StringValue("100Gbps"));
	csma.SetChannelAttribute("Delay", StringValue("0ms"));
	NodeContainer srNodes = senderNodes;
	srNodes.Add(routerNodes.Get(0));
	NetDeviceContainer sr_dev = csma.Install(srNodes);

	NetDeviceContainer rr_dev = p2p.Install(receiverNode.Get(0), routerNodes.Get(1));

	// Assign ip addresses to LAN: 208.104.70.0/24 (sender)
	ipv4.SetBase(Ipv4Address("208.104.70.0"), "255.255.255.0", "0.0.0.1");
	ipv4.Assign(sr_dev);
	tch.Uninstall(sr_dev);

	// Assign ip addresses to LAN: 208.104.72.0/24 (receiver)
	ipv4.SetBase(Ipv4Address("208.104.72.0"), "255.255.255.0", "0.0.0.1");
	ipv4.Assign(rr_dev);
	tch.Uninstall(rr_dev);

	///static routing (obsolete)
	/*
	// Static routing setting
	Ipv4StaticRoutingHelper routing;

	// for end systems
	for (int i = 0; i < num_of_sender; i++) {
		Ptr<Ipv4StaticRouting> rs;
		rs = routing.GetStaticRouting(senderNodes.Get(0)->GetObject<Ipv4>());
		rs->AddNetworkRouteTo(Ipv4Address::GetAny(), Ipv4Mask::GetZero(), 1);
	}
	Ptr<Ipv4StaticRouting> rt = routing.GetStaticRouting(receiverNode.Get(0)->GetObject<Ipv4>());
	rt->AddNetworkRouteTo(Ipv4Address::GetAny(), Ipv4Mask::GetZero(), 1);

	// for routers
	Ptr<Ipv4StaticRouting> ra = routing.GetStaticRouting(routerNodes.Get(0)->GetObject<Ipv4>());
	Ptr<Ipv4StaticRouting> rb = routing.GetStaticRouting(routerNodes.Get(1)->GetObject<Ipv4>());
	// for Router 0
	ra->AddHostRouteTo(Ipv4Address("208.104.72.1"), routerDevs.Get(0)->GetIfIndex());
	for (int i = 0; i < num_of_sender; i++) {
		ostringstream oss;
		oss.str("");
		oss << "208.104.70." << i;
		ra->AddHostRouteTo(Ipv4Address(oss), sr_dev_ptr[i].Get(1)->GetIfIndex());
	}
	// for Router 1
	rb->AddHostRouteTo(Ipv4Address("208.104.72.1"), rr_dev_ptr->Get(1)->GetIfIndex());
	for (int i = 0; i < num_of_sender; i++) {
		ostringstream oss;
		oss.str("");
		oss << "208.104.70." << i;
		rb->AddHostRouteTo(Ipv4Address(oss), routerDevs.Get(1)->GetIfIndex());
	}
	*/

	//global routing
	Ipv4GlobalRoutingHelper::PopulateRoutingTables();

	///
	/// Build traffic
	///

	// Random variables
	default_random_engine generator;
	double mean = TRAFFIC_INTENSITY * 1024 * 1024;
	normal_distribution<> trRate(mean, TRAFFIC_DEV * 1024 * 1024);

	// Sender
	for (int i = 0; i < num_of_sender; i++) {
		double temp = max(trRate(generator), 1.0);
		OnOffHelper onOffHelper("ns3::UdpSocketFactory", InetSocketAddress(Ipv4Address("208.104.72.1"), PORT));
		onOffHelper.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
		onOffHelper.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
		onOffHelper.SetAttribute("PacketSize", UintegerValue(MAX_PKT_SIZE));
		int rateToSet = round(temp * 8);
		if (rateToSet < 1) rateToSet = 1;
		onOffHelper.SetAttribute("DataRate", DataRateValue(DataRate(rateToSet)));
		// cout << "Rate to set " << rateToSet << endl;

		ApplicationContainer onOffApp = onOffHelper.Install(senderNodes.Get(i));
		onOffApp.Start(Seconds(i * NOISE_RESOLUTION));
		onOffApp.Stop(Seconds((i + 1) * NOISE_RESOLUTION));
	}

	// Receiver
	PacketSinkHelper sink("ns3::UdpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), PORT));

	ApplicationContainer sinkApp = sink.Install(receiverNode.Get(0));
	sinkApp.Start(Seconds(0));
	sinkApp.Stop(Seconds(MAX_SIM_TIME * 1.5));

	p2p.EnablePcap("P2PSim-Receiver", rr_dev.Get(0), true);
	p2p.EnablePcap("P2PSim-Router2", rr_dev.Get(1), true);
	p2p.EnablePcap("P2PSim-Router1", routerDevs.Get(0), true);
	csma.EnablePcap("Csma-Sender", sr_dev.Get(0), true);

	// Simulation start!
	Simulator::Run();
	Simulator::Destroy();

	cout << "Drop: " << droptimes << endl;

	return 0;
}