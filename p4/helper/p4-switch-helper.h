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
   * to the node and attaches the given NetDevices as ports of the
   * switch.
   *
   * \param node The node to install the device in
   * \param c Container of NetDevices to add as switch ports
   * \returns A container holding the added net device.
   */
   NetDeviceContainer Install ( Ptr<Node> node, 
                                NetDeviceContainer c, 
                                int (*init_tables_callback)(lookup_table_t**), 
                                int (*p4_msg_digest_callback)(lookup_table_t** t, char* name, int receiver, struct type_field_list* digest_field_list));
  

private:
  ObjectFactory m_deviceFactory;
  //Ptr<P4SwitchNetDevice> m_device;
  int m_mtu;

};

}//ns3 namespace

#endif /* P4_SWITCH_HELPER_H */

