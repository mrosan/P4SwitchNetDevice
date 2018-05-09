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
// - Tracing of queues and packet receptions to file "lookup-tables-test.tr"
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
#include "ns3/log.h"

extern "C"
{
  #include "ns3/p4-module.h"
}

extern int exact_add (lookup_table_t* t, uint8_t* key, uint8_t* value);
extern int lpm_add (lookup_table_t* t, uint8_t* key, uint8_t prefix_length, uint8_t* value);
extern int ternary_add (lookup_table_t* t, uint8_t* key, uint8_t* mask, uint8_t* value);
extern uint8_t* exact_lookup (lookup_table_t* t, uint8_t* key);
extern uint8_t* lpm_lookup (lookup_table_t* t, uint8_t* key);
extern uint8_t* ternary_lookup (lookup_table_t* t, uint8_t* key);
extern int exact_remove (lookup_table_t* t, uint8_t* key);
extern int lpm_remove (lookup_table_t* t, uint8_t* key, uint8_t prefix_length );
extern int ternary_remove (lookup_table_t* t, uint8_t* key);
extern void free_entries (table_entry_t* t);
extern void print_lpm_tree(table_entry_t* t);
 
int
init_tables_v1(lookup_table_t** t)
{			  
    //set default smac action    
    struct smac_action def_smac_action;
    def_smac_action.action_id = 0; //action_mac_learn
    memcpy(t[0]->default_val, (uint8_t*)&def_smac_action, sizeof(struct smac_action));

    //set default dmac action
    struct action_forward_params param;
    param.port[0] = 100; param.port[1] = 0;
    struct dmac_action def_dmac_action;	
    def_dmac_action.action_id = 3; //action_bcast
    def_dmac_action.forward_params = param;
    memcpy(t[1]->default_val, (uint8_t*)&def_dmac_action, sizeof(struct dmac_action));

    //add broadcast entry to dmac table
    uint8_t key[6] = {255,255,255,255,255,255};
    struct action_forward_params bcast_param;
    bcast_param.port[0] = (uint8_t) BROADCAST_PORT;
    struct dmac_action dmac_action_val;	
    dmac_action_val.action_id = 3; //action_bcast
    dmac_action_val.forward_params = bcast_param;
    exact_add(t[1],key,(uint8_t*)&dmac_action_val);

    //--------------------------TESTING------------------------------
    // use smac table for testing purposes
    
    printf("\n   <------------>\n     EXACT tests \n   <------------>\n");
    
    uint8_t key0[6] = {10,1,0,0,0,0};
    //uint8_t port[2] = {0,0};
    //uint8_t mac[6] = {0,0,0,0,0,42};
    struct smac_action test_smac;
    test_smac.action_id = 42;

    uint8_t key1[6] = {10,1,1,1,0,0};
    uint8_t key2[6] = {10,1,1,0,0,0};
    uint8_t key3[6] = {10,1,1,2,0,0};
    uint8_t key4[6] = {10,1,0,0,0,0};
    uint8_t key5[6] = {45,1,45,1,0,0};
    uint8_t key6[6] = {0,0,1,1,0,0};
    uint8_t key7[6] = {60,0,1,1,0,0};
    uint8_t key8[6] = {45,1,45,0,0,0};
    uint8_t key9[6] = {45,1,45,2,0,0};
    uint8_t key10[6] = {45,1,45,1,5,0};
    
    printf("\n-------> Add a value to the empty table, then remove it right away.\n");
    exact_add(t[0],key0,(uint8_t*)&test_smac);
    exact_remove(t[0],key0);
    printf("\n-------> Lookup for removed value:");
    exact_lookup(t[0],key0);
    
    printf("\n-------> Add a value to different positions of the table.\n");
    exact_add(t[0],key0,(uint8_t*)&test_smac);
    exact_add(t[0],key1,(uint8_t*)&test_smac);
    exact_add(t[0],key2,(uint8_t*)&test_smac);
    exact_add(t[0],key3,(uint8_t*)&test_smac);
    exact_add(t[0],key6,(uint8_t*)&test_smac);
    exact_add(t[0],key5,(uint8_t*)&test_smac);
    exact_add(t[0],key10,(uint8_t*)&test_smac);
    
    printf("\n-------> Overwrite an existing value within the table.\n");
    test_smac.action_id = 11;
    exact_add(t[0],key3,(uint8_t*)&test_smac);
    
    printf("\n-------> Check it with lookup.\n");
    exact_lookup(t[0],key3);
    
    printf("\n-------> Lookup different positions of the table, lookup non-entries.\n");
    exact_lookup(t[0],key8);
    exact_lookup(t[0],key2);
    exact_lookup(t[0],key0);
    exact_lookup(t[0],key1);
    
    printf("\n-------> Remove entries, and try to remove non-entries.\n");
    exact_remove(t[0],key0);
    exact_remove(t[0],key7);
    exact_remove(t[0],key1);
    exact_remove(t[0],key5);
    
    printf("\n-------> Lookup again.\n");
    exact_lookup(t[0],key5);
    exact_lookup(t[0],key10);
    
    printf("\n-------> Free up everything for LPM tests.\n");
    free_entries(t[0]->table);
    t[0]->table = NULL;
    
    
    printf("\n   <------------>\n      LPM tests \n   <------------>\n");
    
    
    printf("\n-------> Add a value to the empty tree, but then remove it right away.\n");
    lpm_add(t[0],key6,16,(uint8_t*)&test_smac);
    lpm_remove(t[0],key6,16);
    lpm_remove(t[0],key6,8);
    
    printf("\n-------> Search for not-existing key in the (empty) tree.\n");
    lpm_lookup(t[0],key6);
    
    print_lpm_tree(t[0]->table);
    
    printf("\n-------> Add a value at depth 2\n");
    lpm_add(t[0],key0,16,(uint8_t*)&test_smac);
    printf("\n-------> Add a different value with the same key at the same depth (overwrite)\n");
    test_smac.action_id = 24;
    lpm_add(t[0],key0,16,(uint8_t*)&test_smac);
    
    printf("\n-------> Look up the previous key -- the entry should contain the new value.\n");
    lpm_lookup(t[0],key0);
    
    printf("\n-------> Add more values with the same key, but at different depths\n");
    test_smac.action_id = 10;
    lpm_add(t[0],key0,8,(uint8_t*)&test_smac);
    test_smac.action_id = 11;
    lpm_add(t[0],key0,24,(uint8_t*)&test_smac);
    
    printf("\n-------> branch out on the first level\n");
    // ("creating neighbours" test)
    test_smac.action_id = 200;
    lpm_add(t[0],key7,16,(uint8_t*)&test_smac);    

    printf("\n-------> set values on the lower levels, the keys aren't in ascending order\n");
    // ("ordering entries" test)
    test_smac.action_id = 101;
    lpm_add(t[0],key5,32,(uint8_t*)&test_smac);
    test_smac.action_id = 100;
    lpm_add(t[0],key8,32,(uint8_t*)&test_smac);
    test_smac.action_id = 102;
    lpm_add(t[0],key9,32,(uint8_t*)&test_smac);
    
    /* This is how the LPM tree should look like now:
    * ( wildcard is *, empty entry is in { }, entry with value is in [ ]
    * For simplicity's sake I shortened the keys by two bytes (the last two 0-s are omitted).
    *
    *   [def_val>]
    *       |
    *   [10,*,*,*] ---- {45,*,*,*} ---- {60,*,*,*}
    *       |               |               |
    *   [10,1,*,*]      {45,1,*,*}      [60,0,*,*]
    *       |               |
    *   [10,1,0,*]      {45,1,45,*}
    *                       |
    *                   [45,1,45,0] ---- [45,1,45,1] ---- [45,1,45,2]
    *
    */

    //lookup tests
    printf("\n-------> test with key {10,1,1,1,0,0}\n");
    lpm_lookup(t[0],key1);
    printf("\n-------> test with key {10,1,1,0,0,0}\n");
    lpm_lookup(t[0],key2);
    printf("\n-------> test with key {10,1,1,2,0,0}\n");
    lpm_lookup(t[0],key3);
    printf("\n-------> test with key {10,1,0,0,0,0}\n");
    lpm_lookup(t[0],key4);
    printf("\n-------> test with key {45,1,45,0,0,0}\n");
    lpm_lookup(t[0],key8);
    printf("\n-------> test with key {45,1,45,1,0,0}\n");
    lpm_lookup(t[0],key5);
    printf("\n-------> test with key {45,1,45,2,0,0}\n");
    lpm_lookup(t[0],key9);
    
    
    print_lpm_tree(t[0]->table);
    
    //add two more just to make it more complicated
    lpm_add(t[0],key10,40,(uint8_t*)&test_smac);
    lpm_add(t[0],key5,40,(uint8_t*)&test_smac);
    
    
    printf("\n-------> remove various values\n");
    lpm_remove(t[0],key9,32);
    lpm_remove(t[0],key5,32);
    lpm_remove(t[0],key5,32);
    lpm_remove(t[0],key5,16);
    lpm_remove(t[0],key6,32);
    lpm_remove(t[0],key0,24);
    lpm_remove(t[0],key0,16);
    lpm_remove(t[0],key0,8);
    
    print_lpm_tree(t[0]->table);
    
    free_entries(t[0]->table);
    t[0]->table = NULL; // important if we still want to use the table after this!
    
    printf("\n   <------------>\n     TERNARY tests \n   <------------>\n");
  
    //uint8_t mask1[6] = {255,0,0,0,0,0};
    //uint8_t mask2[6] = {255,255,0,0,0,0};
    //uint8_t mask3[6] = {255,0,255,255,0,0};
    uint8_t mask4[6] = {255,0,255,255,255,255};
    uint8_t mask5[6] = {0,0,0,255,0,0};
    
    printf("\n-------> adding key {10,1,1,1,0,0} with mask {0,0,0,255,0,0}\n");
    ternary_add(t[0],key1,mask5,(uint8_t*)&test_smac);
    printf("\n-------> adding key {10,1,1,2,0,0} with mask {0,0,0,255,0,0}\n");
    ternary_add(t[0],key3,mask5,(uint8_t*)&test_smac);
    printf("\n-------> adding key {10,1,1,0,0,0} with mask {0,0,0,255,0,0}\n");
    test_smac.action_id = 4;
    ternary_add(t[0],key2,mask5,(uint8_t*)&test_smac);
    
    printf("\n-------> lookup with key {10,1,1,1,0,0}\n");
    ternary_lookup(t[0],key2);
    
    
    printf("\n-------> removing them with key {10,1,1,1,0,0}\n");
    ternary_remove(t[0],key1);
    printf("\n-------> table should be empty now\n");
    print_lpm_tree(t[0]->table);
    
    printf("\n-------> adding key {10,1,1,0,0,0} with mask {255,0,255,255,255,255};\n");
    ternary_add(t[0],key2,mask4,(uint8_t*)&test_smac);
    printf("\n-------> adding key {60,0,1,1,0,0} with mask {255,0,255,255,255,255};\n");
    test_smac.action_id = 12;
    ternary_add(t[0],key7,mask4,(uint8_t*)&test_smac);
    printf("\n-------> lookup with key {10,1,1,1,0,0}\n");
    ternary_lookup(t[0],key2);


    printf("\n");
    return 0;
}

int 
p4_msg_digest_v1(lookup_table_t** t, char* name, int receiver, struct type_field_list* digest_field_list)
{
    if(strcmp("mac_learn_digest",name)==0) {
        //extract data
        uint8_t* port;
        uint8_t* mac;
        mac = digest_field_list->field_offsets[0];
        port = digest_field_list->field_offsets[1];
        printf("     : Learned that port %d belongs to MAC address %02x:%02x:%02x:%02x:%02x:%02x \n",
                                                      *port,mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
        //add SMAC entry
        struct smac_action smac_action_val;
        smac_action_val.action_id = 1; //action__nop
        exact_add(t[0],mac,(uint8_t*)&smac_action_val);

        //add DMAC entry				
        struct action_forward_params param;
        param.port[0] = *port; param.port[1] = 0;
        struct dmac_action dmac_action_val;	
        dmac_action_val.action_id = 2; //action_forward
        dmac_action_val.forward_params = param;
        exact_add(t[1],mac,(uint8_t*)&dmac_action_val);	
    } else {
        printf("Error: p4_msg_digest could not recognize the command.\n");
    }
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
  app.Stop (Seconds (1.08));

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
  app.Start (Seconds (1.05));
  app.Stop (Seconds (1.08));

  app = sink.Install (terminals.Get (0));
  app.Start (Seconds (0.0));

  NS_LOG_INFO ("Configure Tracing.");

  //
  // Configure tracing of all enqueue, dequeue, and NetDevice receive events.
  // Trace output will be sent to the file "lookup-tables-test.tr"
  //
  AsciiTraceHelper ascii;
  csma.EnableAsciiAll (ascii.CreateFileStream ("lookup-tables-test.tr"));

  //
  // Also configure some tcpdump traces; each interface will be traced.
  // The output files will be named:
  //     lookup-tables-test-<nodeId>-<interfaceId>.pcap
  // and can be read by the "tcpdump -r" command (use "-tt" option to
  // display timestamps correctly)
  //
  csma.EnablePcapAll ("lookup-tables-test", false);

  //
  // Now, do the actual simulation.
  //
  NS_LOG_INFO ("Run Simulation.");
  Simulator::Run ();
  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");
}
