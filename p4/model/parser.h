 #ifndef __HEADER_INFO_H__// sugar@95
 #define __HEADER_INFO_H__// sugar@96

 #define MODIFIED 1// sugar@98

 typedef struct parsed_fields_s {// sugar@100
 uint32_t field_instance_ipv4_ttl;// sugar@104
 uint8_t attr_field_instance_ipv4_ttl;// sugar@105
 } parsed_fields_t;// sugar@106
 
 #define HEADER_INSTANCE_COUNT 3// sugar@108
 #define HEADER_STACK_COUNT 0// sugar@109
 #define FIELD_INSTANCE_COUNT 21// sugar@110
 
 enum header_stack_e {
  header_stack_
};

// sugar@112
 enum header_instance_e {
  header_instance_standard_metadata,
  header_instance_ethernet,
  header_instance_ipv4
};

// sugar@113
 enum field_instance_e {
  field_instance_standard_metadata_ingress_port,
  field_instance_standard_metadata_packet_length,
  field_instance_standard_metadata_egress_spec,
  field_instance_standard_metadata_egress_port,
  field_instance_standard_metadata_egress_instance,
  field_instance_standard_metadata_instance_type,
  field_instance_standard_metadata_clone_spec,
  field_instance_standard_metadata__padding,
  field_instance_ethernet_dstAddr,
  field_instance_ethernet_srcAddr,
  field_instance_ethernet_etherType,
  field_instance_ipv4_versionIhl,
  field_instance_ipv4_diffserv,
  field_instance_ipv4_totalLen,
  field_instance_ipv4_identification,
  field_instance_ipv4_fragOffset,
  field_instance_ipv4_ttl,
  field_instance_ipv4_protocol,
  field_instance_ipv4_hdrChecksum,
  field_instance_ipv4_srcAddr,
  field_instance_ipv4_dstAddr
};

// sugar@114
 typedef enum header_instance_e header_instance_t;// sugar@115
 typedef enum field_instance_e field_instance_t;// sugar@116
 static const int header_instance_byte_width[HEADER_INSTANCE_COUNT] = {
  20 /* header_instance_standard_metadata */,
  14 /* header_instance_ethernet */,
  20 /* header_instance_ipv4 */
};

// sugar@117
 static const int header_instance_is_metadata[HEADER_INSTANCE_COUNT] = {
  1 /* header_instance_standard_metadata */,
  0 /* header_instance_ethernet */,
  0 /* header_instance_ipv4 */
};

// sugar@118
 static const int header_instance_var_width_field[HEADER_INSTANCE_COUNT] = {
  -1 /* fixed-width header */ /* header_instance_standard_metadata */,
  -1 /* fixed-width header */ /* header_instance_ethernet */,
  -1 /* fixed-width header */ /* header_instance_ipv4 */
};

// sugar@119
 static const int field_instance_bit_width[FIELD_INSTANCE_COUNT] = {
  9 /* field_instance_standard_metadata_ingress_port */,
  32 /* field_instance_standard_metadata_packet_length */,
  9 /* field_instance_standard_metadata_egress_spec */,
  9 /* field_instance_standard_metadata_egress_port */,
  32 /* field_instance_standard_metadata_egress_instance */,
  32 /* field_instance_standard_metadata_instance_type */,
  32 /* field_instance_standard_metadata_clone_spec */,
  5 /* field_instance_standard_metadata__padding */,
  48 /* field_instance_ethernet_dstAddr */,
  48 /* field_instance_ethernet_srcAddr */,
  16 /* field_instance_ethernet_etherType */,
  8 /* field_instance_ipv4_versionIhl */,
  8 /* field_instance_ipv4_diffserv */,
  16 /* field_instance_ipv4_totalLen */,
  16 /* field_instance_ipv4_identification */,
  16 /* field_instance_ipv4_fragOffset */,
  8 /* field_instance_ipv4_ttl */,
  8 /* field_instance_ipv4_protocol */,
  16 /* field_instance_ipv4_hdrChecksum */,
  32 /* field_instance_ipv4_srcAddr */,
  32 /* field_instance_ipv4_dstAddr */
};

// sugar@120
 static const int field_instance_bit_offset[FIELD_INSTANCE_COUNT] = {
  0 /* field_instance_standard_metadata_ingress_port */,
  1 /* field_instance_standard_metadata_packet_length */,
  1 /* field_instance_standard_metadata_egress_spec */,
  2 /* field_instance_standard_metadata_egress_port */,
  3 /* field_instance_standard_metadata_egress_instance */,
  3 /* field_instance_standard_metadata_instance_type */,
  3 /* field_instance_standard_metadata_clone_spec */,
  3 /* field_instance_standard_metadata__padding */,
  0 /* field_instance_ethernet_dstAddr */,
  0 /* field_instance_ethernet_srcAddr */,
  0 /* field_instance_ethernet_etherType */,
  0 /* field_instance_ipv4_versionIhl */,
  0 /* field_instance_ipv4_diffserv */,
  0 /* field_instance_ipv4_totalLen */,
  0 /* field_instance_ipv4_identification */,
  0 /* field_instance_ipv4_fragOffset */,
  0 /* field_instance_ipv4_ttl */,
  0 /* field_instance_ipv4_protocol */,
  0 /* field_instance_ipv4_hdrChecksum */,
  0 /* field_instance_ipv4_srcAddr */,
  0 /* field_instance_ipv4_dstAddr */
};

// sugar@121
 static const int field_instance_byte_offset_hdr[FIELD_INSTANCE_COUNT] = {
  0 /* field_instance_standard_metadata_ingress_port */,
  1 /* field_instance_standard_metadata_packet_length */,
  5 /* field_instance_standard_metadata_egress_spec */,
  6 /* field_instance_standard_metadata_egress_port */,
  7 /* field_instance_standard_metadata_egress_instance */,
  11 /* field_instance_standard_metadata_instance_type */,
  15 /* field_instance_standard_metadata_clone_spec */,
  19 /* field_instance_standard_metadata__padding */,
  0 /* field_instance_ethernet_dstAddr */,
  6 /* field_instance_ethernet_srcAddr */,
  12 /* field_instance_ethernet_etherType */,
  0 /* field_instance_ipv4_versionIhl */,
  1 /* field_instance_ipv4_diffserv */,
  2 /* field_instance_ipv4_totalLen */,
  4 /* field_instance_ipv4_identification */,
  6 /* field_instance_ipv4_fragOffset */,
  8 /* field_instance_ipv4_ttl */,
  9 /* field_instance_ipv4_protocol */,
  10 /* field_instance_ipv4_hdrChecksum */,
  12 /* field_instance_ipv4_srcAddr */,
  16 /* field_instance_ipv4_dstAddr */
};

// sugar@122
 static const uint32_t field_instance_mask[FIELD_INSTANCE_COUNT] = {
  0x80ff /* field_instance_standard_metadata_ingress_port */,
  0x0 /* field_instance_standard_metadata_packet_length */,
  0xc07f /* field_instance_standard_metadata_egress_spec */,
  0xe03f /* field_instance_standard_metadata_egress_port */,
  0x0 /* field_instance_standard_metadata_egress_instance */,
  0x0 /* field_instance_standard_metadata_instance_type */,
  0x0 /* field_instance_standard_metadata_clone_spec */,
  0x1f /* field_instance_standard_metadata__padding */,
  0x0 /* field_instance_ethernet_dstAddr */,
  0x0 /* field_instance_ethernet_srcAddr */,
  0xffff /* field_instance_ethernet_etherType */,
  0xff /* field_instance_ipv4_versionIhl */,
  0xff /* field_instance_ipv4_diffserv */,
  0xffff /* field_instance_ipv4_totalLen */,
  0xffff /* field_instance_ipv4_identification */,
  0xffff /* field_instance_ipv4_fragOffset */,
  0xff /* field_instance_ipv4_ttl */,
  0xff /* field_instance_ipv4_protocol */,
  0xffff /* field_instance_ipv4_hdrChecksum */,
  0xffffffff /* field_instance_ipv4_srcAddr */,
  0xffffffff /* field_instance_ipv4_dstAddr */
};

// sugar@123
 static const header_instance_t field_instance_header[FIELD_INSTANCE_COUNT] = {
  header_instance_standard_metadata /* field_instance_standard_metadata_ingress_port */,
  header_instance_standard_metadata /* field_instance_standard_metadata_packet_length */,
  header_instance_standard_metadata /* field_instance_standard_metadata_egress_spec */,
  header_instance_standard_metadata /* field_instance_standard_metadata_egress_port */,
  header_instance_standard_metadata /* field_instance_standard_metadata_egress_instance */,
  header_instance_standard_metadata /* field_instance_standard_metadata_instance_type */,
  header_instance_standard_metadata /* field_instance_standard_metadata_clone_spec */,
  header_instance_standard_metadata /* field_instance_standard_metadata__padding */,
  header_instance_ethernet /* field_instance_ethernet_dstAddr */,
  header_instance_ethernet /* field_instance_ethernet_srcAddr */,
  header_instance_ethernet /* field_instance_ethernet_etherType */,
  header_instance_ipv4 /* field_instance_ipv4_versionIhl */,
  header_instance_ipv4 /* field_instance_ipv4_diffserv */,
  header_instance_ipv4 /* field_instance_ipv4_totalLen */,
  header_instance_ipv4 /* field_instance_ipv4_identification */,
  header_instance_ipv4 /* field_instance_ipv4_fragOffset */,
  header_instance_ipv4 /* field_instance_ipv4_ttl */,
  header_instance_ipv4 /* field_instance_ipv4_protocol */,
  header_instance_ipv4 /* field_instance_ipv4_hdrChecksum */,
  header_instance_ipv4 /* field_instance_ipv4_srcAddr */,
  header_instance_ipv4 /* field_instance_ipv4_dstAddr */
};

// sugar@124
 
 static const header_instance_t header_stack_elements[HEADER_STACK_COUNT][10] = {
  
};

// sugar@126
 static const unsigned header_stack_size[HEADER_STACK_COUNT] = {
  
};

// sugar@127
 typedef enum header_stack_e header_stack_t;// sugar@128
 
 #endif // __HEADER_INFO_H__// sugar@130
