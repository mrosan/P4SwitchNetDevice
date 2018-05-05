/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "p4-switch-net-device.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("P4SwitchNetDevice");
NS_OBJECT_ENSURE_REGISTERED (P4SwitchNetDevice);


TypeId
P4SwitchNetDevice::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::P4SwitchNetDevice")
    .SetParent<NetDevice> ()
    .SetGroupName ("P4")
    .AddConstructor<P4SwitchNetDevice> ()
  ;
  return tid;
}


P4SwitchNetDevice::P4SwitchNetDevice ()
  : m_node (0),
    m_ifIndex (0),
    m_mtu (0xffff)
{
  NS_LOG_FUNCTION_NOARGS ();
  m_channel = CreateObject<BridgeChannel> ();

  m_ports.reserve (MAX_PORTS);
  
  m_tables = new lookup_table_t*[NB_TABLES];
  create_tables(m_tables);
  
  m_drop = false;
}

P4SwitchNetDevice::~P4SwitchNetDevice ()
{
  NS_LOG_FUNCTION_NOARGS ();
  delete_tables(m_tables);
  delete[] m_tables;  
}

int
P4SwitchNetDevice::SetCallbackFunctions( int (*init_tables_callback)(lookup_table_t**), int (*p4_msg_digest_callback)(lookup_table_t** t, char* name, int receiver, struct type_field_list* digest_field_list) )
{
  NS_LOG_INFO ("Setting callback functions.");
  init_tables_callback(m_tables);
  m_digest = p4_msg_digest_callback;
  return 0;
}

int
P4SwitchNetDevice::AddSwitchPort (Ptr<NetDevice> switchPort)
{
  NS_LOG_FUNCTION_NOARGS ();
  NS_ASSERT (switchPort != this);
  if (!Mac48Address::IsMatchingType (switchPort->GetAddress ()))
    {
      NS_FATAL_ERROR ("Device does not support eui 48 addresses: cannot be added to switch.");
    }
  if (!switchPort->SupportsSendFrom ())
    {
      NS_FATAL_ERROR ("Device does not support SendFrom: cannot be added to switch.");
    }
  if (m_address == Mac48Address ())
    {
      m_address = Mac48Address::ConvertFrom (switchPort->GetAddress ());
    }

  if (m_ports.size () < MAX_PORTS)
    { 
      int port_num;
      if ( m_ports.empty() ) 
      {
        port_num = 0;
      } else 
      {
        port_num = m_ports.back().second + 1;
      }
      if (port_num == BROADCAST_PORT) port_num++;
      
      m_ports.push_back (std::make_pair(switchPort,port_num));
      NS_LOG_DEBUG ("RegisterProtocolHandler for " << switchPort->GetInstanceTypeId ().GetName ());
      m_node->RegisterProtocolHandler (MakeCallback (&P4SwitchNetDevice::ReceiveFromDevice, this), 0, switchPort, true);
      m_channel->AddChannel (switchPort->GetChannel ());
    }
  else
    {
      return EXFULL;
    }

  return 0;
}

void
P4SwitchNetDevice::ReceiveFromDevice (Ptr<NetDevice> netdev, Ptr<const Packet> packet, uint16_t protocol,
                                            const Address& src, const Address& dst, PacketType packetType)
{
  NS_LOG_FUNCTION_NOARGS ();
  NS_LOG_INFO ("--------------------------------------------");
  NS_LOG_DEBUG ("UID is " << packet->GetUid ());

  if (!m_promiscRxCallback.IsNull ())
    {
      m_promiscRxCallback (this, packet, protocol, src, dst, packetType);
    }

  Mac48Address src48 = Mac48Address::ConvertFrom (src);
  Mac48Address dst48 = Mac48Address::ConvertFrom (dst);
  NS_LOG_INFO ("Received packet from " << src48 << " looking for " << dst48);


  for (size_t i = 0; i < m_ports.size (); i++)
    {    
      if (m_ports[i].first == netdev)
      {
        if (packetType == PACKET_HOST && dst48 == m_address)
        {
          m_rxCallback (this, packet, protocol, src);
        }
        else if (packetType == PACKET_BROADCAST || packetType == PACKET_MULTICAST || packetType == PACKET_OTHERHOST)
        {
            if (packetType == PACKET_OTHERHOST && dst48 == m_address)
            {
              m_rxCallback (this, packet, protocol, src);
            } else
            {
              if (packetType != PACKET_OTHERHOST)
              {
                m_rxCallback (this, packet, protocol, src);
              }
              
              if ( HandlePacket(packet,src48,dst48,m_ports[i].second,protocol,netdev->GetMtu ()) )
              {
                NS_LOG_INFO("Error occurred at HandlePacket."); 
              }
              //else NS_LOG_INFO("Returned from HandlePacket without error.");  
            }
        }
        break;
      }
    }
}

int
P4SwitchNetDevice::HandlePacket(Ptr<const Packet> packet, const Address& src, const Address& dst, int inport, uint16_t protocol, uint16_t mtu){
/*
  //For testing purposes:
  std::cout << "\n-----------------------The content of the packet ---------------------------\n";
  std::cout << packet->ToString() << std::endl;
  std::cout << "----------------------------------------------------------------------------\n\n";
*/

  //initialize packet buffer and dataplane
  packet_descriptor_t* pd = new packet_descriptor_t;
  BufferFromPacket (pd,packet,src,dst,protocol,mtu);
  init_dataplane(pd, m_tables);
  
  //set the packet descriptor's inport field manually
  initialize(pd, inport);
    
  //set the global "backend" variable that the p4 code uses
  set_fake_backend(m_tables,m_digest);

  //let the P4 code handle the packet
  handle_packet(pd, m_tables);
 
  
  if (m_drop && pd->dropped)
  {
      NS_LOG_INFO ("Packet dropped. (pd->dropped is set to true)");
  } else
  {
      //get the result port address
      int port = get_outport(pd);     
      
      //send the packet through the appropriate port
      //if broadcast, send everywhere except 'inport'
      if (port == BROADCAST_PORT)
      {
        for (size_t i = 0; i < m_ports.size (); i++)
        {
          if (m_ports[i].second != inport) 
          {
            NS_LOG_INFO ("(broadcast) Sending packet " << packet->GetUid () << " over port " << i);
            m_ports[i].first->SendFrom(packet->Copy (),src,dst,protocol);
          }
        }
      } else
      {
        bool found = false;
        size_t i = 0;
        while ( !found && i < m_ports.size () )
        {
          if (m_ports[i].second == port)
          {
              NS_LOG_INFO ("Sending packet " << packet->GetUid () << " over port " << port <<".");
              m_ports[i].first->SendFrom(packet->Copy (),src,dst,protocol);
              found = true;
          }
          i++;
        }
        if (!found)
        {
          NS_LOG_ERROR("ERROR: The port number received from the lookup doesn't exist.");
          return 1;
        }
      }
  }
  delete pd->data;
  delete pd;
  
  return 0;
}

void
P4SwitchNetDevice::BufferFromPacket (packet_descriptor_t* pd, Ptr<const Packet> constPacket, const Address& src, const Address& dst, uint16_t protocol, uint16_t mtu)
{
  NS_LOG_INFO ("Creating P4 buffer from packet.");
  //printf("The protocol (EtherType) of this packet is: \n"); print_key((uint8_t*)&protocol, 2);

  Ptr<Packet> packet = constPacket->Copy ();
  Mac48Address src48 = Mac48Address::ConvertFrom (src);
  Mac48Address dst48 = Mac48Address::ConvertFrom (dst);
  uint8_t srcBuf[6]; 
  src48.CopyTo(srcBuf);
  uint8_t dstBuf[6]; 
  dst48.CopyTo(dstBuf);

  size_t s = packet->GetSize ();  
  uint8_t dataBuf[s];
  packet->CopyData ((uint8_t*)&dataBuf, s);

  size_t size = 2 * ETH_ADDR_LEN + ETH_TYPE_LEN + s;
  pd->data = new uint8_t[size];
 
  //copying destination MAC address
  memcpy(pd->data,dstBuf,ETH_ADDR_LEN);
  //copying source MAC address
  memcpy(pd->data + ETH_ADDR_LEN,srcBuf,ETH_ADDR_LEN);
  
  //the protocol is in reverse byteorder compared to EtherType, so extra steps are necessary:
  //memcpy(pd->data + 2*ETH_ADDR_LEN,&protocol,ETH_TYPE_LEN);
  memcpy(pd->data + 2*ETH_ADDR_LEN,(uint8_t*)&protocol+1,1);
  memcpy(pd->data + 2*ETH_ADDR_LEN+1,(uint8_t*)&protocol,1);
  
  memcpy(pd->data + 2*ETH_ADDR_LEN + ETH_TYPE_LEN,(uint8_t*)&dataBuf,s);  
  
  //For testing purposes:
  //printf("Print dataBuf: "); print_key((uint8_t*)&dataBuf,s);
  
  //possible improvement: check for VLAN tag (0x8100 EtherType value) and handle header accordingly (+4 bytes)
}

void
P4SwitchNetDevice::SetPacketDrop( bool allowDrop )
{
    m_drop = allowDrop;
}


//---------NetDevice methods---------

void
P4SwitchNetDevice::SetIfIndex (const uint32_t index)
{
  NS_LOG_FUNCTION_NOARGS ();
  m_ifIndex = index;
}

uint32_t
P4SwitchNetDevice::GetIfIndex (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_ifIndex;
}

Ptr<Channel>
P4SwitchNetDevice::GetChannel (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_channel;
}

void
P4SwitchNetDevice::SetAddress (Address address)
{
  NS_LOG_FUNCTION_NOARGS ();
  m_address = Mac48Address::ConvertFrom (address);
}

Address
P4SwitchNetDevice::GetAddress (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_address;
}

bool
P4SwitchNetDevice::SetMtu (const uint16_t mtu)
{
  NS_LOG_FUNCTION_NOARGS ();
  m_mtu = mtu;
  return true;
}

uint16_t
P4SwitchNetDevice::GetMtu (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_mtu;
}


bool
P4SwitchNetDevice::IsLinkUp (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return true;
}


void
P4SwitchNetDevice::AddLinkChangeCallback (Callback<void> callback)
{
}

bool
P4SwitchNetDevice::IsBroadcast (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return true;
}

Address
P4SwitchNetDevice::GetBroadcast (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return Mac48Address ("ff:ff:ff:ff:ff:ff");
}

bool
P4SwitchNetDevice::IsMulticast (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return true;
}

Address
P4SwitchNetDevice::GetMulticast (Ipv4Address multicastGroup) const
{
  NS_LOG_FUNCTION (this << multicastGroup);
  Mac48Address multicast = Mac48Address::GetMulticast (multicastGroup);
  return multicast;
}


bool
P4SwitchNetDevice::IsPointToPoint (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return false;
}

bool
P4SwitchNetDevice::IsBridge (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return true;
}


bool
P4SwitchNetDevice::Send (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber)
{
  NS_LOG_FUNCTION_NOARGS ();
  return SendFrom (packet, m_address, dest, protocolNumber);
}


bool
P4SwitchNetDevice::SendFrom (Ptr<Packet> packet, const Address& src, const Address& dest, uint16_t protocolNumber)
{
  NS_LOG_FUNCTION_NOARGS ();
  
  //ReceivefromDevice parameters:
  //(Ptr<NetDevice> netdev, Ptr<const Packet> packet, uint16_t protocol, const Address& src, const Address& dst, PacketType packetType)
  ReceiveFromDevice(this,packet,protocolNumber,src,dest,PACKET_OTHERHOST);

  return true;  
  //return false;
  
}


Ptr<Node>
P4SwitchNetDevice::GetNode (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_node;
}

void
P4SwitchNetDevice::SetNode (Ptr<Node> node)
{
  NS_LOG_FUNCTION_NOARGS ();
  m_node = node;
}

bool
P4SwitchNetDevice::NeedsArp (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return true;
}

void
P4SwitchNetDevice::SetReceiveCallback (NetDevice::ReceiveCallback cb)
{
  NS_LOG_FUNCTION_NOARGS ();
  m_rxCallback = cb;
}

void
P4SwitchNetDevice::SetPromiscReceiveCallback (NetDevice::PromiscReceiveCallback cb)
{
  NS_LOG_FUNCTION_NOARGS ();
  m_promiscRxCallback = cb;
}

bool
P4SwitchNetDevice::SupportsSendFrom () const
{
  NS_LOG_FUNCTION_NOARGS ();
  return true;
  //return false;
}

Address
P4SwitchNetDevice::GetMulticast (Ipv6Address addr) const
{
  NS_LOG_FUNCTION (this << addr);
  return Mac48Address::GetMulticast (addr);
}


} //ns3 namespace

