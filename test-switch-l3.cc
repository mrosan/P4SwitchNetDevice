/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

// Network topology
//
//        n0     n1
//        |      |
//       ----------
//       | Switch |
//       ----------
//        |      |
//        n2     n3
//
//
// - CBR/UDP flows from n0 to n1 and from n3 to n0
// - DropTail queues
// - Tracing of queues and packet receptions to file "openflow-switch.tr"
// - If order of adding nodes and netdevices is kept:
//      n0 = 00:00:00;00:00:01, n1 = 00:00:00:00:00:03, n3 = 00:00:00:00:00:07
//	and port number corresponds to node number, so port 0 is connected to n0, for example.

#include <iostream>
#include <fstream>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
//#include "ns3/openflow-module.h"
#include "ns3/log.h"

extern "C"
{
  #include "ns3/p4-module.h"
}

extern int exact_add (lookup_table_t* t, uint8_t* key, uint8_t* value);
extern int lpm_add (lookup_table_t* t, uint8_t* key, uint8_t depth, uint8_t* value);

int
init_tables_v1(lookup_table_t** t)
{		
    printf("   ~~~executing init_tables_v1~~~   \n");    
    
    //set default action for both tables
    struct ipv4_fib_lpm_action def_fib_action; 
    def_fib_action.action_id = 0; //action_on_miss
    memcpy(t[0]->default_val, (uint8_t*)&def_fib_action, sizeof(struct ipv4_fib_lpm_action));

    struct sendout_action def_so_action;
    def_so_action.action_id = 0; //action_on_miss
    memcpy(t[1]->default_val, (uint8_t*)&def_so_action, sizeof(struct sendout_action));

    //add entries to lpm table
    uint8_t i;
    for (i=0; i<4; i++){
		    uint8_t key[4] = {10,1,1,i};
		    uint8_t port[2] = {i,0};
		    uint8_t dmac[6] = {0,0,0,0,0,(uint8_t)(1+2*i)};
		    struct action_fib_hit_nexthop_params param;
		    memcpy(param.port,port,sizeof(param.port));
		    memcpy(param.dmac,dmac,sizeof(param.dmac));
		    struct ipv4_fib_lpm_action lpm_action_val;
		    lpm_action_val.action_id = 1;	//action_fib_hit_nexthop
		    lpm_action_val.fib_hit_nexthop_params = param;
		    lpm_add(t[0],key,32,(uint8_t*)&lpm_action_val);
    }	    

    //set smac_rewrite table
    for (i=0; i<4; i++){
		    //uint8_t* smac_value;
		    uint8_t key[2] = {i,0};
		    uint8_t smac[6] = {0,0,0,0,0,(uint8_t)(1+2*i)};
		    struct action_rewrite_src_mac_params param;
		    memcpy(param.smac,smac,sizeof(param.smac));
		    struct sendout_action action_val;	
		    action_val.action_id = 2; //action_rewrite_src_mac
		    action_val.rewrite_src_mac_params = param;
		    exact_add(t[1],key,(uint8_t*)&action_val);
    }


            //set value for testing
            uint8_t key[4] = {10,1,0,0};
            uint8_t port[2] = {0,0};
		        uint8_t dmac[6] = {0,0,0,0,0,42};
            struct action_fib_hit_nexthop_params param;
            memcpy(param.port,port,sizeof(param.port));
		        memcpy(param.dmac,dmac,sizeof(param.dmac));
            struct ipv4_fib_lpm_action lpm_action_val;
            lpm_action_val.action_id = 0;
		        lpm_action_val.fib_hit_nexthop_params = param;
            lpm_add(t[0],key,16,(uint8_t*)&lpm_action_val);
            lpm_add(t[0],key,16,(uint8_t*)&lpm_action_val);
            lpm_add(t[0],key,8,(uint8_t*)&lpm_action_val);
            lpm_add(t[0],key,24,(uint8_t*)&lpm_action_val);

            //TEST
            uint8_t key1[4] = {10,1,0,0};
            uint8_t key2[4] = {10,1,1,0};
            uint8_t key3[4] = {10,1,1,2};
            uint8_t key4[4] = {10,1,0,0};
            uint8_t key5[4] = {0,1,45,0};

            printf("  -> test with key {10,1,0,0}\n");
            lpm_lookup(t[0],key1);
            printf("  -> test with key {10,1,1,0}\n");
            lpm_lookup(t[0],key2);
            printf("  -> test with key {10,1,1,2}\n");
            lpm_lookup(t[0],key3);
            printf("  -> test with key {10,1,0,0}\n");
            lpm_lookup(t[0],key4);
            printf("  -> test with key {0,1,45,0}\n");
            lpm_lookup(t[0],key5);
    
            //another test (overwrite exact_add)
		        uint8_t k[2] = {1,0};
		        uint8_t smac[6] = {0,0,0,0,0,5};
		        struct action_rewrite_src_mac_params param2;
		        memcpy(param2.smac,smac,sizeof(param2.smac));
		        struct sendout_action action_val;	
		        action_val.action_id = 2; //action_rewrite_src_mac
		        action_val.rewrite_src_mac_params = param2;
		        exact_add(t[1],k,(uint8_t*)&action_val);
		        
		        
    
    return 0;
}

int 
p4_msg_digest_v1(lookup_table_t** t, char* name, int receiver, struct type_field_list* digest_field_list)
{
    printf("   ~~~executing p4_msg_digest_v1~~~   \n");
    return 0;
}



using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("P4CsmaSwitchExample");

bool verbose = false;
bool use_drop = false;
ns3::Time timeout = ns3::Seconds (0);

bool
SetVerbose (std::string value)
{
  verbose = true;
  return true;
}

bool
SetDrop (std::string value)
{
  use_drop = true;
  return true;
}

bool
SetTimeout (std::string value)
{
  try {
      timeout = ns3::Seconds (atof (value.c_str ()));
      return true;
    }
  catch (...) { return false; }
  return false;
}

int
main (int argc, char *argv[])
{
  //
  // Allow the user to override any of the defaults and the above Bind() at
  // run-time, via command-line arguments
  //
  CommandLine cmd;
  cmd.AddValue ("v", "Verbose (turns on logging).", MakeCallback (&SetVerbose));
  cmd.AddValue ("verbose", "Verbose (turns on logging).", MakeCallback (&SetVerbose));
  cmd.AddValue ("d", "Use Drop Controller (Learning if not specified).", MakeCallback (&SetDrop));
  cmd.AddValue ("drop", "Use Drop Controller (Learning if not specified).", MakeCallback (&SetDrop));
  cmd.AddValue ("t", "Learning Controller Timeout (has no effect if drop controller is specified).", MakeCallback ( &SetTimeout));
  cmd.AddValue ("timeout", "Learning Controller Timeout (has no effect if drop controller is specified).", MakeCallback ( &SetTimeout));

  cmd.Parse (argc, argv);

  if (verbose)
    {
      LogComponentEnable ("P4CsmaSwitchExample", LOG_LEVEL_INFO);
      LogComponentEnable ("P4SwitchNetDevice", LOG_LEVEL_INFO);
      LogComponentEnable ("P4SwitchHelper", LOG_LEVEL_INFO);
    }

  //
  // Explicitly create the nodes required by the topology (shown above).
  //
  NS_LOG_INFO ("Create nodes, 4+1.");
  NodeContainer terminals;
  terminals.Create (4);

  NodeContainer csmaSwitch;
  csmaSwitch.Create (1);

  NS_LOG_INFO ("Build Topology.");
  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", DataRateValue (5000000));
  csma.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (2)));

  // Create the csma links, from each terminal to the switch
  NetDeviceContainer terminalDevices;
  NetDeviceContainer switchDevices;
  for (int i = 0; i < 4; i++)
    {
      NetDeviceContainer link = csma.Install (NodeContainer (terminals.Get (i), csmaSwitch));
      terminalDevices.Add (link.Get (0));
      switchDevices.Add (link.Get (1));
    }



  // Create the switch netdevice, which will do the packet switching
  Ptr<Node> switchNode = csmaSwitch.Get (0);
  P4SwitchHelper swtch;
  
  swtch.Install(switchNode,switchDevices,init_tables_v1,p4_msg_digest_v1);



  // Add internet stack to the terminals
  InternetStackHelper internet;
  internet.Install (terminals);

  // We've got the "hardware" in place.  Now we need to add IP addresses.
  NS_LOG_INFO ("Assign IP Addresses.");
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  ipv4.Assign (terminalDevices);

  // Create an OnOff application to send UDP datagrams from n0 to n1.
  NS_LOG_INFO ("Create Applications.");
  uint16_t port = 9;   // Discard port (RFC 863)

  OnOffHelper onoff ("ns3::UdpSocketFactory",
                     Address (InetSocketAddress (Ipv4Address ("10.1.1.2"), port)));
  onoff.SetConstantRate (DataRate ("500kb/s"));

  ApplicationContainer app = onoff.Install (terminals.Get (0));
  // Start the application
  app.Start (Seconds (1.0));
  //app.Stop (Seconds (2.0));
  app.Stop (Seconds (1.4));

  // Create an optional packet sink to receive these packets
  PacketSinkHelper sink ("ns3::UdpSocketFactory",
                         Address (InetSocketAddress (Ipv4Address::GetAny (), port)));
  app = sink.Install (terminals.Get (1));
  app.Start (Seconds (0.0));

  //
  // Create a similar flow from n3 to n0, starting at time 1.1 seconds
  //
  onoff.SetAttribute ("Remote",
                      AddressValue (InetSocketAddress (Ipv4Address ("10.1.1.1"), port)));
  app = onoff.Install (terminals.Get (3));
  app.Start (Seconds (1.1));
  //app.Stop (Seconds (2.0));
  app.Stop (Seconds (1.4));

  app = sink.Install (terminals.Get (0));
  app.Start (Seconds (0.0));

  NS_LOG_INFO ("Configure Tracing.");

  //
  // Configure tracing of all enqueue, dequeue, and NetDevice receive events.
  // Trace output will be sent to the file "openflow-switch.tr"
  //
  AsciiTraceHelper ascii;
  csma.EnableAsciiAll (ascii.CreateFileStream ("openflow-switch.tr"));

  //
  // Also configure some tcpdump traces; each interface will be traced.
  // The output files will be named:
  //     openflow-switch-<nodeId>-<interfaceId>.pcap
  // and can be read by the "tcpdump -r" command (use "-tt" option to
  // display timestamps correctly)
  //
  csma.EnablePcapAll ("openflow-switch", false);

  //
  // Now, do the actual simulation.
  //
  NS_LOG_INFO ("Run Simulation.");
  Simulator::Run ();
  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");
}
