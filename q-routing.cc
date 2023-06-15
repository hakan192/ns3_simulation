#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/point-to-point-layout-module.h"
#include "ns3/csma-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/gnuplot.h"

using namespace ns3;

int main(int argc, char* argv[]) {
  // Create nodes and set up network topology
  NodeContainer nodes;
  nodes.Create(36); // 6x6 grid, total 36 nodes

  PointToPointHelper p2p;
  p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
  p2p.SetChannelAttribute("Delay", StringValue("2ms"));

  CsmaHelper csma;
  csma.SetChannelAttribute("DataRate", StringValue("100Mbps"));
  csma.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560)));

  PointToPointGridHelper grid(p2p, 6, 6);
  grid.Install(nodes);

  InternetStackHelper internet;
  internet.Install(nodes);

  Ipv4AddressHelper address;
  address.SetBase("10.0.0.0", "255.255.255.0");
  address.Assign(grid.GetLeftDevices());

  NodeContainer csmaNodes;
  csmaNodes.Add(nodes.Get(24));
  csmaNodes.Add(nodes.Get(25));
  csmaNodes.Add(nodes.Get(26));
  csmaNodes.Add(nodes.Get(30));
  csmaNodes.Add(nodes.Get(31));
  csmaNodes.Add(nodes.Get(32));
  csma.Install(csmaNodes);

  address.SetBase("20.0.0.0", "255.255.255.0");
  address.Assign(csmaDevices);

  // Install applications
  uint16_t port = 50000;
  Address sinkAddress(InetSocketAddress(Ipv4Address::GetAny(), port));
  PacketSinkHelper packetSinkHelper("ns3::TcpSocketFactory", sinkAddress);
  ApplicationContainer sinkApps = packetSinkHelper.Install(nodes.Get(0));
  sinkApps.Start(Seconds(0.0));
  sinkApps.Stop(Seconds(10.0));

  OnOffHelper onOffHelper("ns3::TcpSocketFactory", Address());
  onOffHelper.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
  onOffHelper.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
  onOffHelper.SetAttribute("DataRate", StringValue("1Mbps"));
  onOffHelper.SetAttribute("PacketSize", UintegerValue(1000));

  for (uint32_t i = 1; i <= 6; ++i) {
    AddressValue remoteAddress(InetSocketAddress(csmaInterfaces.GetAddress(i), port));
    onOffHelper.SetAttribute("Remote", remoteAddress);
    ApplicationContainer apps = onOffHelper.Install(nodes.Get(i));
    apps.Start(Seconds(1.0));
    apps.Stop(Seconds(10.0));
  }

  // Enable packet capture
  p2p.EnablePcapAll("grid");
  csma.EnablePcapAll("grid");

  // Create FlowMonitor
  FlowMonitorHelper flowMonitorHelper;
  Ptr<FlowMonitor> flowMonitor = flowMonitorHelper.InstallAll();

  // Run simulation
  Simulator::Run();
  Simulator::Destroy();

  // Collect and analyze statistics
  flowMonitor->CheckForLostPackets();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(flowMonitorHelper.GetClassifier());
  std::map<FlowId, FlowMonitor::FlowStats> stats = flowMonitor->GetFlowStats();

  // Create graph to visualize delay
  Gnuplot plot("delay_plot.png");
  plot.SetTitle("Delay vs. Time");
  plot.SetLegend("Time", "Delay (ms)");

  Gnuplot2dDataset dataset;
  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator it = stats.begin(); it != stats.end(); ++it) {
    Ipv4FlowClassifier::FiveTuple flow = classifier->FindFlow(it->first);
    double time = it->second.timeLastRxPacket.GetSeconds();
    double delay = (it->second.delaySum / it->second.rxPackets) * 1000.0; // Delay in milliseconds
    dataset.Add(time, delay);
  }

  plot.AddDataset(dataset);
  plot.GenerateOutput();

  return 0;
}
