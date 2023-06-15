#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/traffic-control-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("CongestionRoutingExample");

int main(int argc, char *argv[]) {
  // Create a simulation object
  Ptr<Simulation> sim = CreateObject<Simulation>();

  // Set up global configuration
  Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpNewReno"));
  
  // Set up network topology
  NodeContainer nodes;
  nodes.Create(2);

  PointToPointHelper p2p;
  p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
  p2p.SetChannelAttribute("Delay", StringValue("2ms"));

  NetDeviceContainer devices;
  devices = p2p.Install(nodes);

  // Enable RED queue management on the devices
  TrafficControlHelper tch;
  tch.SetRootQueueDisc("ns3::RedQueueDisc");
  tch.Install(devices);

  // Install internet stack on nodes
  InternetStackHelper internet;
  internet.Install(nodes);

  // Assign IP addresses to devices
  Ipv4AddressHelper address;
  address.SetBase("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces = address.Assign(devices);

  // Set up routing
  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  // Create applications
  uint16_t port = 9;  // Discard port (RFC 863)

  OnOffHelper onoff("ns3::UdpSocketFactory", InetSocketAddress(interfaces.GetAddress(1), port));
  onoff.SetConstantRate(DataRate("1Mbps"));

  ApplicationContainer apps = onoff.Install(nodes.Get(0));
  apps.Start(Seconds(1.0));
  apps.Stop(Seconds(10.0));

  // Enable packet tracing
  p2p.EnablePcapAll("congestion_routing");

  // Run the simulation
  sim->Run();
  sim->Destroy();

  return 0;
}
