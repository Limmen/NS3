
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

// Default Network Topology
//
//       10.1.1.0
// n0 -------------- n1
//    point-to-point
//

// Add ns3 applications to scope (avoid having to prefix with ns3:: all the time)
using namespace ns3;

//ns3 has rich built in logging capabilities.
// Here we define a logging component
NS_LOG_COMPONENT_DEFINE ("HelloWorldExample");

/**
 * Configure logging
 */
void enableLogging() {
    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);
    LogComponentEnable("HelloWorldExample", LOG_LEVEL_INFO);
}

/**
 * Creates nodes for the simulation (abstraction for computers).
 *
 * NodeContainer has the following purpose:
 * Typically ns-3 helpers operate on more than one node at a time.  For example
 * a device helper may want to install devices on a large number of similar nodes.
 * The helper Install methods usually take a NodeContainer as a parameter.
 * NodeContainers hold the multiple Ptr<Node> which are used to refer to the nodes.
 *
 * @return container with the created nodes
 */
NodeContainer createNodes() {
    NodeContainer nodes;
    // Creates 2 nodes and add their pointers to the NodeContainer
    nodes.Create(2);
    return nodes;
}

/**
 * Just as we used the NodeContainer topology helper object to create the Nodes for our simulation,
 * we will ask the PointToPointHelper to do the work involved in creating, configuring and installing our devices
 * for us. We will need to have a list of all of the NetDevice objects that are created, so we use a NetDeviceContainer to
 * hold them just as we used a NodeContainer to hold the nodes we created.
 *
 * NetDeviceContainer holds a vector of ns3::NetDevice pointers
 * Typically ns-3 NetDevices are installed on nodes using a net device helper.
 * The helper Install method takes a NodeContainer which holds
 * some number of Ptr<Node>.  For each of the Nodes in the NodeContainer
 * the helper will instantiate a net device, add a MAC address and a queue
 * to the device and install it to the node.  For each of the devices, the
 * helper also adds the device into a Container for later use by the caller.
 * This is that container used to hold the Ptr<NetDevice> which are
 * instantiated by the device helper.
 *
 * @param nodes nodes to connect with point to point links
 * @returns container with net devices
 */
NetDeviceContainer createPointToPointLinks(NodeContainer nodes) {

    // Helper to Build a set of PointToPointNetDevice objects.
    // A NetDevice is an abstraction for a network card and a "Channel" is an abstraction for a network cable
    PointToPointHelper pointToPoint;

    // Configure the point to point helper to use 5Mbs data rate whenever creating NetDevices
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));

    // Configure the point to point helper to use 2ms propagation delay whenever creating channels
    pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));

    NetDeviceContainer devices;
    devices = pointToPoint.Install(nodes);

    // Record pcap files of the communication over this link
    pointToPoint.EnablePcap("hello_world", nodes, true);

    // Capture tracing output in ASCII files as well
    AsciiTraceHelper ascii;
    pointToPoint.EnableAsciiAll (ascii.CreateFileStream ("hello_world.tr"));

    return devices;
}

/**
 * Install IP stack and configure interfaces on network cards
 *
 * @param nodes nodes that contain the network cards
 * @param devices the network cards
 * @return list of configured interfaces
 */
Ipv4InterfaceContainer installAndConfigureInternetStacksOnNodes(NodeContainer nodes, NetDeviceContainer devices) {
    /**
     * The InternetStackHelper is a topology helper that is to internet stacks what the PointToPointHelper is
     * to point-to-point net devices. The Install method takes a NodeContainer as a parameter. When it is executed,
     * it will install an Internet Stack (TCP, UDP, IP, etc.) on each of the nodes in the node container.
     */
    InternetStackHelper stack;
    stack.Install(nodes);

    /**
     * Associate nodes with IP addresses. Again we use a topology helper to manage a IPs for a container of nodes
     * at once. The lines below define that the helper should begin by allocated IP addresses from the network
     * 10.1.1.0 using the mask 255.255.255.0. By default the addresses allocated will start at one and increase
     * monotonically, so the first address allocated from this base will be 10.1.1.1, followed by 10.1.1.2, etc.
     */
    Ipv4AddressHelper address;
    char ip[] = "10.1.1.0";
    char mask[] = "255.255.255.0";
    address.SetBase(ip, mask);

    /**
     * Use the helper to assign the IPs to the network devices
     * Just as we sometimes need a list of net devices created by a helper for future reference we
     * sometimes need a list of Ipv4Interface objects. The Ipv4InterfaceContainer provides this functionality
     */
    Ipv4InterfaceContainer interfaces = address.Assign(devices);

    return interfaces;
}

/**
 * Another one of the core abstractions of the ns-3 system is the Application. In this script we
 * use two specializations of the core ns-3 class Application called UdpEchoServerApplication and
 * UdpEchoClientApplication. Just as we have in our previous explanations, we use helper objects to help
 * configure and manage the underlying objects. Here, we use UdpEchoServerHelper and UdpEchoClientHelper
 * objects to make our lives easier.
 *
 * @param port the port of the app
 * @param nodes the list of nodes to install the server-app on
 * @returns the created server apps
 */
ApplicationContainer createServers(int port, NodeContainer nodes) {
    /**
     *  Create a server application which waits for input UDP packets
     *  and sends them back to the original sender.
     */
    UdpEchoServerHelper echoServer(port);

    // Causes the underlying echo server application to be instantiated and attached to a node
    // Returns a container with all of the installed applications
    ApplicationContainer serverApps = echoServer.Install(nodes.Get(1));

    /**
     * Applications require a time to “start” generating traffic and may take an optional time to “stop”. We provide both.
     * These times are set using the ApplicationContainer methods Start and Stop. These methods take Time
     * parameters.
     *
     * Setting Start to 1 and Stop to 10 causes the echo server application to Start (enable itself) at one second
     * into the simulation and to Stop (disable itself) at ten seconds into the simulation.
     *
     * By virtue of the fact that we have declared a simulation event (the application
     * stop event) to be executed at ten seconds, the simulation will last at least ten seconds.
     */
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(10.0));

    return serverApps;
}

/**
 * Create client applications and install on list of nodes
 *
 * @param port the port of the application
 * @param nodes the list of nodes where to install the application
 * @return a container with the created client applications
 */
ApplicationContainer createClients(int port, NodeContainer nodes, Ipv4InterfaceContainer interfaces) {

    /**
     * Create an application which sends a UDP packet and waits for an echo of this packet
     */
    UdpEchoClientHelper echoClient(interfaces.GetAddress(1), port);

    /**
     * The “MaxPackets” Attribute tells the client the maximum number of packets we allow it to send during
     * the simulation. The “Interval” Attribute tells the client how long to wait between packets, and the
     * “PacketSize” Attribute tells the client how large its packet payloads should be.
     * With this particular combination of Attributes, we are
     * telling the client to send one 1024-byte packet
     */
    echoClient.SetAttribute("MaxPackets", UintegerValue(1));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    echoClient.SetAttribute("PacketSize", UintegerValue(1024));

    //Installs the application on the node
    ApplicationContainer clientApps = echoClient.Install(nodes.Get(0));

    /**
     * Just as in the case of the echo server, we tell the echo client to Start and Stop,
     * but here we start the client one
     * second after the server is enabled (at two seconds into the simulation).
     * */
    clientApps.Start(Seconds(2.0));
    clientApps.Stop(Seconds(10.0));

    return clientApps;

}

//Main function, program entrypoint
int main(int argc, char *argv[]) {

    // Parse command line args
    CommandLine cmd;
    cmd.Parse(argc, argv);

    // Sets time resolution to nano second
    Time::SetResolution(Time::NS);

    // Specify Log levels for sub-applications that we are going to use
    enableLogging();

    // Create nodes
    NS_LOG_INFO ("Creating Nodes");
    NodeContainer nodes = createNodes();

    // Create p2p links
    NS_LOG_INFO ("Creating P2P links");
    NetDeviceContainer devices = createPointToPointLinks(nodes);

    // Install internet stacks on nodes and network cards for the p2p links
    NS_LOG_INFO ("Installing Internet Stacks");
    Ipv4InterfaceContainer interfaces = installAndConfigureInternetStacksOnNodes(nodes, devices);

    // Create server apps and install on nodes
    NS_LOG_INFO ("Installing Server Applications");
    int port = 9;
    ApplicationContainer serverApps = createServers(port, nodes);
    // Create client apps and install on nodes
    NS_LOG_INFO ("Installing Client Applications");
    ApplicationContainer clientApps = createClients(port, nodes, interfaces);

    // Run the simulation
    NS_LOG_INFO ("Start Simulation");
    Simulator::Run();

    // Cleanup after completion
    NS_LOG_INFO ("SimulationFinished, Cleanup");
    Simulator::Destroy();

    return 0;
}
