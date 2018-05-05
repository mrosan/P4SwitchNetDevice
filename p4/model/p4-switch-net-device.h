/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef P4_SWITCH_NET_DEVICE_H
#define P4_SWITCH_NET_DEVICE_H

#include "ns3/simulator.h"
#include "ns3/log.h"
#include "ns3/mac48-address.h"

#include "ns3/ethernet-header.h"
#include "ns3/arp-header.h"
#include "ns3/tcp-header.h"
#include "ns3/udp-header.h"

#include "ns3/ipv4-l3-protocol.h"
#include "ns3/arp-l3-protocol.h"

#include "ns3/bridge-channel.h"
#include "ns3/node.h"
#include "ns3/enum.h"
#include "ns3/string.h"
#include "ns3/integer.h"
#include "ns3/uinteger.h"

#include <map>
#include <set>


extern "C"
{
  #include "p4-interface.h" 
}


namespace ns3 {

class P4SwitchNetDevice : public NetDevice
{
public:
  /**
   * Register this type.
   * \return The TypeId.
   */
  static TypeId GetTypeId (void);

  P4SwitchNetDevice ();
  virtual ~P4SwitchNetDevice ();
  
  
  /**
   * \brief Add a 'port' to a switch device
   *
   * This method adds a new switch port to a P4SwitchNetDevice, so that
   * the new switch port NetDevice becomes part of the switch and L2
   * frames start being forwarded to/from this NetDevice.
   *
   * \note The netdevice that is being added as switch port must
   * _not_ have an IP address.  In order to add IP connectivity to a
   * bridging node you must enable IP on the P4SwitchNetDevice itself,
   * never on its port netdevices.
   *
   * \param switchPort The port to add.
   * \return 0 if everything's ok, otherwise an error number.
   */
  int AddSwitchPort (Ptr<NetDevice> switchPort);
 
  
  /**
   * Called when a packet is received on one of the switch's ports.
   * Performs checks on the received packet, then calls SendToSwitch.
   *
   * \param netdev The port the packet was received on.
   * \param packet The Packet itself.
   * \param protocol The protocol defining the Packet.
   * \param src The source address of the Packet.
   * \param dst The destination address of the Packet.
   * \param PacketType Type of the packet.
   */
  void ReceiveFromDevice (Ptr<NetDevice> netdev, Ptr<const Packet> packet, uint16_t protocol, const Address& src, const Address& dst, PacketType packetType);


  /**
   * \brief Set callback functions passed down from the simulation file.
   *
   * This method sets two function pointers, one used for initializing
   * the table, the other used as a controller for the switch.
   *
   * \return 0 if everything's ok.
   */
  int SetCallbackFunctions( int (*init_tables_callback)(lookup_table_t**), int (*p4_msg_digest_callback)(lookup_table_t** t, char* name, int receiver, struct type_field_list* digest_field_list) );


  /**
   * \brief Process the packet.
   *
   * This method initializes the P4 switch, sends the Packet to it to be proccessed,
   * extracts the egress port belonging to the Packet, then forwards the Packet
   * through the appropriate ports.
   *
   * \param packet The Packet itself.
   * \param src The source address of the Packet.
   * \param dst The destination address of the Packet.
   * \param inport The port number through which the Packet has arrived.
   * \param protocol The protocol defining the Packet.
   * \param mtu Maximum transmission unit.
   * \return 0 if everything's ok, otherwise an error number.
   */
  int HandlePacket ( Ptr<const Packet> packet, const Address& src, const Address& dst, int inport, uint16_t protocol, uint16_t mtu );
  
  
  /**
   * \brief Convert the packet.
   *
   * This method converts the Packet into a bytestring, re-adds the ethernet
   * header, and puts it into the P4 switch's packet descriptor structure.
   *
   * \param pd The P4 switch's packet format.
   * \param constPacket The Packet itself.
   * \param src The source address of the Packet.
   * \param dst The destination address of the Packet.
   * \param protocol The protocol defining the Packet.
   * \param mtu Maximum transmission unit.
   */
  void BufferFromPacket ( packet_descriptor_t* pd, Ptr<const Packet> constPacket, const Address& src, const Address& dst, uint16_t protocol, uint16_t mtu );
  
  /**
   * This method sets whether the "dropped" field of the packet_descriptor_t should be disregarded or not.
   *
   * \param allowDrop If set to true, the P4 switch will be expected to set the packet descriptor's dropped field. 
   */
  void SetPacketDrop( bool allowDrop );
  
  // From NetDevice
  virtual void SetIfIndex (const uint32_t index);
  virtual uint32_t GetIfIndex (void) const;
  virtual Ptr<Channel> GetChannel (void) const;
  virtual void SetAddress (Address address);
  virtual Address GetAddress (void) const;
  virtual bool SetMtu (const uint16_t mtu);
  virtual uint16_t GetMtu (void) const;
  virtual bool IsLinkUp (void) const;
  virtual void AddLinkChangeCallback (Callback<void> callback);
  virtual bool IsBroadcast (void) const;
  virtual Address GetBroadcast (void) const;
  virtual bool IsMulticast (void) const;
  virtual Address GetMulticast (Ipv4Address multicastGroup) const;
  virtual bool IsPointToPoint (void) const;
  virtual bool IsBridge (void) const;
  virtual bool Send (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber);
  virtual bool SendFrom (Ptr<Packet> packet, const Address& source, const Address& dest, uint16_t protocolNumber);
  virtual Ptr<Node> GetNode (void) const;
  virtual void SetNode (Ptr<Node> node);
  virtual bool NeedsArp (void) const;
  virtual void SetReceiveCallback (NetDevice::ReceiveCallback cb);
  virtual void SetPromiscReceiveCallback (NetDevice::PromiscReceiveCallback cb);
  virtual bool SupportsSendFrom () const;
  virtual Address GetMulticast (Ipv6Address addr) const;

private:
  NetDevice::ReceiveCallback m_rxCallback;
  NetDevice::PromiscReceiveCallback m_promiscRxCallback;
  
  Mac48Address m_address;                 ///< Address of this device.
  Ptr<Node> m_node;                       ///< Node this device is installed on.
  Ptr<BridgeChannel> m_channel;           ///< Collection of port channels into the Switch Channel.
  uint32_t m_ifIndex;                     ///< Interface Index.
  uint16_t m_mtu;                         ///< Maximum Transmission Unit.
  bool m_drop;                            ///< Consider the packet descriptor's dropped field.
  
  typedef std::pair<Ptr<NetDevice>,int> Port_t;
  std::vector<Port_t> m_ports;            ///< Switch's ports.
  
  lookup_table_t** m_tables;              ///< Lookup tables.
  
  p4_msg_digest m_digest;                 ///< Function pointer to message digest function.
  
};

}

#endif /* P4_SWITCH_NET_DEVICE_H */

