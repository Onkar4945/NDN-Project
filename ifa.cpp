#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ndnSIM-module.h"
#include "ns3/applications-module.h"

namespace ns3 {

int
main(int argc, char* argv[])
{
  Config::SetDefault("ns3::PointToPointNetDevice::DataRate", StringValue("1Mbps"));
  Config::SetDefault("ns3::PointToPointChannel::Delay", StringValue("10ms"));
  Config::SetDefault("ns3::DropTailQueue<Packet>::MaxSize", StringValue("20p"));

  CommandLine cmd;
  cmd.Parse(argc, argv);

  NodeContainer nodes;
  nodes.Create(5);

  PointToPointHelper p2p;
  p2p.Install(nodes.Get(0), nodes.Get(2));
  p2p.Install(nodes.Get(1), nodes.Get(2));
  p2p.Install(nodes.Get(2), nodes.Get(3));
  p2p.Install(nodes.Get(3), nodes.Get(4));

  ndn::StackHelper ndnHelper;
  ndnHelper.SetDefaultRoutes(true);
  ndnHelper.InstallAll();

  ndn::StrategyChoiceHelper::InstallAll("/prefix", "/localhost/nfd/strategy/multicast");

  // Install applications
  ndn::AppHelper consumerHelper("ns3::ndn::ConsumerCbr");
  consumerHelper.SetPrefix("/prefix");
  consumerHelper.SetAttribute("Frequency", StringValue("10"));
  auto consumerApps = consumerHelper.Install(nodes.Get(0));
  consumerApps.Start(Seconds(1.0)); // Start consumer apps
  consumerApps.Stop(Seconds(10.0)); // Stop consumer apps after 10 seconds

  // Implementing Interest Flooding Attack (IFA)
  // Creating a malicious node
  Ptr<Node> maliciousNode = nodes.Get(1);

  // Flood the network with Interests
  ndn::AppHelper maliciousConsumerHelper("ns3::ndn::ConsumerCbr");
  maliciousConsumerHelper.SetPrefix("/prefix");
  maliciousConsumerHelper.SetAttribute("Frequency", StringValue("500")); // Set a high frequency to flood the network
  auto consumerApps1 = maliciousConsumerHelper.Install(maliciousNode);
  consumerApps1.Start(Seconds(1.0)); // Start malicious consumer apps
  consumerApps1.Stop(Seconds(10.0)); // Stop malicious consumer apps after 10 seconds
  
  ndn::AppHelper producerHelper("ns3::ndn::Producer");
  producerHelper.SetPrefix("/prefix");
  producerHelper.SetAttribute("PayloadSize", StringValue("1024"));
  producerHelper.Install(nodes.Get(4));
  
  AsciiTraceHelper ascii;
  p2p.EnableAsciiAll(ascii.CreateFileStream("2p.tr"));

  Simulator::Stop(Seconds(20.0));
  Simulator::Run();
  Simulator::Destroy();

  return 0;
}

} // namespace ns3

int
main(int argc, char* argv[])
{
  return ns3::main(argc, argv);
}

