#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ndnSIM-module.h"
#include "ns3/internet-module.h" // Add this line

#include <iostream>
#include <fstream>

using namespace ns3; // Add this line

namespace ns3 {

class SatisfactionBasedPushback {
public:
  SatisfactionBasedPushback() : forwardingLimit(100), graceThreshold(0.5) {}

  void adjustForwardingLimits() {
    double satisfactionLevel = calculateSatisfactionLevel();
    if (satisfactionLevel < graceThreshold) {
      // Reduce the forwarding limits by 10%
      std::cout << "Satisfaction level below threshold. Reducing forwarding limits by 10%" << std::endl;
      forwardingLimit *= 0.9;
    } else {
      // Increase the forwarding limits by 10%
      std::cout << "Satisfaction level above threshold. Increasing forwarding limits by 10%" << std::endl;
      forwardingLimit *= 1.1;
    }
    std::cout << "Current forwarding limit: " << forwardingLimit << std::endl;
  }

private:
  double calculateSatisfactionLevel() {
    // Placeholder for satisfaction level calculation
    return static_cast<double>(rand()) / static_cast<double>(RAND_MAX);
  }

  double forwardingLimit;
  double graceThreshold;
};

class SatisfactionBasedInterestAcceptance {
public:
  SatisfactionBasedInterestAcceptance() : acceptanceProbability(1.0), graceThreshold(0.5) {}

  bool shouldAcceptInterest() {
    double satisfactionLevel = calculateSatisfactionLevel();
    if (satisfactionLevel < graceThreshold) {
      // Reduce the acceptance probability by 10%
      acceptanceProbability *= 0.9;
      std::cout << "Satisfaction level below threshold. Reducing acceptance probability by 10%" << std::endl;
    } else {
      // Increase the acceptance probability by 10%
      acceptanceProbability *= 1.1;
      std::cout << "Satisfaction level above threshold. Increasing acceptance probability by 10%" << std::endl;
    }
    std::cout << "Current acceptance probability: " << acceptanceProbability << std::endl;
    // Accept the interest with the current probability
    return static_cast<double>(rand()) / static_cast<double>(RAND_MAX) < acceptanceProbability;
  }

private:
  double calculateSatisfactionLevel() {
    // Placeholder for satisfaction level calculation
    return static_cast<double>(rand()) / static_cast<double>(RAND_MAX);
  }

  double acceptanceProbability;
  double graceThreshold;
};

} // namespace ns3

int main(int argc, char* argv[]) {
  // Redirect stdout to a file
  std::ofstream out("log.txt");
  std::streambuf* coutbuf = std::cout.rdbuf(); 
  std::cout.rdbuf(out.rdbuf()); 

  // Set up logging levels
  LogComponentEnable("ndn.ConsumerCbr", LOG_LEVEL_INFO); 
  LogComponentEnable("ndn.Producer", LOG_LEVEL_INFO);   

  Config::SetDefault("ns3::PointToPointNetDevice::DataRate", StringValue("1Mbps"));
  Config::SetDefault("ns3::PointToPointChannel::Delay", StringValue("10ms"));
  Config::SetDefault("ns3::DropTailQueue<Packet>::MaxSize", StringValue("20p"));

  CommandLine cmd;
  cmd.Parse(argc, argv);

  NodeContainer nodes;
  nodes.Create(5); // Create 6 nodes instead of 5 to include the malicious node

  PointToPointHelper p2p;
  p2p.Install(nodes.Get(0), nodes.Get(2));
  p2p.Install(nodes.Get(1), nodes.Get(2));
  p2p.Install(nodes.Get(2), nodes.Get(3));
  p2p.Install(nodes.Get(3), nodes.Get(4));


  // Integrating SatisfactionBasedPushback
  ns3::SatisfactionBasedPushback pushback;
  pushback.adjustForwardingLimits(); // Apply SatisfactionBasedPushback

  // Integrating SatisfactionBasedInterestAcceptance
  ns3::SatisfactionBasedInterestAcceptance interestAcceptance;
  bool acceptInterest = interestAcceptance.shouldAcceptInterest();
  std::cout << "Should accept interest? " << std::boolalpha << acceptInterest << std::endl;

  ns3::ndn::StackHelper ndnHelper;
  ndnHelper.SetDefaultRoutes(true);
  ndnHelper.InstallAll();

  // Selecting strategy for individual nodes
  ns3::ndn::StrategyChoiceHelper strategyHelper;
  strategyHelper.Install(nodes.Get(2), "/prefix", "/localhost/nfd/strategy/multicast");
  for (uint32_t i = 0; i < nodes.GetN(); ++i) {
    if (i != 2) {
      strategyHelper.Install(nodes.Get(i), "/prefix", "/localhost/nfd/strategy/best-route");
    }
  }

  // Install applications
  ns3::ndn::AppHelper consumerHelper("ns3::ndn::ConsumerCbr");
  consumerHelper.SetPrefix("/prefix");
  consumerHelper.SetAttribute("Frequency", StringValue("10"));
  auto consumerApps = consumerHelper.Install(nodes.Get(0));
  consumerApps.Start(Seconds(1.0)); 
  consumerApps.Stop(Seconds(20.0)); 
  
  // Malicious consumer setup
  ns3::ndn::AppHelper maliciousConsumerHelper("ns3::ndn::ConsumerCbr");
  maliciousConsumerHelper.SetPrefix("/prefix");
  maliciousConsumerHelper.SetAttribute("Frequency", StringValue("150")); 
  auto consumerApps1 = maliciousConsumerHelper.Install(nodes.Get(1)); 
  consumerApps1.Start(Seconds(1.0)); 
  consumerApps1.Stop(Seconds(10.0)); 

  ns3::ndn::AppHelper producerHelper("ns3::ndn::Producer");
  producerHelper.SetPrefix("/prefix");
  producerHelper.SetAttribute("PayloadSize", StringValue("1024"));
  producerHelper.Install(nodes.Get(4));

  // Enable NDN visualizer
  ns3::ndn::AppDelayTracer::InstallAll("ndn-traffic-trace.txt");
  
  AsciiTraceHelper ascii;
  p2p.EnableAsciiAll(ascii.CreateFileStream("3p.tr"));

  Simulator::Stop(Seconds(20.0));
  Simulator::Run();
  Simulator::Destroy();

  // Restore stdout
  std::cout.rdbuf(coutbuf); 

  return 0;
}

