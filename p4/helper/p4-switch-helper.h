/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef P4_SWITCH_HELPER_H
#define P4_SWITCH_HELPER_H

#include "ns3/p4-switch-net-device.h"
#include "ns3/net-device-container.h"
#include "ns3/object-factory.h"

#include <string>

namespace ns3 {

class Node;

class P4SwitchHelper
{
public:
  P4SwitchHelper();
  //virtual ~P4SwitchHelper();

  /**
   * This method creates an ns3::P4SwitchNetDevice, adds the device
   * to the node and attaches the given NetDevices as ports of the switch.
   *
   * \param node The node to install the device in.
   * \param c Container of NetDevices to add as switch ports.
   * \param *init_tables_callback Pointer to the function that will be called after table creation.
   * \param *p4_msg_digest_callback Pointer to the function that functions as a controller for the switch.
   * \returns A container holding the added net device.
   */
   NetDeviceContainer Install ( Ptr<Node> node, 
                                NetDeviceContainer c, 
                                int (*init_tables_callback)(lookup_table_t**), 
                                int (*p4_msg_digest_callback)(lookup_table_t** t, char* name, int receiver, struct type_field_list* digest_field_list));
  
  
  /**
   * This method sets whether the "dropped" field of the packet_descriptor_t should be disregarded or not.
   *
   * \param allowed If set to true, the P4SwitchNetDevice will expect the P4 switch to set the packet descriptor's dropped field. 
   */
  void AllowPacketDrop ( bool allowed );

private:
  ObjectFactory m_deviceFactory;
  Ptr<P4SwitchNetDevice> m_device;
  int m_mtu;

};

}//ns3 namespace

#endif /* P4_SWITCH_HELPER_H */

