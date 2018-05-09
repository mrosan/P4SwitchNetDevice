/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include "p4-switch-helper.h"
#include "ns3/p4-switch-net-device.h"
#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/names.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("P4SwitchHelper");

P4SwitchHelper::P4SwitchHelper ()
{
  NS_LOG_FUNCTION_NOARGS (); 
  m_deviceFactory.SetTypeId ("ns3::P4SwitchNetDevice");
}

/*
P4SwitchHelper::~P4SwitchHelper ()
{
  NS_LOG_FUNCTION_NOARGS ();
}
*/

NetDeviceContainer
P4SwitchHelper::Install (Ptr<Node> node, NetDeviceContainer c, int (*init_tables_callback)(lookup_table_t**), int (*p4_msg_digest_callback)(lookup_table_t** t, char* name, int receiver, struct type_field_list* digest_field_list))
{
  NS_LOG_FUNCTION_NOARGS ();
  NS_LOG_INFO ("Installing switch device on node " << node->GetId () << ".");

  NetDeviceContainer devs;
  
  Ptr<P4SwitchNetDevice> dev = m_deviceFactory.Create<P4SwitchNetDevice> ();
  devs.Add (dev);
  node->AddDevice (dev);
  
  for (NetDeviceContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      NS_LOG_INFO ("Adding SwitchPort " << *i << ".");
      dev->AddSwitchPort (*i);
    }
  
  dev->SetCallbackFunctions(init_tables_callback, p4_msg_digest_callback);
  m_device = dev;
    
  return devs;
}

void
P4SwitchHelper::AllowPacketDrop ( bool allowed )
{
    m_device->SetPacketDrop(allowed);
}

}//ns3 namespace
