#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"

using namespace ns3;

class BellmanFordApplication : public Application
{
public:
  BellmanFordApplication();
  virtual ~BellmanFordApplication();

  void SetGraphData(uint32_t source, uint32_t destination, int32_t weight);
  void SetSource(uint32_t source);

private:
  virtual void StartApplication();
  virtual void StopApplication();

  void RunBellmanFord();

  struct GraphEdge
  {
    uint32_t source;
    uint32_t destination;
    int32_t weight;
  };

  uint32_t m_source;
  std::vector<GraphEdge> m_graph;
};

BellmanFordApplication::BellmanFordApplication()
{
}

BellmanFordApplication::~BellmanFordApplication()
{
}

void BellmanFordApplication::SetGraphData(uint32_t source, uint32_t destination, int32_t weight)
{
  GraphEdge edge;
  edge.source = source;
  edge.destination = destination;
  edge.weight = weight;
  m_graph.push_back(edge);
}

void BellmanFordApplication::SetSource(uint32_t source)
{
  m_source = source;
}

void BellmanFordApplication::StartApplication()
{
  RunBellmanFord();
}

void BellmanFordApplication::StopApplication()
{
}

void BellmanFordApplication::RunBellmanFord()
{
  std::vector<int32_t> dist(GetNode()->GetN(), std::numeric_limits<int32_t>::max());
  dist[m_source] = 0;

  for (uint32_t i = 0; i < GetNode()->GetN() - 1; ++i)
  {
    for (const auto &edge : m_graph)
    {
      if (dist[edge.source] != std::numeric_limits<int32_t>::max() && dist[edge.source] + edge.weight < dist[edge.destination])
      {
        dist[edge.destination] = dist[edge.source] + edge.weight;
      }
    }
  }

  std::cout << "Bellman-Ford Routing Table:" << std::endl;
  std::cout << "Node \tDistance from Source" << std::endl;
  for (uint32_t i = 0; i < GetNode()->GetN(); ++i)
  {
    std::cout << i << "\t\t" << dist[i] << std::endl;
  }
}

int main(int argc, char *argv[])
{
  // Create network topology
  NodeContainer nodes;
  nodes.Create(5); // Example topology with 5 nodes

  // Create point-to-point links
  PointToPointHelper p2p;
  p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
  p2p.SetChannelAttribute("Delay", StringValue("2ms"));

  NetDeviceContainer devices;
  devices = p2p.Install(nodes.Get(0), nodes.Get(1));
  devices = p2p.Install(nodes.Get(1), nodes.Get(2));
  devices = p2p.Install(nodes.Get(2), nodes.Get(3));
  devices = p2p.Install(nodes.Get(3), nodes.Get(4));
  devices = p2p.Install(nodes.Get(4), nodes.Get(0));

  // Install internet stack
  InternetStackHelper internet;
  internet.Install(nodes);

