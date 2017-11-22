 #include "p4-interface.h"// sugar@62
 #include "actions.h" // apply_table_* and action_code_*// sugar@63

 extern int verify_packet(packet_descriptor_t* pd);// sugar@65

 void print_mac(uint8_t* v) { printf("%02hhX:%02hhX:%02hhX:%02hhX:%02hhX:%02hhX\n", v[0], v[1], v[2], v[3], v[4], v[5]); }// sugar@67
 void print_ip(uint8_t* v) { printf("%d.%d.%d.%d\n",v[0],v[1],v[2],v[3]); }// sugar@68
 
 static inline void p4_pe_header_too_short(packet_descriptor_t *pd) {// sugar@72
 pd->dropped = 1;// sugar@74
 }// sugar@77
 static inline void p4_pe_default(packet_descriptor_t *pd) {// sugar@72
 pd->dropped = 1;// sugar@74
 }// sugar@77
 static inline void p4_pe_checksum(packet_descriptor_t *pd) {// sugar@72
 pd->dropped = 1;// sugar@74
 }// sugar@77
 static inline void p4_pe_unhandled_select(packet_descriptor_t *pd) {// sugar@72
 pd->dropped = 1;// sugar@74
 }// sugar@77
 static inline void p4_pe_index_out_of_bounds(packet_descriptor_t *pd) {// sugar@72
 pd->dropped = 1;// sugar@74
 }// sugar@77
 static inline void p4_pe_header_too_long(packet_descriptor_t *pd) {// sugar@72
 pd->dropped = 1;// sugar@74
 }// sugar@77
 static inline void p4_pe_out_of_packet(packet_descriptor_t *pd) {// sugar@72
 pd->dropped = 1;// sugar@74
 }// sugar@77
 static void// sugar@81
 extract_header_standard_metadata(uint8_t* buf, packet_descriptor_t* pd) {// sugar@82
     pd->headers[header_instance_standard_metadata].pointer = buf;// sugar@83
 }// sugar@90
 
 static void// sugar@81
 extract_header_ethernet(uint8_t* buf, packet_descriptor_t* pd) {// sugar@82
     pd->headers[header_instance_ethernet].pointer = buf;// sugar@83
 }// sugar@90
 
 static void// sugar@81
 extract_header_ipv4(uint8_t* buf, packet_descriptor_t* pd) {// sugar@82
     pd->headers[header_instance_ipv4].pointer = buf;// sugar@83
 }// sugar@90
 
 static void parse_state_start(packet_descriptor_t* pd, uint8_t* buf, lookup_table_t** tables);// sugar@94
 static void parse_state_parse_ethernet(packet_descriptor_t* pd, uint8_t* buf, lookup_table_t** tables);// sugar@94
 static void parse_state_parse_ipv4(packet_descriptor_t* pd, uint8_t* buf, lookup_table_t** tables);// sugar@94

 static inline void build_key_parse_ethernet(packet_descriptor_t *pd, uint8_t *buf, uint8_t *key) {// sugar@100
 EXTRACT_INT32_BITS(pd, field_instance_ethernet_etherType, *(uint32_t*)key)// sugar@109
 key += sizeof(uint32_t);// sugar@110
 }// sugar@119
 static void parse_state_start(packet_descriptor_t* pd, uint8_t* buf, lookup_table_t** tables)// sugar@122
 {// sugar@123
     uint32_t value32;// sugar@124
     (void)value32;// sugar@125
  return parse_state_parse_ethernet(pd, buf, tables);// sugar@21
// sugar@154
 }// sugar@189
 
 static void parse_state_parse_ethernet(packet_descriptor_t* pd, uint8_t* buf, lookup_table_t** tables)// sugar@122
 {// sugar@123
     uint32_t value32;// sugar@124
     (void)value32;// sugar@125
     extract_header_ethernet(buf, pd);// sugar@130
     buf += pd->headers[header_instance_ethernet].length;// sugar@131
 uint8_t key[2];// sugar@157
 build_key_parse_ethernet(pd, buf, key);// sugar@158
     uint8_t case_value_0[2] = {// sugar@171
         8,// sugar@173
         0,// sugar@173
     };// sugar@174
     if ( memcmp(key, case_value_0, 2) == 0)// sugar@175
          return parse_state_parse_ipv4(pd, buf, tables);// sugar@21
// sugar@176
  {// sugar@25
   if(verify_packet(pd)) p4_pe_checksum(pd);// sugar@26
   return apply_table_ipv4_fib_lpm(pd, tables);// sugar@27
 }// sugar@28
// sugar@166
 }// sugar@189
 
 static void parse_state_parse_ipv4(packet_descriptor_t* pd, uint8_t* buf, lookup_table_t** tables)// sugar@122
 {// sugar@123
     uint32_t value32;// sugar@124
     (void)value32;// sugar@125
     extract_header_ipv4(buf, pd);// sugar@130
     buf += pd->headers[header_instance_ipv4].length;// sugar@131
 EXTRACT_INT32_AUTO(pd, field_instance_ipv4_ttl, value32)// sugar@135
 pd->fields.field_instance_ipv4_ttl = value32;// sugar@136
 pd->fields.attr_field_instance_ipv4_ttl = 0;// sugar@137
  {// sugar@25
   if(verify_packet(pd)) p4_pe_checksum(pd);// sugar@26
   return apply_table_ipv4_fib_lpm(pd, tables);// sugar@27
 }// sugar@28
// sugar@154
 }// sugar@189
 
 void parse_packet(packet_descriptor_t* pd, lookup_table_t** tables) {// sugar@192
     parse_state_start(pd, pd->data, tables);// sugar@193
 }// sugar@194
